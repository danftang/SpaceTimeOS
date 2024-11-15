#ifndef LABORATORY_H
#define LABORATORY_H

#include "ThreadPool.h"
#include "Concepts.h"
#include "Agent.h"
#include "LabTimeBoundary.h"

template<
    SpaceTime SPACETIME, 
    DifferentiableField LAMBDAFIELD,
    FirstOrderField BOUNDARY,
    Executor EXECUTOR = ThreadPool<0>
    >
class ForwardSimulation {
public:
//    typedef ForwardSimulation<SPACETIME,LAMBDAFIELD,BOUNDARY,EXECUTOR>  Simulation;
    typedef SPACETIME               SpaceTime;
//    typedef SpaceTime::Time         Time;
    typedef EXECUTOR                Executor;
    typedef LAMBDAFIELD             LambdaField;
    typedef BOUNDARY                Boundary;

    // static inline LambdaField       lambdaField;// field associated with lambda functions
    // static inline Boundary          boundary;   // field that defines the boundary
    // static inline EXECUTOR          executor;   // task executor. Non-static so we can clean up in destructor
    // static inline CallbackField<Simulation> 

    // typedef decltype(TranslatedField(lambdaField, std::declval<SpaceTime>())) TranslatedlambdaField;
    // typedef TranslatedLambdaField TranslatedBlockingField;


//     static void start() {
// //        boundary.setTime(endTime);
//         Agent<Simulation>::start();
//         executor.join();
//     }
};

#endif
