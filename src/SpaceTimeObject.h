#ifndef SPACETIMEOBJECT_H
#define SPACETIMEOBJECT_H

#include <vector>
#include <list>
#include <queue>
#include "Channel.h"
#include "spacetime/ReferenceFrame.h"
#include "ThreadPool.h"

// template<SpaceTime SPACETIME>
// class ChannelWriter {
//     SPACETIME position;
//     std::queue<std::function<void()>>   callOnMove;

//     template<class P>
//     ChannelWriter(P &&position) : 
//         position(std::forward<P>(position)) {}

//     void callbackOnMove(std::function<void()> callback) {
//         callOnMove.push(std::move(callback));
//     }

//     void execCallbacks() {
//         while(!callOnMove.empty()) {
//             executor.submit(callOnMove.front());
//             callOnMove.pop();
//         }
//     }
// };

template<SpaceTime SPACETIME>
class SpaceTimeBase {
public:
    SPACETIME                           position;
    std::queue<std::function<void()>>   callOnMove;

    SpaceTimeBase(const SpaceTimeBase<SPACETIME> &other) = default;

    SpaceTimeBase(SpaceTimeBase<SPACETIME> &&other) = default;

    SpaceTimeBase(const SPACETIME &position) : position(position) {}

    SpaceTimeBase(SPACETIME &&position) : position(std::move(position)) {}

    void callbackOnMove(std::function<void()> callback) {
        callOnMove.push(std::move(callback));
    }

    void execCallbacks() {
        while(!callOnMove.empty()) {
            executor.submit(callOnMove.front());
            callOnMove.pop();
        }
    }
};


template<class T, ReferenceFrame FRAME>
class SpaceTimeObject : public SpaceTimeBase<typename FRAME::SpaceTime> {
protected:
    std::vector<ChannelReader<T,FRAME>>  inChannels;

public:
    T       object;
    FRAME   frameOfReference;

    typedef FRAME::SpaceTime SpaceTime;

    template<class P, class F, class... ARGS>
    SpaceTimeObject(P &&position, F &&frame, ARGS &&... args) : 
        SpaceTimeBase<SpaceTime>(std::forward<P>(position)),
        frameOfReference(std::forward<F>(frame)),
        object(std::forward<ARGS>(args)...) { }


    void attach(ChannelReader<T,FRAME> &&inChan) {
        inChannels.emplace_back(std::move(inChan));
    }


    // step to move this object forward until it blocks
    void operator()() {
        bool hasMoved = false;
        while(processNextEvent() && this->position.isWithinBounds()){
            hasMoved = true;
        }
        if(hasMoved) this->execCallbakcs();
        if(!this->position.isWithinBounds() || inChannels.empty()) {
            // kill this object if it falls off the spacetime or has no chance of executing any more code.
            delete(this); 
        }
    }


protected:

    bool processNextEvent() {
        if(inChannels.empty()) return false;
        auto chan = inChannels.beigin();
        auto earliestChannel = chan;
        auto earliestIntersection = intersection(*chan);
        ++chan;
        while(chan != inChannels.end()) {
            auto intersection = intersection(*chan);
            if(intersection <= earliestIntersection) {
                earliestIntersection = intersection;
                earliestChannel = &chan;
            }
            ++chan;
        }
        this->position = this->frameOfReference.positionAfter(this->position, earliestIntersection);
        bool foundEvent = !earliestChannel->empty();
        if(foundEvent) {
            earliestChannel->executeNext();
            if(earliestChannel->empty() && !earliestChannel->isOpen()) {
                // we've just executed the last event on a closed channel, so erase channel
                if(&*earliestChannel != &inChannels.back()) *earliestChannel = std::move(inChannels.back());
                inChannels.pop_back();
            }
        } else {
            earliestChannel->setBlockingCallback();
        }
        return foundEvent;
    }

    // intersection of this with a channel
    FRAME::SpaceTime intersection(const ChannelReader<T,FRAME> &inChan) {
        typename FRAME::SpaceTime *pchanpos = inChan.positionPtr();
        return pchanpos == nullptr ? this->position : this->frameOfReference.intersect(*pchanpos, this->position);
    }
};



#endif
