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

    ~SpaceTimeBase() { execCallbacks(); } // make sure all writeChannels are unblocked
    

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
    typedef FRAME::SpaceTime SpaceTime;
    typedef FRAME::Time Time;

    T       object;
    FRAME   frameOfReference;

    static constexpr Time PROCESSINGTIME = 0.1; // local time between spatial intersection with a call and actually calling it


    template<class P, class F, class... ARGS>
    SpaceTimeObject(P &&position, F &&frame, ARGS &&... args) : 
        SpaceTimeBase<SpaceTime>(std::forward<P>(position)),
        frameOfReference(std::forward<F>(frame)),
        object(std::forward<ARGS>(args)...) { }


    void attach(ChannelReader<T,FRAME> &&inChan) {
        inChannels.emplace_back(std::move(inChan));
    }


    // step to move this object forward until it blocks
    void step() {
        bool hasMoved = false;
        while(processNextEvent() && this->position.isWithinBounds()){
            hasMoved = true;
        }
        if(hasMoved) this->execCallbacks();
        if(!this->position.isWithinBounds() || inChannels.empty()) {
            // kill this object if it falls off the spacetime or has no chance of executing any more code.
            delete(this); 
        }
    }


protected:

    bool processNextEvent() {
        if(inChannels.empty()) return false;
        // Find earliest channel
        auto chan = inChannels.begin();
        auto earliestChannel = chan;
        auto earliestIntersectionTime = intersectionTime(*chan);
        ++chan;
        while(chan != inChannels.end()) {
            auto intersectTime = intersectionTime(*chan);
            if(intersectTime <= earliestIntersectionTime) {
                earliestIntersectionTime = intersectTime;
                earliestChannel = chan;
            }
            ++chan;
        }

        // Process earliest channel
        this->position = this->frameOfReference.positionAfter(this->position, earliestIntersectionTime + PROCESSINGTIME);
        std::cout << "Moving " << this << " to " << this->position << std::endl;
        bool foundEvent = earliestChannel->executeNext();
        if(earliestChannel->isClosed()) { // remove closed channel from inChannels
            if(&*earliestChannel != &inChannels.back()) *earliestChannel = std::move(inChannels.back());
            inChannels.pop_back();
        }
        return foundEvent;
    }

    // intersection of this with a channel
    Time intersectionTime(const ChannelReader<T,FRAME> &inChan) {
        return std::max(this->frameOfReference.intersection(inChan.position(), this->position),-PROCESSINGTIME);
    }
};



#endif
