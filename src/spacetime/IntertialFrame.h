#include "SpaceTime.h"

namespace spacetime {

    template<SpaceTime SPACETIME>
    class IntertialFrame {
    public:
        typedef SPACETIME SpaceTime;
        typedef double Time;

        SPACETIME   velocity; // unit 4-velocity

        // default to the laboratory reference frame
        IntertialFrame() {
            velocity.v[0] = 1.0; // TODO: make this more portable
        }

        IntertialFrame(SPACETIME velocity) : velocity(std::move(velocity)) {}

        // Calculates the time in this frame from agentPosition to the intersection with a call
        // at callPosition (i.e. |time*velocity - (callPosition-agentPosition)| = 0). 
        Time intersection(const SpaceTime &callPosition, const SpaceTime &agentPosition) const {
            Time time = (callPosition - agentPosition) / velocity; // is this true for all frames by definition of division?
            return time;
        }

        // The position of an object after moving from startPoint in this frame for time t.
        SpaceTime positionAfter(const SpaceTime &startPoint, const Time &time) {
            return startPoint + velocity*time;
        }

    };

    
    
};
