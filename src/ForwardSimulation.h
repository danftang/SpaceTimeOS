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

template<Trajectory TRAJECTORY, Executor EXECUTOR = ThreadPool<0>, class BOUNDARY = LabTimeBoundary<typename TRAJECTORY::SpaceTime,decltype(deleteAgent)>>
class ForwardSimulation {
public:
    typedef TRAJECTORY::SpaceTime   SpaceTime;
    typedef SpaceTime::Scalar       Scalar;
    typedef ForwardSimulation<TRAJECTORY,EXECUTOR,BOUNDARY>  Simulation;
    typedef TRAJECTORY              Trajectory;

    static inline BOUNDARY                      boundary;

    template<class T>
    static inline void submit(T &&runnable) {
        executor.submit(std::forward<T>(runnable));
    }


    static void start(SpaceTime::Scalar endTime) {
        boundary.setTime(endTime);
        Agent<Simulation>::initialisingAgent.execCallbacks();
        executor.join();
    }
protected:
    static inline EXECUTOR                      executor;
};

#endif
