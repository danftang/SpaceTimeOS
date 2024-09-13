#ifndef LABORATORY_H
#define LABORATORY_H


// #include "SpaceTimePtr.h"
#include "ThreadPool.h"
#include "Concepts.h"
#include "Agent.h"


// A Laboratory is a spatial object in the default reference frame, at the default position
// However, a laboratory provides initiating methods that do not respect limitations on the
// maximum velocity of the flow of information.
template<SpaceTime SPACETIME, Executor EXECUTOR = ThreadPool<0>>
class ForwardSimulation {
public:
    typedef SPACETIME                       SpaceTime;
    typedef SPACETIME::Scalar               Scalar;
    typedef ForwardSimulation<SPACETIME,EXECUTOR>  Simulation;

    class Lab : public Agent<Lab,Simulation> {
    public:
        Lab() : Agent<Lab,Simulation>(SpaceTime(),SpaceTime(1)) {}
    };

    static inline EXECUTOR                  executor;
//    static inline std::function<bool(const SpaceTime &)>   isInBounds = [](const SpaceTime &) { return false; };
    static inline Scalar maxTime = 0;
    // The laboratory provides a reference origin, and its callback queue
    // is used by Agent to store callbacks of new agents.
    static inline thread_local Lab      laboratory;

    static inline bool deleteOnBoundary = true;

public:

    template<class T> requires std::derived_from< T, Agent<T,Simulation> > // TODO: make Agent a concept
    static void submit(T &obj) {
        executor.submit([&obj]() {
            obj.step();
        });
    }

    static void start(SpaceTime::Scalar endTime) {
        maxTime = endTime;
        laboratory.execCallbacks();
        executor.join();
    }


    static void stop() {
        executor.join();
    }

    // static void setMaxTime(SpaceTime::Scalar laboratoryEndTime) {
    //     isInBounds = [laboratoryEndTime](const SpaceTime &position) {
    //         return position[0] < laboratoryEndTime;
    //     };
    // }

    template<class T> requires std::derived_from< T, Agent<T,Simulation> >
    static void agentFinished(T &agent) {
        if(deleteOnBoundary) {
            std::cout << "deleting " << &agent << std::endl;
            delete(&agent);
        } else {
            agent.sendCallbackTo(laboratory);
        }
    }

    static Scalar timeToIntersection(const SpaceTime &agentPosition, const SpaceTime &agentVelocity) {
        return (maxTime - agentPosition[0])/agentVelocity[0];
    }


};

#endif
