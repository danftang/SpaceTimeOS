#ifndef DISCRETETRAJECTORY_H
#define DISCRETETRAJECTORY_H

// A discrete trajectory defines an agent that starts at point X
// then jumps to point Y>X at t=0 with velocity v.
// An agent doesn't pass through any other points to get to Y
// so cannot intersect at any point between X and Y.
// BUT: this is unneccesary if we don't require trajectories to
// be continuous...
template<class SPACETIME>
class DiscreteTrajectory {
    typedef SPACETIME              SpaceTime;
    typedef SPACETIME::Scalar      Scalar;

    SpaceTime               start;
    SpaceTime               end;

public:
    template<class MANIFOLD>
    Scalar timeToIntersection(MANIFOLD &manifold) {
        if(manifold(start) >= 0) { return start; }

    }

    void moveOriginTo(Scalar time) {

    }

    const SpaceTime &origin() { return pos; }


};

#endif
