#ifndef SPATIALFUNCTION_H
#define SPATIALFUNCTION_H

#include <functional>
#include "ReferenceFrame.h"

template<class T, ReferenceFrame FRAME>
class SpatialFunction {
    public:
    FRAME::SpaceTime                    position;   // position on emission
    std::function<void(T &, FRAME &)>   event;       // returns new velocity of agent

    SpatialFunction(SPACETIME position, std::function<SPACETIME(T &)> event) : position(std::move(position)), event(std::move(event)) {}
    SpatialFunction(SPACETIME position) : position(std::move(position)) {} // create a blocking entry

    bool hasEvent() { return (bool)event; }

    void operator()(SpaceTimeObject<T,FRAME> &agent) { event(agent); }

};

#endif
