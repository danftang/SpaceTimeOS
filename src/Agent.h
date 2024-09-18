#ifndef AGENT_H
#define AGENT_H

#include "Concepts.h"
#include "CallbackQueue.h"
#include "Channel.h"
#include "Boundary.h"

// Base class for all agents without reference to the derived type of the agent.
template<SpaceTime SPACETIME>
class AgentBase {
private:
    SPACETIME       pos;
    CallbackQueue   callbacks;

    AgentBase(const SPACETIME &position) : pos(position) { }

    AgentBase(SPACETIME &&position) : pos(std::move(position)) { }

    template<class T, class S> friend class Agent;
public:

    AgentBase(const AgentBase<SPACETIME> &other) = delete; // Just don't copy objects
    AgentBase(AgentBase<SPACETIME> &&other) = delete;

    const SPACETIME &position() const { return pos; }

    template<std::invocable LAMBDA>
    void callbackOnMove(LAMBDA &&callback) {
        callbacks.push(std::forward<LAMBDA>(callback));
    }

    void execCallbacks() { callbacks.execAll(); }

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

protected:
    SpaceTime                       vel;

public:


    static constexpr Scalar PROCESSINGTIME = 1; // local time between spatial intersection with a call and actually calling it

    template<class S, class F = SpaceTime>
    Agent(const BoundaryCoordinate<S> &boundaryCoord, F &&velocity = SpaceTime(1)) : 
        AgentBase<SpaceTime>(ENV::boundary.onBoundary(boundaryCoord)),
        vel(std::forward<F>(velocity)) {
            std::cout << "Creating agent at " << this->position() << std::endl;
            sendCallbackTo(ENV::boundary);
        }

    // Construct with parent's position and velocity
    template<class OTHERT>
    explicit Agent(const Agent<OTHERT,ENV> &parent) : AgentBase<SpaceTime>(parent.position()), vel(parent.velocity()) {
        sendCallbackTo(parent);
    }

    // Attaches a ChannelReader to this object.
    void attach(ChannelReader<T> &&inChan) {
        if(inChan.timeToIntersection(this->position(),velocity()) < -PROCESSINGTIME) throw(std::runtime_error("Attempt to attach channel to an agent's past"));
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
                ENV::boundary.boundaryEvent(derived()); // hit the boundary
                return; // paranoia
            }
        }
    }

    // Kills this agent by deleting all inChannels.
    // This will signal the end of the current step
    // which will then delete this object.
    void die() { inChannels.clear(); }

    template<class DESTINATION>
    void sendCallbackTo(DESTINATION &blockingAgent) {
        blockingAgent.callbackOnMove([&me = derived()]() {
                ENV::submit([&me]() {
                    me.step();
                });
        });
    }

    // Access the derived type
    inline T &derived() { return *static_cast<T *>(this); }

    const SpaceTime &velocity() const { return vel; }
    
private:

    // finds the earliest channel and moves this to its intersection point,
    // detaching any closed channels as it goes.
    // If inChannels is empty, moves this to SpaceTime::TOP
    auto moveToEarliestChannel() {
        auto chanIt = inChannels.begin();
        auto end = inChannels.end();
        auto earliestChanIt = inChannels.end();
        Scalar earliestIntersectionTime = ENV::boundary.timeToIntersection(this->position(),velocity());
        while(chanIt != end) {
            if(chanIt->isClosed()) {
                --end;
                if(chanIt != end) *chanIt = std::move(*end);
            } else {
                auto intersectTime = chanIt->timeToIntersection(this->position(),velocity()) + PROCESSINGTIME;
                if(intersectTime <= earliestIntersectionTime) {
                    earliestIntersectionTime = intersectTime;
                    earliestChanIt = chanIt;
                }
                ++chanIt;
            }
        }
        if(earliestIntersectionTime > 0) this->pos += this->vel * earliestIntersectionTime;
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
