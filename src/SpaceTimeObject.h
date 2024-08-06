#include <vector>
#include <list>
#include <queue>
#include "Channel.h"

template<class SPACETIME>
class SpaceTimeBase {
public:
    SPACETIME position;
    SPACETIME velocity;
    std::list<SpaceTimeBase<SPACETIME> *>          objectsImBlocking;

};


template<class T, class SPACETIME>
class SpaceTimeObject : public T {
public:
    std::vector<Channel<T,SPACETIME>>   inChannels;
    std::list<Steppable *>          objectsImBlocking;

    SpaceTimeBase<SPACETIME> spacetime;
public:

    SpaceTimeObject(T &&obj) : object(std::move(obj)) { }

    Channel<SPACETIME> &addChannel(const SpaceTimeBase<SPACETIME> &source) {
        inChannels.push_back(Channel<T,SPACETIME>(source));
    }

    // step to move this object forward until it blocks
    void operator () {
        bool isBlocking;
        do {
            Channel<T,SPACETIME> & channelQueue.top().
            channelQueue.pop();

        } while(!isBlocking && !channelQueue.is);
    }

    bool processNextEvent() {
        for(Channel<T,SPACETIME> &chan : inChannels) {
            chan
            // |Ds + vt| = 0
        }
    }



};
