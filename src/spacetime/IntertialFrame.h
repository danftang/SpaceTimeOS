#ifndef INERTIALFRAME_H
#define INERTIALFRAME_H

#include "SpaceTime.h"

namespace spacetime {

    template<SpaceTime SPACETIME>
    class IntertialFrame {
    public:
        typedef SPACETIME SpaceTime;
        typedef typename SPACETIME::ScalarType Time;

        SPACETIME   velocity; // 4-velocity

        // default to the laboratory reference frame
        IntertialFrame() { velocity[0] = 1; }
        IntertialFrame(const SPACETIME &velocity) : velocity(velocity) {}
        IntertialFrame(SPACETIME &&velocity) : velocity(std::move(velocity)) {}

        // Calculates the time in this frame from agentPosition to the intersection with a call
        // at callPosition (i.e. |time*velocity - (callPosition-agentPosition)| = 0). 
        Time intersection(const SPACETIME &callPosition, const SPACETIME &agentPosition) const {
            Time time = (callPosition - agentPosition) / velocity; // is this true for all frames by definition of division? [should we divide by the Frame itself?]
            return time;
        }

        // The position of an object after moving from startPoint in this frame for time t.
        SPACETIME positionAfter(const SPACETIME &startPoint, const Time &time) {
            return startPoint + velocity*time; // is this also true for all frames by definition of 4-velocity and multiplication?
            // (or perhaps we should multiply the Frame itself with a scalar)
        }

        // returns the displacement of a clock moving in this frame after that clock measures properTime 
        SPACETIME operator *(Time properTime) const {
            return velocity * time;
        }


        // We can also add velocities together...

    };
};

#endif
