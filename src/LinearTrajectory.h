#ifndef LINEARTRAJECTORY_H
#define LINEARTRAJECTORY_H

#include <limits>
#include <concepts>
#include <atomic>
#include <string>

#include "LabTimeBoundary.h"
#include "numerics.h"
#include "ThreadSafePosition.h"
#include "predeclarations.h"
#include "Concepts.h"
#include "Velocity.h"

// 


/// @brief A LinearTrajectory describes a trajectory on a vector space 
/// that is of the form x(t) = x_0 + vt
/// where t is the scalar of the field.
///
/// @tparam BLOCKINGFIELD defines the field that other agents must block on
/// if they have an incoming connection from an agent on this trajecctory.
/// The blockingfield should be constructible from a position
/// @tparam METRICFIELD defines the velocity that
template<SpaceTime SPACETIME>
class LinearTrajectory  {
public:
    typedef SPACETIME          SpaceTime;
    typedef SpaceTime::Time    Time;


    LinearTrajectory(SpaceTime startPosition, Velocity<SpaceTime> velocity = Velocity<SpaceTime>()) : pos(std::move(startPosition)), vel(std::move(velocity)) { 
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
//     Time timeToIntersection(const SpaceTime &emissionPoint) const {
//         assert(fabs(vel*vel - 1.0) < 1e-8);
//         SpaceTime displacement = emissionPoint - position();
//         displacement += SpaceTime(delta(static_cast<Time>(displacement))); // ensure intersection is strictly in the future of emission point
//         Time mb = vel * displacement; // -b/2
//         Time c = displacement * displacement;
//         Time sq = mb*mb - c;
// //        if constexpr(std::floating_point<Time>) sq += delta(sq); // ensure we don't round into negative
//         if(sq < 0) return std::numeric_limits<Time>::max(); // never intersects

//         Time t = mb + sqrt(sq);

//         return std::move(t); // quadratic formula with a=1 second^2
//     }


    // Intersection point of a field is the first point on the trajectory that the field is non negative
    template<FirstOrderField F>
    Time timeToIntersection(const F &field) const {
        return -field(position())/field.d_dt(vel); // should be valid integer division when time is integer
    }



    template<SecondOrderField F>
    Time timeToIntersection(const F &field) const {
        auto a = field.d2_dt2(vel);
        auto mb = -field.d_dt(position(),vel)/2;
        auto sq = mb*mb - a*field(position());
        if constexpr(std::floating_point<Time>) sq += delta(sq); // ensure we don't round into negative
        if(sq < 0) return std::numeric_limits<Time>::max(); // never intersects

        return (mb + sqrt(sq))/a;
    }


    // The above should optimize to these anyway...
    // Specializations for fields that are based on the metric for which our velocity is defined
    // template<Time Offset> requires StaticFirstOrderField<METRICFIELD>
    // Time timeToIntersection(const HomogeneousField<METRICFIELD, Offset> &field) const {
    //     return -field(position()); 
    // }

    // template<Time Offset> requires StaticSecondOrderField<METRICFIELD>
    // Time timeToIntersection(const HonogeneousField<METRICFIELD, Offset> &field) const {
    //     auto mb = -field.d_dt(position(),vel)/2;
    //     auto sq = mb*mb - field(position());
    //     if constexpr(std::floating_point<Time>) sq += delta(sq); // ensure we don't round into negative
    //     if(sq < 0) return std::numeric_limits<Time>::max(); // never intersects

    //     return mb + sqrt(sq); // quadratic formula with a=1 second^2
    // }


    void advanceBy(Time time) {
        pos += vel*time;
    }


    const Velocity<SpaceTime> & velocity() const { return vel; }
    Velocity<SpaceTime> & velocity() { return vel; }

    const SpaceTime &position() const { return pos; }

    // tjread-safe position write with validity check for agent
    inline void jumpTo(const SpaceTime &newPosition) {
        if(!(position() < newPosition)) {
            throw(std::runtime_error("An agent can only jumpTo a positions that are in its future light-cone."));
        }
        pos = newPosition;
    }

    // The blocking field associated with the current position
//    const ShiftedField<ENV::LambdaField> &asField() { return blockingField; }


protected:
    SpaceTime               pos; // TODO: pass pos to all methods, so that it can be stored elsewhere...?
    Velocity<SpaceTime>     vel;
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
