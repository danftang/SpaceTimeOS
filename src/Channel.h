#ifndef CHANNEL_H
#define CHANNEL_H


#include <deque>
#include <functional>
#include "SpatialFunction.h"
//#include "SpaceTimeObject.h"
#include "ThreadPool.h"
#include "ReferenceFrame.h"

template<class T, ReferenceFrame FRAME>
class Channel {
public:
    typedef typename FRAME::SpaceTime SpaceTime;

    Channel(const SpaceTimeBase<FRAME> &source) : source(&source) {}
    Channel() : source(nullptr) {} // creates a closed channel

    // reader interface
    bool pop() { // returns true if blocking
        bool isBlocking = (isOpen() && buffer.front().position() == source->position());
        buffer.pop_front();
        if(isBlocking) source->callbackOnMove(target);
        return isBlocking;
    }
    const SpatialFunction<T,SpaceTime> &front();
    FRAME::SpaceTime position() {
        
    }

    // writer interface
    void push(SpatialFunction<T,SpaceTime> &&event) {
        buffer.push_back(std::move(event));
    }

    void open(SpaceTimeBase<SPACETIME> &source) { this->source = &source; }
    void close() { source = nullptr; }
    bool isOpen() { return source != nullptr; }

protected:
    const SpaceTimeBase<SPACETIME> *source; // null if closed (i.e. source has died)
    SpaceTimeObject<T,SPACETIME> &target;
    std::deque<SpatialFunction<T,SPACETIME>> buffer;
};

template <class T, ReferenceFrame FRAME>
const SpatialFunction<T, FRAME> & Channel<T, FRAME>::front()
{
    if (buffer.isEmpty()) {
        if (isOpen()) {
            buffer.push_back(SpatialFunction<T, SPACETIME>(source->position));
        } else {
            buffer.push_back(SpatialFunction<T, SPACETIME>(SPACETIME::top()));
        }
    }
    return buffer.front();
};
#endif

