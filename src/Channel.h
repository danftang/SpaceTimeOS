#ifndef CHANNEL_H
#define CHANNEL_H


#include <deque>
#include <functional>
#include "SpatialFunction.h"
//#include "SpaceTimeObject.h"
#include "ThreadPool.h"

template<class T, class SPACETIME>
class Channel {
public:
    Channel(const SpaceTimeBase<SPACETIME> &source) : source(&source) {}
    Channel() : source(nullptr) {} // creates a closed channel

    // reader interface
    bool pop() { // returns true if blocking
        isBlocking = (isOpen() && buffer.front().position() == source->position());
        buffer.pop_front();
        return isBlocking;
    }
    const SpatialFunction<T,SPACETIME> &front();
    void callbackOnNextWrite(std::function<void()> &&callback) { nextWriteCallback = std::move(callback); }
    bool operator < (const Channel<T,SPACETIME> &other) { return front().position() < other.front().position(); }

    // writer interface
    void push(SpatialFunction<T,SPACETIME> &&event) {
        buffer.push_back(std::move(event));
        if(isBlocking) {
            executor.submit(target);
            isBlocking = false;
        }
    }

    void open(SpaceTimeBase<SPACETIME> &source) { this->source = &source; }
    void close() { source = nullptr; }
    bool isOpen() { return source != nullptr; }

protected:
    const SpaceTimeBase<SPACETIME> *source; // null if closed (i.e. source has died)
    bool isBlocking;
    SpaceTimeObject<T,SPACETIME> &target;
    std::deque<SpatialFunction<T,SPACETIME>> buffer;
};

template <class T, class SPACETIME>
const SpatialFunction<T, SPACETIME> & Channel<T, SPACETIME>::front()
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

