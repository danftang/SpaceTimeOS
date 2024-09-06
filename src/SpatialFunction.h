#ifndef SPATIALFUNCTION_H
#define SPATIALFUNCTION_H

#include <functional>
#include "Concepts.h"


template<class T, Simulation SIM> class SpaceTimePtr;
template<class T, class SIM> class SpaceTimeObject;

template<class T, Simulation SIM>
class SpatialFunction {
public:
    typedef SIM::SpaceTime SpaceTime;

    SpaceTime                                   position;   // position on emission
    std::function<void(SpaceTimePtr<T,SIM>)>    function;   // call to execute
    

    template<class LAMBDA>
    SpatialFunction(const SpaceTime &position, LAMBDA &&lambda) : position(position), function(std::forward<LAMBDA>(lambda)) {}

    void operator()(SpaceTimeObject<T,SIM> &agent) { function(SpaceTimePtr<T,SIM>(&agent)); }
};

#endif
