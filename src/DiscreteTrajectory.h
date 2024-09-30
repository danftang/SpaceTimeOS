#ifndef DISCRETETRAJECTORY_H
#define DISCRETETRAJECTORY_H

// A discrete trajectory defines an agent that starts at point X
// disappears and reappears some local-time later at point Y
// then stays at Y for ever. 
// An agent doesn't pass through any other points to get to Y
// so cannot intersect at any other point than X or Y.
// If time is continuous, then we reduce to the continuous case once we reach Y
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
