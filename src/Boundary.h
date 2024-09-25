#ifndef BOUNDARY_H
#define BOUNDARY_H

#include "CallbackQueue.h"

// template<class FORWARD>
// class BoundaryCoordinate : public FORWARD { 
// public:
//     explicit BoundaryCoordinate(FORWARD &&position) : FORWARD(position) {}
// };

// template<class T> BoundaryCoordinate(std::initializer_list<T>) -> BoundaryCoordinate<std::initializer_list<T>>;

// A boundary is like a global channel with a single lambda.
// It is attached to all agents and will execute the lambda when any agent intersects with it. Unlike a lambda
// it is not absorbed by the agent on execution, so can be executed on multiple agents. 
// template<class SPACETIME>
// class Boundary {
// public:
//     typedef SPACETIME SpaceTime;
//     typedef SPACETIME::Scalar Scalar;

//     template<std::invocable LAMBDA>
//     inline void callbackOnMove(LAMBDA &&lambda) {
//         callbacks.push(std::forward<LAMBDA>(lambda));
//     }

// protected:
//     CallbackQueue   callbacks;
// };

#endif
