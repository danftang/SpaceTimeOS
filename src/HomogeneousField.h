#ifndef HOMOGENEOUSFIELD_H
#define HOMOGENEOUSFIELD_H

#include "Concepts.h"
#include "Velocity.h"

template<StaticDifferentiableField STATICFIELD, STATICFIELD::SpaceTime::Time Offset = 0> class HomogeneousField;


template<StaticFirstOrderField STATICFIELD, STATICFIELD::SpaceTime::Time Offset>
class HomogeneousField<STATICFIELD,Offset> {
public:
    typedef STATICFIELD::SpaceTime SpaceTime;

    SpaceTime origin;
    HomogeneousField(SpaceTime x) : origin(std::move(x)) {}

    auto operator ()(const SpaceTime &pos) const { return STATICFIELD::value(pos-origin) + Offset; }
    template<class VELOCITY>
    static typename SpaceTime::Scalar d_dt(const VELOCITY &vel) { return STATICFIELD::d_dt(vel); }
};


template<StaticSecondOrderField STATICFIELD, STATICFIELD::SpaceTime::Time Offset>
class HomogeneousField<STATICFIELD,Offset> {
public:
    typedef STATICFIELD::SpaceTime SpaceTime;

    SpaceTime origin;
    HomogeneousField(SpaceTime x) : origin(std::move(x)) {}

    auto operator ()(const SpaceTime &pos) const { return STATICFIELD::value(pos-origin) + Offset; }
    auto d_dt(const SpaceTime &pos, const SpaceTime &vel) const { return STATICFIELD::d_dt(pos-origin, vel); }
    template<class VELOCITY>
    static auto d2_dt2(const VELOCITY &vel) { return STATICFIELD::d2_dt2(vel); }
};


template<StaticHigherOrderField STATICFIELD, STATICFIELD::SpaceTime::Time Offset>
class HomogeneousField<STATICFIELD,Offset> {
public:
    typedef STATICFIELD::SpaceTime SpaceTime;

    SpaceTime origin;
    HomogeneousField(SpaceTime x) : origin(std::move(x)) {}

    auto operator ()(const SpaceTime &pos) const { return STATICFIELD::value(pos-origin) + Offset; }
    auto d_dt(const SpaceTime &pos, const SpaceTime &vel) const { return STATICFIELD::d_dt(pos-origin, vel); }
    auto d2_dt2(const SpaceTime &pos, const SpaceTime &vel) const { return STATICFIELD::d2_dt2(pos-origin, vel); }
};


#endif
