#ifndef AGENT_H
#define AGENT_H

#include "Concepts.h"
#include "CallbackQueue.h"
#include "Channel.h"

template<class SPACETIME> class AgentPosition : public SPACETIME { }; // tag to signal this is the position of a real agent

template<SpaceTime SPACETIME>
class AgentBase {
private:
    AgentPosition<SPACETIME>    pos;
    CallbackQueue               callbacks;

    AgentBase(const SPACETIME &position) : pos(position) { }

    AgentBase(SPACETIME &&position) : pos(std::move(position)) { }

    template<class T, class S> friend class Agent;
public:

    AgentBase(const AgentBase<SPACETIME> &other) = delete; // Just don't copy objects
    AgentBase(AgentBase<SPACETIME> &&other) = delete;

    AgentBase(const AgentPosition<SPACETIME> &position) : pos(position) { }

    AgentBase(AgentPosition<SPACETIME> &&position) : pos(std::move(position)) { }


    const AgentPosition<SPACETIME> &position() { return this->pos; }

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

public:
    typedef SIM::SpaceTime              SpaceTime;
    typedef SIM::SpaceTime::ScalarType  Scalar;
    typedef T                           DerivedType;
    typedef SIM                         Simulation;

    SpaceTime                       velocity;

    static constexpr Scalar PROCESSINGTIME = 1; // local time between spatial intersection with a call and actually calling it

    template<class P, class F>
    Agent(P &&position, F &&velocity) : 
        AgentBase<SpaceTime>(std::forward<P>(position)),
        velocity(std::forward<F>(velocity)) {
            setCallback(SIM::laboratory);
        }

    Agent(const Agent<T,SIM> &) = delete;  // don't copy agents;


    void attach(ChannelReader<T> &&inChan) { inChannels.push_back(std::move(inChan)); }

    ChannelReader<T> &getInChannel(size_t index) { return inChannels[index]; }

    size_t nChannels() { return inChannels.size(); }

    void submitStep() { SIM::submit(*static_cast<T *>(this)); }

    void detach(decltype(inChannels)::iterator channelIt) {
        if(&*channelIt != &inChannels.back()) *channelIt = std::move(inChannels.back());
        inChannels.pop_back();
    }  

    // step to move this object forward until it blocks
    void step() {
        bool isBlocking = false;
        typename decltype(inChannels)::iterator earliestChanIt;
        do {
            earliestChanIt = moveToEarliestChannel();
            if(earliestChanIt != inChannels.end()) {
                isBlocking = earliestChanIt->executeNext(*static_cast<T *>(this));
                SIM::laboratory.execCallbacks(); // start any new agents created by the last call.
            } else {
                // TODO: move to boundary and treat as out-of-bounds
                isBlocking = true;
            }
        } while(!isBlocking && SIM::isInBounds(this->position()));
        this->execCallbacks();
        if(earliestChanIt != inChannels.end() && isBlocking) {
            setCallback(*earliestChanIt);
        } else {
            // no inChannels or gone out of bounds
            SIM::agentFinished(this); // submit self to the simulation so that the sim can decide what to do next (e.g. delete, save)
            return; // just for safety as this may be deleted by now.
        }
    }

    void setCallback(AgentBase<SpaceTime> &agent) {
        agent.setCallback([this]() {
                this->submitStep();
        });
    }

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
        auto earliestChanIt = inChannels.begin();
        Scalar earliestIntersectionTime = SpaceTime::TOP / this->velocity;
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
            (inChan.position() - this->position()) / this->velocity
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
