#ifndef LINEARTRAJECTORY_H
#define LINEARTRAJECTORY_H

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
