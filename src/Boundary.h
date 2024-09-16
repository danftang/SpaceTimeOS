#ifndef BOUNDARY_H
#define BOUNDARY_H

#include "CallbackQueue.h"

template<class FORWARD>
class BoundaryCoordinate : public FORWARD { 
public:
    explicit BoundaryCoordinate(FORWARD &&position) : FORWARD(position) {}
};

template<class T> BoundaryCoordinate(std::initializer_list<T>) -> BoundaryCoordinate<std::initializer_list<T>>;

template<class SPACETIME>
class Boundary {
public:
    typedef SPACETIME SpaceTime;
    typedef SPACETIME::Scalar Scalar;

    template<std::invocable LAMBDA>
    inline void callbackOnMove(LAMBDA &&lambda) {
        callbacks.push(std::forward<LAMBDA>(lambda));
    }

protected:
    CallbackQueue   callbacks;
};

#endif
