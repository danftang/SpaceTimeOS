#ifndef LINEARTRAJECTORY_H
#define LINEARTRAJECTORY_H

#include <limits>
#include "LabTimeBoundary.h"
//
template<class SPACETIME>
class LinearTrajectory {
public:
    typedef SPACETIME              SpaceTime;
    typedef SPACETIME::Scalar      Time;
    typedef SPACETIME::Velocity    Velocity;

    LinearTrajectory(SpaceTime startPosition, Velocity velocity = Velocity(1)) : pos(std::move(startPosition)), vel(std::move(velocity)) { 
        // TODO: absorb this into velocity type
        if(fabs(velocity*velocity - 1) > 1e-6)
            throw(std::runtime_error("Can't construct a LinearTrajectory with velocity that isn't of unit length"));
        if(velocity[0] < 0)
            throw(std::runtime_error("Can't construct a LinearTrajectory with velocity that isn't future pointing in the laboratory frame"));
        // SpaceTime displacementFromParent = position - ENV::activeAgent->position();
        // if(displacementFromParent*displacementFromParent < 0)
        //     throw(std::runtime_error("Can't construct an agent outside the future light-cone of the point of construction"));
    }

    // timeToIntersection is defined as t such that (tV - S).(tV - S) > 0 and (tV-S).r > 0.
    // where S = emissionPoint - agentPosition and r is the reference direction-of-time.
    //
    // i.e. if V is a velocity and S is a displacement, timeToIntersection is the
    // time it would take in a reference frame moving with V to reach zero
    // distance to S, starting at the origin.
    // If V.S = S.V and (A + B).C =  A.C + B.C then this equivalent to the largest solution of
    // V.Vt^2 - 2V.St + S.S > 0
    // We assume V.V = 1 so 
    // t^2 - 2V.St + S+S > 0
    // TODO: we can generalise this even further by solving |tV - S| = c. This corresponds to a channel that has a constant distance between emission and absorbtion
    Time timeToIntersection(const SpaceTime &emissionPoint) {
        auto displacement = emissionPoint - pos;
        auto mb = vel * displacement; // -b/2
        auto c = displacement * displacement;
        auto mbmb = mb*mb;
        if(c > mbmb) { // no solution so the only constraint is (tV-S).r > 0, so t = S.r/V.r
            return (displacement * SpaceTime(1)) / (vel * SpaceTime(1)) + std::numeric_limits<Time>::epsilon();
        }

        if constexpr(std::numeric_limits<Time>::is_integer) {
            return mb + Time(floor(sqrt(static_cast<double>(mbmb - c)))) + 1;
        }
        return mb + sqrt(mbmb - c) + std::numeric_limits<Time>::epsilon()*SpaceTime::Dimensions; // quadratic formula with a=1 second^2 + epsilon to ensure strictly greater
    }

    template<class LAMBDA>
    Time timeToIntersection(const LabTimeBoundary<SpaceTime,LAMBDA> &boundary) {
        return (boundary.getTime() - pos[0])/vel[0];
    }


    void advanceBy(Time time) {
        pos += vel*time;
    }

    void jumpTo(const SpaceTime &position) {
        assert(pos < position); // must be in agent's future
        pos = position;
    }

    Velocity &velocity() { return vel; }

    const SpaceTime &origin() const { return pos; }

protected:
    SpaceTime    pos;
    Velocity     vel;

    
};

#endif
