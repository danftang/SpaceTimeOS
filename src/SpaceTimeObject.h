#ifndef SPACETIMEOBJECT_H
#define SPACETIMEOBJECT_H

#include <vector>
#include <list>
#include <queue>
#include "Channel.h"
#include "spacetime/ReferenceFrame.h"
#include "ThreadPool.h"
#include "OutChannel.h"
#include "InChannel.h"

template<ReferenceFrame FRAME>
class SpaceTimeBase {

public:
    FRAME::SpaceTime    position;
    FRAME               frameOfReference;
    std::queue<std::function<void()>>   callOnMove;

    template<class P, class F>
    SpaceTimeBase(P &&position, F &&frame) : 
        position(std::forward<P>(position)), 
        frameOfReference(std::forward<F>(frame)) {}

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
class SpaceTimeObject : public SpaceTimeBase<FRAME> {
protected:
    std::vector<InChannel<T,FRAME>>   inChannels; // TODO: make InChannel object that closes the channel on deletion

public:
    T object;

    template<class P, class F, class... ARGS>
    SpaceTimeObject(P &&position, F &&frame, ARGS &&... args) : 
        SpaceTimeBase<FRAME>(std::forward<P>(position), std::forward<F>(frame)),
        object(std::forward<ARGS>(args)...) { }


    void connect(InChannel<T,FRAME> &&inChannel) {
        inChannel.open(*this);
        inChannels.push_back(std::move(inChannel));
    }

    void disconnect(InChannel<T,FRAME> &inChannel) {
        // this should always be the currently active channel from processNextEvent
    }


    // step to move this object forward until it blocks
    void operator()() {
        bool hasMoved = false;
        while(processNextEvent() && this->position.isWithinBounds()){
            hasMoved = true;
        }
        if(hasMoved) this->execCallbakcs();
        if(!this->position.isWithinBounds()) delete(this);
    }


protected:

    bool processNextEvent() {
        if(inChannels.empty()) return false;
        auto chan = inChannels.beigin();
        auto earliestChannel = chan;
        auto earliestIntersection = this->frameOfReference.intersect(chan->position(), this->position);
        ++chan;
        while(chan != inChannels.end()) {
            auto intersection = this->frameOfReference.intersect(chan->position(), this->position);
            if(intersection <= earliestIntersection) {
                earliestIntersection = intersection;
                earliestChannel = &chan;
            }
            ++chan;
        }
        this->position = this->frameOfReference.positionAfter(this->position, earliestIntersection);
        bool foundEvent = !earliestChannel->empty();
        if(foundEvent) {
            earliestChannel->front()(SpaceTimePtr(this)); // execute event
            earliestChannel->pop();
            if(earliestChannel->empty() && !earliestChannel->isOpen()) {
                // erase channel
                if(inChannels.size() == 1) {
                    inChannels.clear();
                } else {
                    *earliestChannel = std::move(inChannels.back()); // TODO: can't move channels as this would invalidate ChannelRefs
                    inChannels.pop_back();
                }
            }
        } else {
            earliestChannel->setBlockingCallback();
        }
        return foundEvent;
    }
};



#endif
