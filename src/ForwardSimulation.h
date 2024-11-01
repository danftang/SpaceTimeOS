#ifndef LABORATORY_H
#define LABORATORY_H

#include "ThreadPool.h"
#include "Concepts.h"
#include "Agent.h"
#include "LabTimeBoundary.h"

template<
    Trajectory TRAJECTORY, 
    StaticDifferentiableField LAMBDAFIELD,
    StaticDifferentiableField METRIC,
    StaticDifferentiableField BLOCKINGFIELD = LAMBDAFIELD, // assume lambdafield is its own blocking field w.r.t the metric
    Executor EXECUTOR = ThreadPool<0>, 
    class BOUNDARY = LabTimeBoundary<typename TRAJECTORY::SpaceTime>
    >
class ForwardSimulation {
public:
    typedef TRAJECTORY::SpaceTime   SpaceTime;
    typedef SpaceTime::Time         Time;
    typedef ForwardSimulation<TRAJECTORY,EXECUTOR,BOUNDARY>  Simulation;
    typedef TRAJECTORY              Trajectory;
    typedef EXECUTOR                Executor;

    typedef LAMBDAFIELD             LambdaField;
    typedef METRIC                  Metric;
    typedef BLOCKINGFIELD           BlockingField;
    // LambdaFieldType      // Defines a region where F(X)>0.
    //                          This defines a group order (not vector since lambda emission is discrete) via it's closure, C, w.r.t 
    //                          convolution (vector set addition). This should conform to:
    //                         if X is in C then -X isn't in C. But also, if X is in C then -X isn't in the positive velecity field, V.
    //                         The group closure of C and V should conform to X is in it implies -X isn't. 
    //                         (i.e. Lambda can't intersect with an agent's own past.) That is:
    //                           - it can't form loops via pure lambda emission
    //                           - it can't form loops via any combination of lambda emission and movement.
    //                         
    //                          
    // VelocityFieldType    // Defines (square of?) local time from origin to a point.
    //                         The positive portion should be a positive cone of the vector space (should be a vector subspace)
    //                      // [should we assume this is an inner product? Diagonal?     
    // BlockingFieldType    // differentiable for floating point time. Should be the convolution of LambdaField and VelocityField

    static inline BOUNDARY                      boundary;
    static inline EXECUTOR                      executor;

    static void start(Time endTime) {
        boundary.setTime(endTime);
        Agent<Simulation>::start();
        executor.join();
    }
};

#endif
