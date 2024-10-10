#ifndef LINEARTRAJECTORY_H
#define LINEARTRAJECTORY_H

#include <limits>
#include <concepts>
#include <atomic>
#include <string>

#include "LabTimeBoundary.h"
#include "numerics.h"
#include "ThreadSafePosition.h"

template<class SPACETIME>
class LinearTrajectory {
public:
    typedef SPACETIME             SpaceTime;
    typedef SPACETIME::Time       Time;
    typedef SPACETIME::Velocity   Velocity;


    LinearTrajectory(SpaceTime startPosition, Velocity velocity = Velocity(1)) : pos(std::move(startPosition)), vel(std::move(velocity)) { 
        // TODO: absorb this into velocity type
        if(fabs(velocity*velocity - 1) > 1e-6)
            throw(std::runtime_error("Can't construct a LinearTrajectory with velocity that isn't of unit length"));
        if(velocity.labTime() < 0)
            throw(std::runtime_error("Can't construct a LinearTrajectory with velocity that isn't future pointing in the laboratory frame"));
        // SpaceTime displacementFromParent = position - ENV::activeAgent->position();
        // if(displacementFromParent*displacementFromParent < 0)
        //     throw(std::runtime_error("Can't construct an agent outside the future light-cone of the point of construction"));
    }


    // Calculates time to intersection with the inner product field (x.x)
    // timeToIntersection is defined as the largest t such that F = (tV - S).(tV - S) = 0.
    // where S = emissionPoint - agentPosition
    //
    // i.e. if V is a velocity and S is a displacement, timeToIntersection is the
    // time it would take in a reference frame moving with V to reach zero
    // distance to S, starting at the origin.
    // If V.S = S.V and (A + B).C =  A.C + B.C then this equivalent to the largest solution of
    // V.Vt^2 - 2V.St + S.S > 0
    // We assume V.V = 1 so 
    // t^2 - 2V.St + S.S > 0
    // TODO: we can generalise this even further by solving |tV - S| = c. This corresponds to a channel that has a constant distance between emission and absorbtion
    Time timeToIntersection(const SpaceTime &emissionPoint) const {
        assert(fabs(vel*vel - 1.0) < 1e-8);
        SpaceTime displacement = emissionPoint - this->position();
        displacement += SpaceTime(delta(displacement.labTime())); // ensure intersection is strictly in the future of emission point
        Time mb = vel * displacement; // -b/2
        Time c = displacement * displacement;
        Time sq = mb*mb - c;
//        if constexpr(std::floating_point<Time>) sq += delta(sq); // ensure we don't round into negative
        if(sq < 0) return std::numeric_limits<Time>::max(); // never intersects

        Time t = mb + sqrt(sq);

        // if(pos.labTime() + t*vel.labTime() <= emissionPoint.labTime()) { // ensure strictly increasing intersection by requiring lab-time strictly increases
        //     t = (emissionPoint.labTime() - pos.labTime()) / vel.labTime();
        //     t += delta(t);
        // }

        return std::move(t); // quadratic formula with a=1 second^2
    }


    // Intersection point of a field is the first point on the trajectory that the field is non negative
    template<FirstOrderField F>
    Time timeToIntersection(const F &field) const {
        if constexpr(std::floating_point<Time>) {
            return -field(this->position())/field.d_dt(vel);
        }
        return ceil(-field(this->position())/field.d_dt(vel));
    }


    template<SecondOrderField F>
    Time timeToIntersection(const F &field) const {
        auto a = field.d2_dt2(vel);
        auto mb = -field.d_dt(this->position(),vel)/2;
        auto sq = mb*mb - a*field(this->position());
        if constexpr(std::floating_point<Time>) sq += delta(sq); // ensure we don't round into negative
        if(sq < 0) return std::numeric_limits<Time>::max(); // never intersects

        Time t = (mb + sqrt(sq))/a;

        return std::move(t); // quadratic formula with a=1 second^2
    }
    

    void advanceBy(Time time) {
        pos += vel*time;
    }


    const Velocity & velocity() const { return vel; }
    Velocity & velocity() { return vel; }

    const SPACETIME &position() const { return pos; }

    // tjread-safe position write with validity check for agent
    inline void jumpTo(const SPACETIME &position) {
        if(!(pos < position)) {
            throw(std::runtime_error("An agent can only jumpTo a positions that are in its future light-cone."));
        }
        pos = position;
    }


protected:
    SpaceTime   pos;
    Velocity    vel;
};


// Specialization for 1-dimensional case, where SpaceTime is Time
// template<class SPACETIME> requires (SPACETIME::DIMENSIONS == 1)
// class LinearTrajectory<SPACETIME>  : public ThreadSafePosition<SPACETIME> {
//     typedef SPACETIME             SpaceTime;
//     typedef SPACETIME::Time       Time;

//     LinearTrajectory(SpaceTime startPosition) : ThreadSafePosition<SPACETIME>(std::move(startPosition)) { }

//     // Calculates time to intersection with the inner product field (x.x)
//     // timeToIntersection is defined as the largest t such that F = (tV - S).(tV - S) = 0.
//     // where S = emissionPoint - agentPosition
//     //
//     // i.e. if V is a velocity and S is a displacement, timeToIntersection is the
//     // time it would take in a reference frame moving with V to reach zero
//     // distance to S, starting at the origin.
//     // If V.S = S.V and (A + B).C =  A.C + B.C then this equivalent to the largest solution of
//     // V.Vt^2 - 2V.St + S.S > 0
//     // We assume V.V = 1 so 
//     // t^2 - 2V.St + S.S > 0
//     // TODO: we can generalise this even further by solving |tV - S| = c. This corresponds to a channel that has a constant distance between emission and absorbtion
//     Time timeToIntersection(const SpaceTime &emissionPoint) const {
//         SpaceTime displacement = emissionPoint - this->position();
//         displacement += SpaceTime(delta(displacement.labTime())); // ensure intersection is strictly in the future of emission point
//         return displacement; // quadratic formula with a=1 second^2
//     }

//     // Intersection point of a field is the first point on the trajectory that the field is non negative
//     template<FirstOrderField F>
//     Time timeToIntersection(const F &field) const {
//         if constexpr(std::floating_point<Time>) {
//             return -field(this->position())/field.d_dt(1);
//         }
//         return ceil(-field(this->position())/field.d_dt(1));
//     }
    

//     void advanceBy(Time time) {
//         this->setPosition(this->position() + time); // channel locks ensure this doesn't commute with previous lambda emissions
//     }
// };

#endif
