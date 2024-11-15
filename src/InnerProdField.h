#ifndef INNERPRODFIELD_H
#define INNERPRODFIELD_H

#include "Concepts.h"
#include "TranslatedField.h"

/// @brief We define an InnerProductField to be a family of fields over an inner-product space of the form:
/// F(X) = (X-Origin).(X-Origin) - Offset;
/// for some constant vector X0 and constant c.
template<class SPACETIME, SPACETIME::Time Offset = 0>
class InnerProdField {
public:
    typedef SPACETIME SpaceTime;

    static auto value(const SpaceTime &x) {
        return x * x - Offset;
    }

    // dF(X)/dt = dF(X)/dX dX/dt = 2(X-Origin).velocity
    // where dX/dt=velocity
    static auto d_dt(const SpaceTime &x, const SpaceTime &velocity) {
        return 2*(x*velocity);
    }

    // Assuming d^2X/dt^2 = 0
    // d^2F(X)/dt^2 = (dX/dt)^T d^2F(X)/dX^2 dX/dt = v.v
    // where dX/dt=velocity
    static auto d2_dt2(const SpaceTime &velocity) {
        return velocity * velocity;
    }

    // if velocity is a velocity of this space then v.v=1 by definition
    static SpaceTime::Time d2_dt2(const Velocity<SpaceTime> &velocity) {
        return 1;
    }

    inline auto operator ()(const SpaceTime &x) const { return value(x); }
};




#endif
