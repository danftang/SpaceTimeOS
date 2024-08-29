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

    SpaceTimeBase(const SpaceTimeBase<SPACETIME> &other) = delete; // Just don't copy objects
    SpaceTimeBase(SpaceTimeBase<SPACETIME> &&other) = delete;

    SpaceTimeBase(const SPACETIME &position) : position(position) {
//        std::cout << this << " Creating object at " << this->position << std::endl;
    }

    SpaceTimeBase(SPACETIME &&position) : position(std::move(position)) {
//        std::cout << this << " Move-creating object at " << this->position << std::endl;
    }

    ~SpaceTimeBase() { execCallbacks(); } // make sure all writeChannels are unblocked
    

    void callbackOnMove(std::function<void()> callback) {
        std::cout << this << " Adding callback" << std::endl;
        callOnMove.push(std::move(callback));
    }

    void execCallbacks() {
        std::cout << this << " executing callbacks " << std::endl;
        while(!callOnMove.empty()) {
            executor.submit(std::move(callOnMove.front()));
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
        object(std::forward<ARGS>(args)...) { 
        }


    void attach(ChannelReader<T,FRAME> &&inChan) {
        // TODO: Make inChannels a containter that never copies.
        inChannels.push_back(std::move(inChan));
    }


    // step to move this object forward until it blocks
    void step() {
        std::cout << this << " starting step" << std::endl;
        bool hasMoved = false;
        while(processNextEvent()) { hasMoved = true; }
        if(hasMoved) {
//            std::cout << this << " submitting " << this->callOnMove.size() << " callbacks " << std::endl;
            this->execCallbacks();
        } else {
//            std::cout << this << " exiting step without moving" << std::endl;
        }

        // while(processNextEvent() && this->position.isWithinBounds()){
        //     hasMoved = true;
        // }
        // if(hasMoved) {
        //     std::cout << this << " submitting " << this->callOnMove.size() << " callbacks " << std::endl;
        //     this->execCallbacks();
        // } else {
        //     std::cout << this << " exiting step without moving" << std::endl;
        // }
        // if(!this->position.isWithinBounds() || inChannels.empty()) {
        //     // kill this object if it falls off the spacetime or has no chance of executing any more code.
        //     std::cout << "Deleting object " << this << std::endl;
        //     delete(this);
        // }


    }


protected:

    bool processNextEvent() {
        assert(!inChannels.empty()); // object should have been deleted if no inChannels
        // Find earliest channel
//        std::cout << this << " finding nearest channel" << std::endl;
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
//        std::cout << "Chose channel " << earliestChannelIdx << std::endl;

        // Process earliest channel
        this->position = this->frameOfReference.positionAfter(this->position, earliestIntersectionTime + PROCESSINGTIME);
        std::cout << this << " Moving to " << this->position << std::endl;
        bool notBlocking = inChannels[earliestChannelIdx].executeNext();
//        std::cout << (foundEvent?"Executed event":"No event found") << std::endl;
        if(inChannels[earliestChannelIdx].isClosed()) { // remove closed channel from inChannels
//            std::cout << "Removing closed channel" << std::endl;
            if(earliestChannelIdx != inChannels.size()-1) inChannels[earliestChannelIdx] = std::move(inChannels.back());
            inChannels.pop_back();
        }
        return notBlocking;
    }


    // intersection of this with a channel
    Time intersectionTime(const ChannelReader<T,FRAME> &inChan) {
        return std::max(this->frameOfReference.intersection(inChan.position(), this->position),-PROCESSINGTIME);
    }
};



#endif
