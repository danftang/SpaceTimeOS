#ifndef SPACETIMEOBJECT_H
#define SPACETIMEOBJECT_H

#include <vector>
#include <list>
#include <queue>
#include "Channel.h"
#include "spacetime/ReferenceFrame.h"
#include "ThreadPool.h"
#include "ChannelRef.h"

template<ReferenceFrame FRAME>
class SpaceTimeBase {

public:
    FRAME               frameOfReference;
    FRAME::SpaceTime    position;
    std::queue<std::function<void()>>   callOnMove;

    SpaceTimeBase(FRAME frame) : frameOfReference(std::move(frame)) {}

    void callbackOnMove(std::function<void()> callback) {
        callOnMove.push(std::move(callback));
    }

    void execCallbakcs() {
        while(!callOnMove.empty()) {
            executor.submit(callOnMove.front());
            callOnMove.pop();
        }
    }


};

template<class T, ReferenceFrame FRAME>
class SpaceTimeObject : public SpaceTimeBase<FRAME> {
protected:
    std::vector<Channel<T,FRAME>>   inChannels;

public:
    T object;

    SpaceTimeObject(T &&obj, FRAME frame) : SpaceTimeBase<FRAME>(std::move(frame)), object(std::move(obj)) { }

    // default T constructor 
    SpaceTimeObject(FRAME frame) : SpaceTimeBase<FRAME>(std::move(frame)) { }


    // ChannelRef<T,FRAME> addChannel(const SpaceTimeBase<FRAME> &source) {
    //     return inChannels.emplace_back(source, *this);
    // }

    ChannelRef<T,FRAME> newChannel() {
        return inChannels.emplace_back(*this);
    }


    // step to move this object forward until it blocks
    void operator()() {
        bool hasMoved = false;
        while(processNextEvent()){
            hasMoved = true;
        }
        if(hasMoved) this->execCallbakcs();
        
    }


protected:
    bool processNextEvent() {
        typename FRAME::SpaceTime earliestIntersection;
        Channel<T,FRAME> *earliestChannel = nullptr;
        for(Channel<T,FRAME> &chan : inChannels) {
            typename FRAME::SpaceTime intersection = this->frameOfReference.intersect(chan.position(), this->position);
            if(earliestChannel == nullptr || intersection <= earliestIntersection) {
                earliestIntersection = intersection;
                earliestChannel = &chan;
            }
        }
        this->position = earliestIntersection;
        earliestChannel->front()(this);
        return !earliestChannel.pop();     // returns true if not blocking 
    }
};



#endif
