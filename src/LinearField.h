#ifndef LINEARFIELD_H
#define LINEARFIELD_H

#include "Concepts.h"

// Linear field over an inner product space. Gradient should be a unit vector (i.e. G.G=1)

/// @brief Field of the form F(X) = Gradient.X - Offset
template<class SpaceTime, SpaceTime::Time Offset=0>
class LinearField {
public:

    const SpaceTime grad;

    LinearField(const SpaceTime &gradient) : grad(gradient) {}

    inline auto operator ()(const SpaceTime &pos) const {
        return pos * grad - Offset;
    }

    inline auto d_dt(const SpaceTime &vel) const {
        return vel * grad;
    }

    // template<SpaceTime::Time OtherOffset>
    // inline static constexpr Time d_dt(const Velocity<LinearField<SpaceTime, OtherOffset>> &) { return 1; }
};

#endif
