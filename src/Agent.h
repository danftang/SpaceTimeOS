#ifndef AGENT_H
#define AGENT_H

#include "Concepts.h"
#include "CallbackQueue.h"
#include "Channel.h"

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

    template<std::convertible_to<std::function<void()>> LAMBDA>
    void callbackOnMove(LAMBDA &&callback) {
        callbacks.push(std::forward<LAMBDA>(callback));
    }

    void execCallbacks() { callbacks.execAll(); }

};


// Base class for all agents
template<class T, class SIM>
class Agent : public AgentBase<typename SIM::SpaceTime> {
private:
    std::vector<ChannelReader<T>>   inChannels;

    template<class P, class F>
    Agent(P &&position, F &&velocity) : 
        AgentBase<SpaceTime>(std::forward<P>(position)),
        velocity(std::forward<F>(velocity)) { }

    friend SIM;

public:
    typedef SIM::SpaceTime              SpaceTime;
    typedef SIM::SpaceTime::Scalar  Scalar;
    typedef T                           DerivedType;
    typedef SIM                         Simulation;

    SpaceTime                       velocity;

    static constexpr Scalar PROCESSINGTIME = 1; // local time between spatial intersection with a call and actually calling it


    template<class OTHERT>
    explicit Agent(Agent<OTHERT,SIM> &parent) : Agent(parent.position(), parent.velocity) { 
        sendCallbackTo(parent);
    }


    void attach(ChannelReader<T> &&inChan) { inChannels.push_back(std::move(inChan)); }

    ChannelReader<T> &getInChannel(size_t index) { return inChannels[index]; }

    size_t nChannels() { return inChannels.size(); }

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
            SIM::agentFinished(derived()); // submit self to the simulation so that the sim can decide what to do next (e.g. delete, save)
            return; // just for safety as this may be deleted by now.
        }
    }

    template<class DESTINATION>
    void sendCallbackTo(DESTINATION &blockingAgent) {
        blockingAgent.callbackOnMove([&me = derived()]() {
                SIM::submit(me);
        });
    }

    inline T &derived() { return *static_cast<T *>(this); }

private:

//     // returns true if an event was successfuly processed, false if blocking or empty
//     bool processNextEvent() {
//         auto ealiestChannelIt = getEarliestChannel();

//         // Process earliest channel
//         this->pos += this->velocity * (earliestIntersectionTime + PROCESSINGTIME);
// //        std::cout << this << " Moved to " << this->position << std::endl;
//         bool notBlocking = inChannels[earliestChannelIdx].executeNext();
//         if(inChannels[earliestChannelIdx].isClosed()) { // remove closed channel from inChannels
//             if(earliestChannelIdx != inChannels.size()-1) inChannels[earliestChannelIdx] = std::move(inChannels.back());
//             inChannels.pop_back();
//         }
//         return notBlocking;
//     }

    // finds the earliest channel and moves this to its intersection point,
    // detaching any closed channels as it goes.
    // If inChannels is empty, moves this to SpaceTime::TOP
    auto moveToEarliestChannel() {
        auto chanIt = inChannels.begin();
        auto earliestChanIt = inChannels.end();
        Scalar earliestIntersectionTime = Simulation::timeToIntersection(this->position(), velocity);
        while(chanIt != inChannels.end()) {
            if(chanIt->isClosed()) {
                detach(chanIt);
            } else {
                auto intersectTime = intersectionTime(*chanIt);
                if(intersectTime <= earliestIntersectionTime) {
                    earliestIntersectionTime = intersectTime;
                    earliestChanIt = chanIt;
                }
                ++chanIt;
            }
        }
        this->pos += this->velocity * (earliestIntersectionTime + PROCESSINGTIME);
        return earliestChanIt;
    }


    // intersection of this with a channel
    Scalar intersectionTime(const ChannelReader<T> &inChan) {
        return std::max(
            inChan.timeToIntersection(this->position(), velocity)
            ,-PROCESSINGTIME
            );
    }
};


template<class T, class SIM>
class AgentWrapper : public Agent<AgentWrapper<T,SIM>,SIM> {
public:
    template<class P, class V, class... ARGS>
    AgentWrapper(P &&position, V &&velocity, ARGS &&... args) : 
        Agent<AgentWrapper<T,SIM>,SIM>(std::forward<P>(position), std::forward<V>(velocity)),
        object(std::forward<ARGS>(args)...) {};

    T object;
};

#endif
