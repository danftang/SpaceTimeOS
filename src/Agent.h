#ifndef AGENT_H
#define AGENT_H

#include "Concepts.h"
#include "CallbackQueue.h"
#include "Channel.h"
#include "Boundary.h"


// TODO: Abstract over trajectory, so it's a function from time to spacetime point and a timeToIntersection with a lambda.\
// (or, perhaps point of intersection [the points should always have a frame-invariant ordering])
// ...or, since an agent must have a position at an event, the trajectory defines an offset over time (v*t)
//    so we're only abstracting over velocity:
//      - velocity * time = offset
//      - position + offset = position
//      - offset / velocity = time

template<class T, class ENV> class Agent;
template<SpaceTime SPACETIME> class AgentBase;

// TODO:
//  - ensure position access is thread-safe
//  - encapsulate pos/vel into a trajectory so that we don't need to speicalise the 1D case.
// Base class for all agents without reference to the derived type of the agent.
template<SpaceTime SPACETIME>
class AgentBase : public CallbackQueue {
private:
    SPACETIME               pos; // TODO:
    SPACETIME::Velocity     vel;

public:
    AgentBase(const SPACETIME &position, const SPACETIME &velocity) : pos(position), vel(velocity) { }
    AgentBase(const AgentBase<SPACETIME> &other) = delete; // Just don't copy objects
    AgentBase(AgentBase<SPACETIME> &&other) = delete;

    const SPACETIME &position() const { return pos; }
    const SPACETIME &velocity() const { return vel; }
    SPACETIME &velocity() { return vel; }

    void moveForward(SPACETIME::Scalar time) { pos += vel * time; }

};

// Base class specialization for agents in a global time.
// In this case all agents have velocity 1 so no need to store it.
template<SpaceTime SPACETIME> requires (SPACETIME::Dimensions == 1)
class AgentBase<SPACETIME> : public CallbackQueue {
private:
    SPACETIME               pos;

public:
    AgentBase(const SPACETIME &position, const SPACETIME &velocity) : pos(position) { }
    AgentBase(const AgentBase<SPACETIME> &other) = delete; // Just don't copy objects
    AgentBase(AgentBase<SPACETIME> &&other) = delete;

    const SPACETIME &position() const { return pos; }
    const SPACETIME velocity() const { return SPACETIME(1); }

    void moveForward(SPACETIME::Scalar time) { pos[0] += time; }
};


// Base class for all agents when the derived type is known.
// To define a new agent, derive from this type (or use AgentWrapper)
// T should be the derived type (in the Curiously Recurring Template Pattern)
// ENV should be the simulation type.
template<class T, class ENV>
class Agent : public AgentBase<typename ENV::SpaceTime> {
public:
    typedef ENV::SpaceTime              SpaceTime;
    typedef ENV::SpaceTime::Scalar      Scalar;
    typedef T                           DerivedType;
    typedef ENV                         Environment;
private:
    // TODO: If we call a vector of ChannelReaders a Field which affects a single agent,
    // and allow ChannelWriters to be freely copied,
    std::vector<ChannelReader<T>>   inChannels;

    friend ENV;
public:

    static constexpr Scalar REACTIONTIME = 1; // local time between absorbtion of a lambda and emission of resulting particles (should this be in SpatialFunction?).


    // Construct with current active agent's position and velocity
    Agent() : AgentBase<SpaceTime>(ENV::activeAgent->position(), ENV::activeAgent->velocity()) {
        sendCallbackTo(*ENV::activeAgent);
    }


    Agent(const SpaceTime &position, const SpaceTime &velocity = ENV::activeAgent->velocity()) : AgentBase<SpaceTime>(position, velocity) {
        // make velocity unit length
        // velocity /= sqrt(velocity*velocity);
        if(fabs(velocity*velocity - 1) > 1e-6)
            throw(std::runtime_error("Can't construct an agent with velocity that isn't of unit length"));
        if(velocity[0] < 0)
            throw(std::runtime_error("Can't construct an agent with velocity that isn't future pointing in the laboratory frame"));
        SpaceTime displacementFromParent = position - ENV::activeAgent->position();
        if(displacementFromParent*displacementFromParent < 0)
            throw(std::runtime_error("Can't construct an agent outside the future light-cone of the point of construction"));
        sendCallbackTo(*ENV::activeAgent);
    }


    // Attaches a ChannelReader to this object.
    void attach(ChannelReader<T> &&inChan) {
        if(inChan.timeToIntersection(this->position(),this->velocity()) < -REACTIONTIME) throw(std::runtime_error("Attempt to attach channel to an agent's past"));
        inChannels.push_back(std::move(inChan));
    }

    // returns a reference to an inCahnnel.
    ChannelReader<T> &getInChannel(size_t index) { return inChannels[index]; }

    // returns the number of in Channels.
    size_t nChannels() { return inChannels.size(); }

    // Closes a given inChannel.
    // invalidates inChannels.back() and inChannels.end()
    void detach(decltype(inChannels)::iterator channelIt) {
        if(&*channelIt != &inChannels.back()) *channelIt = std::move(inChannels.back());
        inChannels.pop_back();
    }  


    // Execute this objects lambdas until it blocks
    void step() {
        ENV::activeAgent = this; // set this to the active agent so all lambdas know where they are.
        bool notBlocking = true;
        typename decltype(inChannels)::iterator earliestChanIt;
        do {
            earliestChanIt = moveToEarliestChannel();
            if(earliestChanIt != inChannels.end()) {
                notBlocking = earliestChanIt->executeNext(derived());
            } else {
                notBlocking = false;
            }
        } while(notBlocking);
        this->execCallbacks();
        if(earliestChanIt != inChannels.end()) {
            sendCallbackTo(*earliestChanIt);
        } else {
            // no inChannels or hit end of simulation
            if(inChannels.empty()) {
                std::cout << "Out of inCahnnels, deleting " << this << std::endl;
                delete(&derived()); // no more inChannels
                return;
            } else {
                ENV::boundary.execute(derived()); // hit the boundary
                return; // paranoia
            }
        }
    }

    // Kills this agent by deleting all inChannels.
    // This will signal the end of the current step
    // which will then delete this object.
    void die() { inChannels.clear(); }

    
private:

    template<class DESTINATION>
    void sendCallbackTo(DESTINATION &blockingAgent) {
        blockingAgent.pushCallback([&me = derived()]() {
                ENV::submit([&me]() {
                    me.step();
                });
        });
    }


    // Access the derived type
    inline T &derived() { return *static_cast<T *>(this); }

    // finds the earliest channel and moves this to its intersection point,
    // detaching any closed channels as it goes.
    // If inChannels is empty, moves this to SpaceTime::TOP
    auto moveToEarliestChannel() {
        auto chanIt = inChannels.begin();
        auto end = inChannels.end();
        auto earliestChanIt = inChannels.end();
        Scalar earliestIntersectionTime = ENV::boundary.timeToIntersection(this->position(),this->velocity());
        while(chanIt != end) {
            if(chanIt->isClosed()) {
                --end;
                if(chanIt != end) *chanIt = std::move(*end);
            } else {
                auto intersectTime = chanIt->timeToIntersection(this->position(),this->velocity()) + REACTIONTIME;
                if(intersectTime <= earliestIntersectionTime) {
                    earliestIntersectionTime = intersectTime;
                    earliestChanIt = chanIt;
                }
                ++chanIt;
            }
        }
        if(earliestIntersectionTime > 0) this->moveForward(earliestIntersectionTime);
        if(earliestChanIt == inChannels.end()) {
            inChannels.erase(end, inChannels.end());
            earliestChanIt = inChannels.end(); // new end pointer
        } else {
            inChannels.erase(end, inChannels.end());
        }
        return earliestChanIt;
    }
};


// Use this class to turn a class that isn't derived from Agent<> into an Agent.
// The underlying class can be accessed through the 'object' member
// or by deference.
template<class CLASSTOWRAP, class ENV>
class AgentWrapper : public Agent<AgentWrapper<CLASSTOWRAP,ENV>,ENV> {
protected:
    template<std::convertible_to<typename ENV::SpaceTime> P, std::convertible_to<typename ENV::SpaceTime> V, class... ARGS>
    AgentWrapper(P &&position, V &&velocity, ARGS &&... args) : 
        Agent<AgentWrapper<CLASSTOWRAP,ENV>,ENV>(std::forward<P>(position), std::forward<V>(velocity)),
        object(std::forward<ARGS>(args)...) {};

public:

    template<class PARENT, class... ARGS>
    AgentWrapper(const Agent<PARENT,ENV> &parent, ARGS &&... args) : 
        Agent<AgentWrapper<CLASSTOWRAP,ENV>,ENV>(parent),
        object(std::forward<ARGS>(args)...) {};


    CLASSTOWRAP &operator ->() { return object; }
    const CLASSTOWRAP &operator ->() const { return object; }

    CLASSTOWRAP object;
};


// Use this class to turn a class that isn't derived from Agent<> into an Agent.
// The unserlying class is mixed-in with the Agent<> class, so members of
// each can be directly accessed.
// T should be the type we want to wrap
// ENV should be the simulation type.
template<class T, class ENV>
class AgentMixin : public Agent<AgentMixin<T,ENV>,ENV>, T {
public:
    template<std::convertible_to<typename ENV::SpaceTime> P, std::convertible_to<typename ENV::SpaceTime> V, class... ARGS>
    AgentMixin(P &&position, V &&velocity, ARGS &&... args) : 
        Agent<AgentMixin<T,ENV>,ENV>(std::forward<P>(position), std::forward<V>(velocity)),
        T(std::forward<ARGS>(args)...) {};

    template<class PARENT, class... ARGS>
    AgentMixin(const Agent<PARENT,ENV> &parent, ARGS &&... args) : 
        Agent<AgentMixin<T,ENV>,ENV>(parent),
        T(std::forward<ARGS>(args)...) {};
};

#endif
