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


// TODO:
//  - ensure position access is thread-safe
//  - encapsulate pos/vel into a trajectory so that we don't need to speicalise the 1D case.
// Base class for all agents without reference to the derived type of the agent.
template<class ENV>
class Agent : public CallbackQueue {
public:
    typedef ENV::SpaceTime              SpaceTime;
    typedef ENV::SpaceTime::Scalar      Scalar;
    typedef ENV                         Environment;

private:
    SpaceTime               pos; // TODO: replace with TRAJECTORY template
    SpaceTime::Velocity     vel;

    std::vector<ChannelExecutor<ENV>>   inChannels;

    friend ENV;

public:

    Agent(const Agent<ENV> &other) = delete; // Just don't copy objects
    Agent(Agent<ENV> &&other) = delete;

    // Construct with current active agent's position and velocity
    Agent() : pos(ENV::activeAgent->position()), vel(ENV::activeAgent->velocity()) {
        sendCallbackTo(*ENV::activeAgent);
    }


    Agent(const SpaceTime &position, const SpaceTime &velocity = ENV::activeAgent->velocity()) : pos(position), vel(velocity) {
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


    virtual ~Agent() {} // necessary to delete an agent without knowing the derived typw

    const SpaceTime &position() const { return pos; }
    const SpaceTime &velocity() const { return vel; }
    SpaceTime &velocity() { return vel; }

    void moveForward(Scalar time) { pos += vel * time; }


    static constexpr Scalar REACTIONTIME = 1; // local time between absorbtion of a lambda and emission of resulting particles (should this be in SpatialFunction?).




    // Attaches a ChannelReader to this object.
    void attach(ChannelExecutor<ENV> &&inChan) {
        if(inChan.timeToIntersection(this->position(),this->velocity()) < -REACTIONTIME) throw(std::runtime_error("Attempt to attach channel to an agent's past"));
        inChannels.push_back(std::move(inChan));
    }

    // returns a reference to an inCahnnel.
    ChannelExecutor<ENV> &getInChannel(size_t index) { return inChannels[index]; }

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
                notBlocking = earliestChanIt->executeNext(*this);
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
                delete(this); return; // no more inChannels
            } else {
                ENV::boundary.execute(*this); // hit the boundary
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
        blockingAgent.pushCallback([me = this]() {
                ENV::submit([me]() {
                    me->step();
                });
        });
    }


    // Access the derived type
    // inline T &derived() { return *static_cast<T *>(this); }

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
class AgentWrapper : public Agent<ENV> {
protected:
    template<std::convertible_to<typename ENV::SpaceTime> P, std::convertible_to<typename ENV::SpaceTime> V, class... ARGS>
    AgentWrapper(P &&position, V &&velocity, ARGS &&... args) : 
        Agent<ENV>(std::forward<P>(position), std::forward<V>(velocity)),
        object(std::forward<ARGS>(args)...) {};

public:

    template<class... ARGS>
    AgentWrapper(const Agent<ENV> &parent, ARGS &&... args) : 
        Agent<ENV>(parent),
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
class AgentMixin : public Agent<ENV>, T {
public:
    template<std::convertible_to<typename ENV::SpaceTime> P, std::convertible_to<typename ENV::SpaceTime> V, class... ARGS>
    AgentMixin(P &&position, V &&velocity, ARGS &&... args) : 
        Agent<ENV>(std::forward<P>(position), std::forward<V>(velocity)),
        T(std::forward<ARGS>(args)...) {};
};

#endif
