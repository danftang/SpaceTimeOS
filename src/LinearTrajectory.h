#ifndef LINEARTRAJECTORY_H
#define LINEARTRAJECTORY_H

// A trajectory defines a map, T(s), from a scalar, s, to points in a spacetime.
// The scalar gives the distance along the trajectory from an origin, as defined by the
// spacetime's dot product. This also defines the local time of the agent.
// Given a higher dimensional manifold, there may exist a point at which the
// trajectory first intersects the manifold (i.e. lowest non-negative scalar).
// If we define a manifold as the points of a scalar field, M, such that M(X)=0,
// then the intersection point becomes finding the roots of M(T(s)),
// which can be done with Newton descent...?
//
// Or, more generally, we can define a subset of a space as the points where
// a field is non-negative and find the earliest point on the trajectory
// that is non-negative. [Perhaps we could add the requirement that the
// negative subset is convex - or at least a set of disjoint convex sets
// or even more generally, perhaps we have a notion of any type of trajectory/manifold
// pair on which a timeToIntersection is defined...or a manifold becomes a
// probability-rate (per unit distance) field, and timeToInteraction becomes a sample from a 
// poisson process]
//
template<class SPACETIME>
class LinearTrajectory {
    typedef SPACETIME              SpaceTime;
    typedef SPACETIME::Scalar      Scalar;

    SpaceTime               pos;
    SpaceTime::Velocity     vel;

public:
    template<class MANIFOLD>
    Scalar timeToIntersection(MANIFOLD &manifold) {

    }

    void moveOriginTo(Scalar time) {

    }

    const SpaceTime &origin() { return pos; }

    
};

#endif
