#ifndef SPATIALFUNCTION_H
#define SPATIALFUNCTION_H

#include <functional>
#include "spacetime/ReferenceFrame.h"

template<class T, ReferenceFrame FRAME> class SpaceTimePtr;

template<class T, ReferenceFrame FRAME>
class SpatialFunction {
    public:
    FRAME::SpaceTime                            position;   // position on emission
    std::function<void(SpaceTimePtr<T,FRAME>)>  function;   // call to execute

    template<class LAMBDA>
    SpatialFunction(const FRAME::SpaceTime &position, LAMBDA &&lambda) : position(position), function(std::forward<LAMBDA>(lambda)) {}

//    void operator()(SpaceTimePtr<T,FRAME> agentPtr) { function(agentPtr); }

};

#endif
