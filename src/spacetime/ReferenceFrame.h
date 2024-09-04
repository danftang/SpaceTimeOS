#ifndef REFERENCEFRAME_H
#define REFERENCEFRAME_H

#include <concepts>
#include "SpaceTime.h"

template<class FRAME>
concept ReferenceFrame = requires(FRAME frame, FRAME::SpaceTime position, FRAME::SpaceTime::ScalarType time) {
    typename FRAME::SpaceTime;
    requires SpaceTime<typename FRAME::SpaceTime>;

    // The earliest positive time at which an agent starting at position2 and moving with this reference frame 
    // enters the future light cone of a call originating at position1. A negative time means no intersection.
    { frame.intersection(position, position) } -> std::convertible_to<typename FRAME::SpaceTime::ScalarType>;

    // The position of an agent starting at a given position and moving in the frame for a given time
    { frame.positionAfter(position, time) } -> std::convertible_to<typename FRAME::SpaceTime>;
};

#endif
