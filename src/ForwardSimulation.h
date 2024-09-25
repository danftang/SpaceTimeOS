#ifndef LABORATORY_H
#define LABORATORY_H

#include "ThreadPool.h"
#include "Concepts.h"
#include "Agent.h"
#include "LabTimeBoundary.h"

auto deleteAgent = [](auto &agent) { 
        std::cout << "Agent on boundary, deleting " << &agent << std::endl;
        delete(&agent);
     };

template<SpaceTime SPACETIME, Executor EXECUTOR = ThreadPool<0>, class BOUNDARY = LabTimeBoundary<SPACETIME,decltype(deleteAgent)>>
class ForwardSimulation {
public:
    typedef SPACETIME                       SpaceTime;
    typedef SPACETIME::Scalar               Scalar;
    typedef ForwardSimulation<SPACETIME,EXECUTOR>  Simulation;
protected:

    static inline EXECUTOR                      executor;

public:
    static inline AgentBase<SpaceTime>          initialisingAgent = AgentBase<SpaceTime>(SPACETIME::BOTTOM, SPACETIME(1));
    static inline BOUNDARY                      boundary;
    static inline thread_local AgentBase<SpaceTime> *activeAgent = &initialisingAgent; // each thread has an active agent on which it is currently running

    template<std::invocable T>
    static void submit(T &&runnable) {
        executor.submit(std::forward<T>(runnable));
    }


    static void start(SpaceTime::Scalar endTime) {
        boundary.setTime(endTime);
        initialisingAgent.execCallbacks();
        executor.join();
    }
};

#endif
