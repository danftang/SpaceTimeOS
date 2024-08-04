#include <deque>
#include <functional>
#include "SpatialFunction.h"
//#include "SpaceTimeObject.h"

template<class T, class SPACETIME>
class Channel {
public:
    Channel(const SpaceTimeBase<SPACETIME> &source) : source(&source) {}
    Channel() : source(nullptr) {} // creates a closed channel

    // reader interface
    const SPACETIME &position() {
        if(bufer.isEmpty()) return sender->position;
        return front().position;
    }
    void pop();
    const SpatialFunction<T,SPACETIME> &front();

    // writer interface
    void push(SpatialFunction<T,SPACETIME> &&event) {
        buffer.push_back(std::move(event));
        if(buffer.size() == 1 && event.position < target.position) {
            // if we're unblocking, send a signal to the target

        }
    }

    void open(SpaceTimeBase<SPACETIME> &source) { this->source == &source; }
    bool isClosed() { return source == nullptr; }
    void close() { source == nullptr; }


protected:
    const SpaceTimeBase<SPACETIME> *source; // null if closed (i.e. source has died)
    SpaceTimeObject<T,SPACETIME> &target;
    std::deque<SpatialFunction<T,SPACETIME>> buffer;
};
