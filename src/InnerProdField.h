#ifndef INNERPRODFIELD_H
#define INNERPRODFIELD_H

#include "Concepts.h"

// A spacetime with an inner product defines a second order scalar field over the spacetime.
// given by F = x.x. So, if x = x_0 + vt then F(t) = (x0+vt).(x0+vt)
// Since, by definition, (a+b).c = a.c + b.c and a.b = b.a then
// F(t) = v.vt^2 + 2x0.vt + x0.x0
// So, at t=0 we have
// dF/dt = 2x0.v and d2F/dt2 = v.v
// If we also allow the origin of the field to be at some parametric position o, then we
// have x0 <- x-o 
template<InnerProductSpace SPACETIME>
class InnerProdField {
public:
    SPACETIME origin;

    auto operator()(const SPACETIME &x) {
        auto displacement = x-origin;
        return displacement * displacement;
    }

    auto d_dt(const SPACETIME &x, const SPACETIME &velocity) {
        return 2*((x-origin)*velocity);
    }

    auto d2_dt2(const SPACETIME &velocity) {
        return velocity * velocity;
    }
};

#endif
