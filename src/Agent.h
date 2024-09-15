#ifndef AGENT_H
#define AGENT_H

#include "Concepts.h"
#include "CallbackQueue.h"
#include "Channel.h"
#include "Boundary.h"

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


// Base class for all agents
template<class T, class ENV>
class Agent : public AgentBase<typename ENV::SpaceTime> {
private:
    std::vector<ChannelReader<T>>   inChannels;


    friend ENV;

public:
    typedef ENV::SpaceTime              SpaceTime;
    typedef ENV::SpaceTime::Scalar      Scalar;
    typedef T                           DerivedType;
    typedef ENV                         Environment;

    SpaceTime                       velocity;

    static constexpr Scalar PROCESSINGTIME = 1; // local time between spatial intersection with a call and actually calling it


    template<class F>
    Agent(const Boundary<SpaceTime>::Position &positionOnBoundary, F &&velocity) : 
        AgentBase<SpaceTime>(positionOnBoundary),
        velocity(std::forward<F>(velocity)) {
            std::cout << "Creating agent at " << this->position() << std::endl;
            sendCallbackTo(ENV::boundary);
        }

    Agent(const Boundary<SpaceTime>::Position &positionOnBoundary) : Agent(positionOnBoundary,SpaceTime(1)) { }

    template<class OTHERT>
    explicit Agent(Agent<OTHERT,ENV> &parent) : AgentBase<SpaceTime>(parent.position()), velocity(parent.velocity) {
        sendCallbackTo(parent);
    }


    void attach(ChannelReader<T> &&inChan) { inChannels.push_back(std::move(inChan)); }

    ChannelReader<T> &getInChannel(size_t index) { return inChannels[index]; }

    size_t nChannels() { return inChannels.size(); }

    // invalidates inChannels.back() and inChannels.end()
    void detach(decltype(inChannels)::iterator channelIt) {
        if(&*channelIt != &inChannels.back()) *channelIt = std::move(inChannels.back());
        inChannels.pop_back();
    }  

    // step to move this object forward until it blocks
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

    void die() { inChannels.clear(); }

    template<class DESTINATION>
    void sendCallbackTo(DESTINATION &blockingAgent) {
        blockingAgent.callbackOnMove([&me = derived()]() {
                ENV::submit([&me]() {
                    me.step();
                });
        });
    }

    inline T &derived() { return *static_cast<T *>(this); }

private:

    // finds the earliest channel and moves this to its intersection point,
    // detaching any closed channels as it goes.
    // If inChannels is empty, moves this to SpaceTime::TOP
    auto moveToEarliestChannel() {
        auto chanIt = inChannels.begin();
        auto end = inChannels.end();
        auto earliestChanIt = inChannels.end();
        Scalar earliestIntersectionTime = ENV::boundary.timeToIntersection(this->position(), velocity);
        while(chanIt != end) {
            if(chanIt->isClosed()) {
                --end;
                if(chanIt != end) *chanIt = std::move(*end);
            } else {
                auto intersectTime = chanIt->timeToIntersection(this->position(), velocity) + PROCESSINGTIME;
                if(intersectTime <= earliestIntersectionTime) {
                    earliestIntersectionTime = intersectTime;
                    earliestChanIt = chanIt;
                }
                ++chanIt;
            }
        }
        this->pos += this->velocity * earliestIntersectionTime;
        if(earliestChanIt == inChannels.end()) {
            inChannels.erase(end, inChannels.end());
            earliestChanIt = inChannels.end(); // new end pointer
        } else {
            inChannels.erase(end, inChannels.end());
        }
        return earliestChanIt;
    }
};


template<class T, class ENV>
class AgentWrapper : public Agent<AgentWrapper<T,ENV>,ENV> {
public:
    template<class P, class V, class... ARGS>
    AgentWrapper(P &&position, V &&velocity, ARGS &&... args) : 
        Agent<AgentWrapper<T,ENV>,ENV>(std::forward<P>(position), std::forward<V>(velocity)),
        object(std::forward<ARGS>(args)...) {};

    T object;
};

#endif
