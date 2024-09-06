#ifndef SPACETIMEOBJECT_H
#define SPACETIMEOBJECT_H

#include <vector>
#include <list>
#include <deque>
#include "Concepts.h"
#include "Channel.h"
#include "ThreadPool.h"
#include "ThreadSafeQueue.h"


template<SpaceTime SPACETIME>
class SpaceTimeBase {
public:
    SPACETIME                               pos;
    ThreadSafeQueue<std::function<void()>>  callOnMove;

    SpaceTimeBase(const SpaceTimeBase<SPACETIME> &other) = delete; // Just don't copy objects
    SpaceTimeBase(SpaceTimeBase<SPACETIME> &&other) = delete;

    SpaceTimeBase(const SPACETIME &position) : pos(position) { }

    SpaceTimeBase(SPACETIME &&position) : pos(std::move(position)) { }

    ~SpaceTimeBase() { execCallbacks(); } // make sure all writeChannels are unblocked
    
    template<std::convertible_to<std::function<void()>> LAMBDA>
    void callbackOnMove(LAMBDA &&callback) {
        callOnMove.emplace(std::forward<LAMBDA>(callback));
    }

    void execCallbacks() {
        while(!callOnMove.empty()) {
            callOnMove.front()();
            callOnMove.pop();
        }
    }
};


template<class T, class SIM>
class SpaceTimeObject : public SpaceTimeBase<typename SIM::SpaceTime> {
protected:
    std::vector<ChannelReader<T,SIM>>  inChannels;

public:
    typedef SIM::SpaceTime              SpaceTime;
    typedef SIM::SpaceTime::ScalarType  Time;
    typedef T                           ValueType;


    SpaceTime   velocity;
    T           object;

    static constexpr Time PROCESSINGTIME = 0.1; // local time between spatial intersection with a call and actually calling it


    template<class P, class F, class... ARGS>
    SpaceTimeObject(P &&position, F &&velocity, ARGS &&... args) : 
        SpaceTimeBase<SpaceTime>(std::forward<P>(position)),
        velocity(std::forward<F>(velocity)),
        object(std::forward<ARGS>(args)...) { 
        }


    void attach(ChannelReader<T,SIM> &&inChan) {
        // TODO: Make inChannels a containter that never copies.
        inChannels.push_back(std::move(inChan));
    }


    // step to move this object forward until it blocks
    void step() {
        bool hasMoved = false;
        while(processNextEvent()) { hasMoved = true; }
        if(hasMoved) this->execCallbacks();
        if(inChannels.empty()) delete(this);
    }

    const SpaceTime &position() { return this->pos; }

protected:

    bool processNextEvent() {
        if(inChannels.empty()) return false;
        // Find earliest channel
        int idx = 0;
        int earliestChannelIdx = 0;
        auto earliestIntersectionTime = intersectionTime(inChannels[idx]);
        ++idx;
        while(idx != inChannels.size()) {
            auto intersectTime = intersectionTime(inChannels[idx]);
            if(intersectTime <= earliestIntersectionTime) {
                earliestIntersectionTime = intersectTime;
                earliestChannelIdx = idx;
            }
            ++idx;
        }

        // Process earliest channel
        this->pos += this->velocity * (earliestIntersectionTime + PROCESSINGTIME);
//        std::cout << this << " Moved to " << this->position << std::endl;
        bool notBlocking = inChannels[earliestChannelIdx].executeNext();
        if(inChannels[earliestChannelIdx].isClosed()) { // remove closed channel from inChannels
            if(earliestChannelIdx != inChannels.size()-1) inChannels[earliestChannelIdx] = std::move(inChannels.back());
            inChannels.pop_back();
        }
        return notBlocking;
    }


    // intersection of this with a channel
    Time intersectionTime(const ChannelReader<T,SIM> &inChan) {
        return std::max(
            (inChan.position() - this->position()) / this->velocity
            ,-PROCESSINGTIME
            );
    }
};

#endif
