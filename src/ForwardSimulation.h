#ifndef LABORATORY_H
#define LABORATORY_H

#include "ThreadPool.h"
#include "Concepts.h"
#include "Agent.h"
#include "LabTimeBoundary.h"

// A Laboratory is a spatial object in the default reference frame, at the default position
// However, a laboratory provides initiating methods that do not respect limitations on the
// maximum velocity of the flow of information.
//
template<SpaceTime SPACETIME, Executor EXECUTOR = ThreadPool<0>, class BOUNDARY = LabTimeBoundary<SPACETIME> >
class ForwardSimulation {
public:
    typedef SPACETIME                       SpaceTime;
    typedef SPACETIME::Scalar               Scalar;
    typedef ForwardSimulation<SPACETIME,EXECUTOR>  Simulation;
protected:

    static inline EXECUTOR                  executor;

public:
    static inline AgentBase<SpaceTime>       initialisingAgent = AgentBase<SpaceTime>(SPACETIME::BOTTOM, SPACETIME(1));
    static inline thread_local BOUNDARY      boundary;
    static inline thread_local AgentBase<SpaceTime> *activeAgent = &initialisingAgent; // each thread has an active agent on which it is currently running

    template<std::invocable T>
    static void submit(T &&runnable) {
        executor.submit(std::forward<T>(runnable));
    }


    static void start(SpaceTime::Scalar endTime) {
        boundary.advanceMaxTime(endTime);
        initialisingAgent.execCallbacks();
        executor.join();
    }

    // static Scalar timeToIntersection(const SpaceTime &agentPosition, const SpaceTime &agentVelocity) {
    //     return (maxTime - agentPosition[0])/agentVelocity[0];
    // }

    // static const BoundaryPos pointOnBoundary(const SpaceTime &pos) {
    //     return { pos + velocity * timeToIntersection(pos, velocity) };
    // }



};

#endif
