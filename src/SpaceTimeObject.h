#ifndef SPACETIMEOBJECT_H
#define SPACETIMEOBJECT_H

#include <vector>
#include <list>
#include <queue>
#include "Channel.h"
#include "ReferenceFrame.h"
#include "ThreadPool.h"

template<ReferenceFrame FRAME>
class SpaceTimeBase {
public:
    typedef typename FRAME::SpaceTime SPACETIME;
    std::queue<std::function<void()>>   callOnMove;
    FRAME frameOfReference;

    void callbackOnMove(std::function<void()> callback) {
        callOnMove.push_back(std::move(callback));
    }


};

template<class T, ReferenceFrame FRAME>
class SpaceTimeObject : public SpaceTimeBase<FRAME> {
public:

    SpaceTimeObject

    T object;

    std::vector<Channel<T,FRAME::SpaceTime>>   inChannels;


    SpaceTimeObject(T &&obj) : T(std::move(obj)) { }

    Channel<SPACETIME> &addChannel(const SpaceTimeBase<FRAME> &source) {
        inChannels.push_back(Channel<T,SPACETIME>(source));
    }

    // step to move this object forward until it blocks
    void operator()() {
        bool hasMoved = false;
        while(processNextEvent()){
            hasMoved = true;
        }
        if(hasMoved) {
            while(!callOnMove.empty()) {
                executor.submit(callOnMove.front());
                callOnMove.pop();
            }
        }
    }

    bool processNextEvent() {
        SPACETIME earliestIntersection = SPACETIME::top();
        Channel<T,SPACETIME> *earliestChannel = nullptr;
        for(Channel<T,SPACETIME> &chan : inChannels) {
            SPACETIME intersection = frameOfReference.intersect(chan.position());
            if(intersection < earliestIntersection) {
                earliestIntersection = intersection;
                earliestChannel = &chan;
            }
        }
        frameOfReference.setPosition(earliestIntersection);
        earliestChannel->front()(object, frameOfReference);
        return !earliestChannel.pop();     // returns true if not blocking 
    }
};



#endif
