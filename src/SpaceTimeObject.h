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

    virtual void step()=0;
};


template<class T, class SPACETIME>
class SpaceTimeObject : public SpaceTimeBase<SPACETIME> {
protected:
    std::priority_queue<Channel<T,SPACETIME>> channelQueue;
//    std::list<Channel<T,SPACETIME>>   inChannels;
    std::list<Steppable *>          objectsImBlocking;
    T object;                                   // the object we're wrapping

public:
    SPACETIME position;
    SPACETIME velocity;

    Channel<SPACETIME> &addChannel(const SpaceTimeBase<SPACETIME> &source) {
        inChannels.push_back(Channel<T,SPACETIME>(source));
    }
    void step() { // moves this object forward until it blocks.

    }

    bool processNextEvent() {
    }

};
