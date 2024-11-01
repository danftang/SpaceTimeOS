#ifndef LINEARFIELD_H
#define LINEARFIELD_H


// Linear field with scale such that unit labtime equals 1
// i.e the first element of Gradient should be 1
template<auto Gradient>
class LinearField {
public:
    typedef decltype(Gradient) SpaceTime;

    inline static auto value(const SpaceTime &pos) {
        return dotProduct(pos,Gradient);
    }

    inline static auto d_dt(const SpaceTime &vel) {
        return dotProduct(vel,Gradient);
    }


    inline static constexpr Time d_dt(const Velocity<LinearField<Gradient>> &) { return 1; }
};

#endif
