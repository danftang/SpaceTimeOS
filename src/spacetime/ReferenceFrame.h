#ifndef REFERENCEFRAME_H
#define REFERENCEFRAME_H

#include <concepts>


template<class FRAME>
concept ReferenceFrame = requires(FRAME frame, FRAME::SpaceTime position, FRAME::Time time) {
    // The earliest positive time at which an agent starting at position2 and moving with this reference frame 
    // enters the future light cone of a call originating at position1. A negative time means no intersection.
    { frame.intersection(position, position) } -> std::convertible_to<typename FRAME::Time>;

    // The position of an agent starting at a given position and moving in the frame for a given time
    { frame.positionAfter(position, time) } -> std::convertible_to<typename FRAME::SpaceTime>;

    // // Every reference frame has a laboratory which provides a default reference frame and 
    // { FRAME::laboratory } -> std::convertible_to<FRAME>; 


};

#endif
