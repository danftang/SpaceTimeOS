#ifndef VELOCITY_H
#define VELOCITY_H

#include "Concepts.h"

// An inner product space defines a subset of velocities {V : V.V=1} 
// 
// The Velocity class acts as a tag on the SpaceTime type that certifies 
// that the highest differential along v is 1 in the field, so we needn't calculate it.
//
// Integer spacetimes:
// If all dimensions of the spacetime are integer, then local time is integer and
// the trajectory consists of points x + vi for some integer i.
// Intersection with a field occurs on the first integer time in the positive region.
// If the length of a fully integer velocity is not necessarily 1 we should just
// represent it as a spacetime type, not explicitly as a Velocity and say that
// proper velocities do not exist in this space and hence consecutive points on a
// trajectory need not be separated by distance 1.
// A Velocity of an integer spacetime states we are restricting ourselves to
// unit velocities within the integer vector space.
//
// Mixed integer spacetimes:
// If the spacetime is mixed floating point/integer, then we have we have two possibilities:
// if local time is integer, then trajectories consist of discrete points separated by distance 1.
// If local time is floating-point and the velocity has non-zero integer components, then
// then the trajectory should follow the projection of the real trajectory to its integer rounding
// with jumps at light speed between integer points, centred about the point
// where the real trajectory would switch the integer point it would round to.
// In this way, we respect locality while maintining a fully defined * operator on the
// vector field.
//
// Addition of velocities: If we assume every agent has unit mass, then the 4-velocity
// is the 4-momentum (energy-momentum) of the agent. If we require that in an interaction
// the total 4-momentum is conserved, and that the rest-mass of the agents is not
// changed (i.e. no nuclear reactions between agents) then we have, in 2D for example...
//

//
template<SpaceTime SPACETIME>
class Velocity : public SPACETIME {
public:
    Velocity() : typename SPACETIME(1) { }

    // We can map all points in spacetime onto the velocity manifold by projecting
    // along lab-energy (lab-time) until we reach unit inner product. If time is
    // integer, there may be more than one value that along this projection that has
    // unit inner product, in which case we take the earliest.
    

};

// In a 1D space there is only one velocity: forward in time so we need not explicitly store it
template<SpaceTime SPACETIME> requires (SPACETIME::DIMENSIONS=1)
class Velocity<SPACETIME> {
public:
};

#endif
