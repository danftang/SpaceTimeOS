#ifndef LABORATORY_H
#define LABORATORY_H


// #include "SpaceTimePtr.h"
#include "ThreadPool.h"
#include "Concepts.h"
#include "Agent.h"


// // Dummy object for a Laboratory
// class Lab {};

// A Laboratory is a spatial object in the default reference frame, at the default position
// However, a laboratory provides initiating methods that do not respect limitations on the
// maximum velocity of the flow of information.
template<SpaceTime SPACETIME, Executor EXECUTOR = ThreadPool<0>>
class ForwardSimulation {
public:
    typedef SPACETIME                       SpaceTime;
    typedef ForwardSimulation<SPACETIME,EXECUTOR>  Simulation;

    static inline EXECUTOR                  executor;
    static inline std::function<bool(const SpaceTime &)>   isInBounds = []() { return false; };

    // The laboratory provides a reference origin, and its callback queue
    // is used by Agent to store callbacks of new agents.
    static inline AgentBase<SpaceTime>      laboratory = AgentBase<SpaceTime>(SpaceTime());

    static inline bool deleteOnBoundary = true;

public:

    template<class T>
    static void submit(T &obj) { 
        if(isInBounds(obj.position())) {
            executor.submit([&obj]() {
                obj.step();
            });
        }
    }

    static void start(SpaceTime::ScalarType endTime) {
        isInBounds = [endTime](const SpaceTime &position) {
            return position[0] < endTime;
        };
        laboratory.execCallbacks();
        executor.join();
    }


    static void stop() {
        executor.join();
    }

    static void setMaxTime(SpaceTime::ScalarType laboratoryEndTime) {
        isInBounds = [laboratoryEndTime](const SpaceTime &position) {
            return position[0] < laboratoryEndTime;
        };
    }

    template<class T>
    static void agentFinished(T *agent) {
        if(deleteOnBoundary) {
            delete(agent);
        } else {
            agent->setCallback(laboratory);
        }
    }    

};

#endif
