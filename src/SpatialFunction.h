#ifndef SPATIALFUNCTION_H
#define SPATIALFUNCTION_H

#include <functional>
#include "Concepts.h"
#include "predeclarations.h"

// A SpatialFunction is a function that fills a subset E of spacetime,
// known as its "execution zone". A target agent can only execute a 
// SpatialFunction if it is in its execution zone.
//
// Suppose an agent at X emits a SpatialFuntion with execution region E(X).
// E(X) should satisfy:
//      1) For all points Y in E(X): Y > X 
//      2) If Y is in E(X) and Z > Y then Z is also in E(X)
//      3) If Y is in E(X) then for all C: Y+C is in E(X+C)
// This has the consequence that an agent at X cannot interact
// outside of E through any combination of moving, emitting and
// communicating with other intermediary agents.
// The third requirement means we can define E(X) for any X by defining
// the set for the origin E(O) and E(X) = {Y : Y-X is in E(O)}
//
// We can define an execution zone in terms of a differentiable
// "execution function" f
// E(X) = {X : f(X) > 0}
// which can be heflpul as it gives an agent an idea of how far away from
// the execution zone it is.
//
// The execution zone can be thought of as more fundamental than the ordering
// on SpaceTime, and we can define a SpaceTime ordering (equivalently the positive cone)
// in terms of E(O), perhaps as the maximal cone that both contains E(O) (condition 1) and
// is contained by E(O) (condition 2). Programatically, we only ever need to know E(O)
// (if we only allow agents to create new agents in their execution zone [what about
//  deleting itself?] and allow agents to only move towards points in their execution zone.
//  Although this alone doesn't define local time, in-case this is important,
//  unless we define local time in terms of E(O) and so the positive light cone is the
//  conic closure of E(O) [what are the necessary/sufficient conditions on E(O) that imply that
//  the convolution of E(O) with its own conic closure is itself?]).
//
// 



template<class ENV>
class SpatialFunction : public std::function<void(Agent<ENV> &)> {
public:
    typedef HomogeneousField<typename ENV::LambdaField> Field;

    template<class LAMBDA>
    SpatialFunction(const ENV::SpaceTime &position, LAMBDA &&lambda) : 
        std::function<void(Agent<ENV> &)>(std::forward<LAMBDA>(lambda)),
        field(position) { }

    const Field &asField() { return field; }


    Field                  field;   // field defining zone of execution.
};

#endif
