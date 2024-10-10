#ifndef LABORATORY_H
#define LABORATORY_H

#include "ThreadPool.h"
#include "Concepts.h"
#include "Agent.h"
#include "LabTimeBoundary.h"

template<Trajectory TRAJECTORY, Executor EXECUTOR = ThreadPool<0>, class BOUNDARY = LabTimeBoundary<typename TRAJECTORY::SpaceTime>>
class ForwardSimulation {
public:
    typedef TRAJECTORY::SpaceTime   SpaceTime;
    typedef SpaceTime::Time         Time;
    typedef ForwardSimulation<TRAJECTORY,EXECUTOR,BOUNDARY>  Simulation;
    typedef TRAJECTORY              Trajectory;

    static inline BOUNDARY                      boundary;

    template<class T>
    static inline void submit(T &&runnable) {
        executor.submit(std::forward<T>(runnable));
    }


    static void start(Time endTime) {
        boundary.setTime(endTime);
        Agent<Simulation>::start();
        executor.join();
    }
protected:
    static inline EXECUTOR                      executor;
};

#endif
