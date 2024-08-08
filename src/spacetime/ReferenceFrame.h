#ifndef REFERENCEFRAME_H
#define REFERENCEFRAME_H

#include <concepts>

template<class FRAME>
concept ReferenceFrame = requires(FRAME frame, FRAME::SpaceTime position) {
    // The earliest time at which the spatial origin of the reference frame intersects the future of the given position. 
    { frame.intersection(position, position) } -> std::convertible_to<typename FRAME::SpaceTime>;
};

#endif
