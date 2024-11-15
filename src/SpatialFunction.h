#ifndef SPATIALFUNCTION_H
#define SPATIALFUNCTION_H

#include <functional>
#include "Concepts.h"
#include "predeclarations.h"

/// @brief A SpatialFunction is a lambda function that fills a subset E of spacetime,
/// known as its "execution zone". A target agent can only execute a 
/// SpatialFunction if it is in its execution zone.
///
/// @tparam ENV The environment whose agents this lambda takes as argument
/// @tparam FIELD The Field of this lambda, should be constructibe from the position of the emitting agent,
template<class ENV, DifferentiableField FIELD>
class SpatialFunction : public std::function<void(Agent<ENV> &)> {
public:

    template<class F, class LAMBDA>
    SpatialFunction(F &&field, LAMBDA &&lambda) : 
        std::function<void(Agent<ENV> &)>(std::forward<LAMBDA>(lambda)),
        field(std::forward<F>(field)) { }

    const FIELD &asField() { return field; }


    const FIELD                  field;   // field defining zone of execution.
};

#endif
