#ifndef BOUNDARYAGENT_H
#define BOUNDARYAGENT_H

#include "Agent.h"

template<Simulation ENV>
class BoundaryAgent : public Agent<ENV> {
public:
    BoundaryAgent() : Agent<ENV>(ENV::Trajectory(SpaceTime::BOTTOM)) {}

    ~BoundaryAgent() {
        this->deleteCallbackAgents();            // delete agents waiting on the boundary
    }
}

#endif
