#include "SpaceTime.h"

namespace spacetime {

    template<SpaceTime SPACETIME>
    class IntertialFrame {
    public:
        typedef SPACETIME SpaceTime;
        typedef double Time;

        SPACETIME   velocity;

        // default to the laboratory reference frame
        IntertialFrame() {}

        IntertialFrame(SPACETIME velocity) : velocity(std::move(velocity)) {}

        Time intersection(SpaceTime &callPosition, SpaceTime &agentPosition) {
            Time time = (agentPosition - callPosition) / velocity;
            if(time < 0.0) time = 0.0; // agent must already be in the call's future light cone.
            return time;
        }

        SpaceTime positionAfter(SpaceTime startPoint, Time time) {
            return startPoint + velocity*time;
        }

        // void setPosition(const SpaceTime &pos) {
        //     origin = pos;
        // }

        

    };

    
    
};
