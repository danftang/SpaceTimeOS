#ifndef SPATIALFUNCTION_H
#define SPATIALFUNCTION_H

#include <functional>
#include "Concepts.h"


template<class T>
class SpatialFunction : public std::function<void(T &)> {

public:
    typedef T::SpaceTime SpaceTime;
    typedef T::SpaceTime::Time Time;
    
    template<class LAMBDA>
    SpatialFunction(const SpaceTime &position, LAMBDA &&lambda) : 
        std::function<void(T &)>(std::forward<LAMBDA>(lambda)),
        pos(position) { 
        }

    const SpaceTime &position() { return pos; }

    // Scalar timeToIntersection(const SpaceTime &agentPosition, const SpaceTime &agentVelocity) {
    //     return (pos - agentPosition) / agentVelocity;
    // }

protected:
    SpaceTime                   pos;   // position on emission
};

#endif
