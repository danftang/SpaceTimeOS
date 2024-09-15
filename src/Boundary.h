#ifndef BOUNDARY_H
#define BOUNDARY_H

#include "CallbackQueue.h"

template<class SPACETIME>
class Boundary {
public:
    typedef SPACETIME SpaceTime;
    typedef SPACETIME::Scalar Scalar;

    class Position : public SPACETIME {
    protected:
        Position(const SPACETIME &pos) : SPACETIME(pos) { }
        friend Boundary<SPACETIME>;
    };


    template<std::invocable LAMBDA>
    inline void callbackOnMove(LAMBDA &&lambda) {
        callbacks.push(std::forward<LAMBDA>(lambda));
    }

protected:
    CallbackQueue   callbacks;

    const Position tagAsBoundaryPosition(const SPACETIME &position) {
        return { position };
    } 



};

#endif
