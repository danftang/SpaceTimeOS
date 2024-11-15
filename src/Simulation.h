#ifndef SIMULATION_H
#define SIMULATION_H

#include "predeclarations.h"

// Given an environment, a Simulation<ENV> is a convenient place to put anything that all agents
// need access to...i.e. any data that pertains to the simulation as a whole.
template<Environment ENV>
class Simulation {
public:
    typedef ENV::SpaceTime  SpaceTime;

    static inline typename ENV::LambdaField     lambdaField;// field associated with lambda functions
    static inline typename ENV::Boundary        boundary;   // field that defines the boundary
    static inline typename ENV::Executor        executor;   // task executor. Non-static so we can clean up in destructor
    static inline SourceAgent<ENV>              mainThread =  SourceAgent<ENV>(SpaceTime(-sqrt(std::numeric_limits<typename SpaceTime::Time>::max()))); // Agent on which the main thread notionally runs
    static inline thread_local SourceAgent<ENV> *currentThreadAgent = &mainThread; // each thread has an active agent on which it is currently running
    

    typedef decltype(TranslatedField(lambdaField, std::declval<SpaceTime>())) TranslatedLambdaField;
    typedef TranslatedLambdaField TranslatedBlockingField;

    // Call this to start the simulation after creating initial agents.
    static void start() {
        mainThread.advanceBy(mainThread.timeToIntersection(Simulation<ENV>::boundary));
        executor.join();
    }

};

#endif
