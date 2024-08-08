#ifndef SPATIALFUNCTION_H
#define SPATIALFUNCTION_H

#include <functional>
#include "spacetime/ReferenceFrame.h"

template<class T, ReferenceFrame FRAME> class SpaceTimePtr;

template<class T, ReferenceFrame FRAME>
class SpatialFunction {
    public:
    FRAME::SpaceTime                            position;   // position on emission
    std::function<void(SpaceTimePtr<T,FRAME>)>  event;       // returns new velocity of agent

    SpatialFunction(FRAME::SpaceTime position, std::function<void(SpaceTimePtr<T,FRAME>)> event) : position(std::move(position)), event(std::move(event)) {}
    SpatialFunction(FRAME::SpaceTime position) : position(std::move(position)) {} // create a blocking entry

    bool hasEvent() { return (bool)event; }

    void operator()(SpaceTimePtr<T,FRAME> &agentPtr) { event(agentPtr); }

};

#endif
