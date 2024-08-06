// A ptr can be captured in a lambda or held in an object. If captured in a lambda the blocking position should be
// the position of emission until called, at which point it should be the position of the target object,
// if held in an object, the blocking position should be that of the object.

#ifndef SPACETIMEPTR_H
#define SPACETIMEPTR_H

#include <functional>
#include "Channel.h"
#include "SpaceTimeObject.h"

template<class T, class SPACETIME>
class SpaceTimePtr {
public:
    SpaceTimePtr(const SpaceTimePtr &)=delete; // no copy constructor
    SpaceTimePtr(const SpaceTimeBase<SPACETIME> &source, SpaceTimeObject<T,SPACETIME> &target) :
    channel(target.addChannel(source)) { }

    // use this to make a new space-time agent (which is responsible for its own deletion)
    SpaceTimePtr(const SpaceTimeBase T &&obj) {
        auto pObj = new SpaceTimeObject<T,SPACETIME>(std::move(obj));
        pObj->
    }

    void exec(SpatialFunction<T,SPACETIME> &sFunction); // code takes the target and returns a new 4-velocity

    
protected:
    Channel<SPACETIME> &  channel;
//    SpaceTimeObject<T,SPACETIME> &       target;
};
#endif
