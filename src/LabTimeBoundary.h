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

    // SpaceTime projectToBoundary(const SpaceTime &pos) {
    //     return pos + labVelocity * timeToIntersection(pos,labVelocity);
    // }


    template<std::convertible_to<Scalar> T> 
    SpaceTime onBoundary(std::initializer_list<T> spacePosition) {
        assert(spacePosition.size() == SpaceTime::size()-1);
        SpaceTime spaceTimePos;
        spaceTimePos[0] = maxTime;
        int i = 1;
        for(const T &coord : spacePosition) {
            spaceTimePos[i] = coord;
        }
        return spaceTimePos + labVelocity * timeToIntersection(spaceTimePos,labVelocity);
    }
};

#endif
