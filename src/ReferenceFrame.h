#ifndef REFERENCEFRAME_H
#define REFERENCEFRAME_H

template<class FRAME>
concept ReferenceFrame = requires(FRAME frame, FRAME::SpaceTime position) {
    // The earliest time at which the spatial origin of the reference frame intersects the future of the given position. 
    { frame.intersection(position) } -> std::convertible_to<typename FRAME::SpaceTime>;
    { frame.setPosition(position) };
    
};


template<class SPACETIME>
class InertialReferenceFrame {
public:
    typedef SPACETIME SpaceTime;

    SPACETIME   origin;
    SPACETIME   velocity; // 4-velocity unit vector

    SPACETIME intersection(SPACETIME &callPosition) {
        // |origin + time*velocity - callPosition| = 0
        double time = (origin - callPosition) / velocity;
        return origin + time*velocity;
    }
};
#endif
