#ifndef LABTIMEBOUNDARY_H
#define LABTIMEBOUNDARY_H

#include "Boundary.h"

template<class SPACETIME>
class LabTimeBoundary : public Boundary<SPACETIME> {
public:
    typedef SPACETIME SpaceTime;
    typedef SPACETIME::Scalar Scalar;

protected:
    Scalar          maxTime = 0;
    bool            deleteOnBoundary = true;

public:

    const SpaceTime labVelocity = SpaceTime(1);

    Scalar timeToIntersection(const SpaceTime &agentPosition, const SpaceTime &agentVelocity) {
        return (maxTime - agentPosition[0])/agentVelocity[0];
    }

    void advanceMaxTime(Scalar time) {
        assert(time > 0);
        maxTime += time;
        this->callbacks.execAll();
    }

    // Agent will call this when it reaches the boundary
    template<class T>
    void boundaryEvent(T &agent) {
        if(deleteOnBoundary) {
            std::cout << "Agent on boundary, deleting " << &agent << std::endl;
            delete(&agent);
        } else {
            // otherwise, put it on my callback queue
            agent.sendCallbackTo(*this);
        }
    }

    const Boundary<SPACETIME>::Position pointOnBoundary(const SpaceTime &pos) {
        return this->tagAsBoundaryPosition(pos + labVelocity * timeToIntersection(pos,labVelocity));
    }
};

#endif
