#ifndef CHANNEL_H
#define CHANNEL_H


#include <deque>
#include <functional>
#include "SpatialFunction.h"
//#include "SpaceTimeObject.h"
#include "ThreadPool.h"
#include "spacetime/ReferenceFrame.h"

template<ReferenceFrame FRAME> class SpaceTimeBase;
template<class T, ReferenceFrame FRAME> class SpaceTimeObject;

template<class T, ReferenceFrame FRAME>
class Channel {
protected:
    const SpaceTimeBase<FRAME> *            source; // null if closed (i.e. source has died)
    SpaceTimeObject<T,FRAME> &              target;
    std::deque<SpatialFunction<T,FRAME>>    buffer;

public:
    typedef typename FRAME::SpaceTime SpaceTime;

    Channel(const SpaceTimeBase<FRAME> *sourcePtr, SpaceTimeObject<T,FRAME> &target) : source(sourcePtr), target(target) {}
    Channel(SpaceTimeObject<T,FRAME> &target) : Channel(nullptr,target) {}

    // reader(target) interface
    bool pop() { // returns true if blocking
        bool isBlocking = (isOpen() && buffer.front().position() == source->position());
        buffer.pop_front();
        if(isBlocking) source->callbackOnMove(target);
        return isBlocking;
    }
//    const SpatialFunction<T,FRAME> &front();

    // error if closed and empty
    FRAME::SpaceTime position() {
        return buffer.empty()?source->position : buffer.front().position;
    }


    bool isOpen() const { return source != nullptr; }

    // writer(ChannelRef) interface
    void send(SpatialFunction<T,FRAME> &&event) {
        buffer.push_back(std::move(event));
    }

    void open(SpaceTimeBase<FRAME> &source) { this->source = &source; }
    void close() { source = nullptr; }

};

// template <class T, ReferenceFrame FRAME>
// const SpatialFunction<T, FRAME> & Channel<T, FRAME>::front()
// {
//     if (buffer.isEmpty()) {
//         if (isOpen()) {
//             buffer.push_back(SpatialFunction<T, FRAME>(source->position));
//         } else {
//             buffer.push_back(SpatialFunction<T, FRAME>(SPACETIME::top()));
//         }
//     }
//     return buffer.front();
// };
#endif

