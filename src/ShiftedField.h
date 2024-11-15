#ifndef SHIFTEDFIELD_H
#define SHIFTEDFIELD_H

#include "Concepts.h"
#include "Velocity.h"

// template<StaticDifferentiableField STATICFIELD> class ShiftedField {};


// template<StaticFirstOrderField STATICFIELD>
// class ShiftedField<STATICFIELD> {
// public:
//     typedef STATICFIELD::SpaceTime SpaceTime;

//     SpaceTime origin;
//     ShiftedField(SpaceTime x = SpaceTime()) : origin(std::move(x)) {}

//     auto operator ()(const SpaceTime &pos) const { return STATICFIELD::value(pos-origin); }
//     template<class VELOCITY>
//     static typename SpaceTime::Scalar d_dt(const VELOCITY &vel) { return STATICFIELD::d_dt(vel); }
// };


// template<StaticSecondOrderField STATICFIELD>
// class ShiftedField<STATICFIELD> {
// public:
//     typedef STATICFIELD::SpaceTime SpaceTime;

//     SpaceTime origin;
//     ShiftedField(SpaceTime x = SpaceTime()) : origin(std::move(x)) {}

//     auto operator ()(const SpaceTime &pos) const { return STATICFIELD::value(pos-origin); }
//     auto d_dt(const SpaceTime &pos, const SpaceTime &vel) const { return STATICFIELD::d_dt(pos-origin, vel); }
//     template<class VELOCITY>
//     static auto d2_dt2(const VELOCITY &vel) { return STATICFIELD::d2_dt2(vel); }
// };


// template<StaticHigherOrderField STATICFIELD>
// class ShiftedField<STATICFIELD> {
// public:
//     typedef STATICFIELD::SpaceTime SpaceTime;

//     SpaceTime origin;
//     ShiftedField(SpaceTime x = SpaceTime()) : origin(std::move(x)) {}

//     auto operator ()(const SpaceTime &pos) const { return STATICFIELD::value(pos-origin); }
//     auto d_dt(const SpaceTime &pos, const SpaceTime &vel) const { return STATICFIELD::d_dt(pos-origin, vel); }
//     auto d2_dt2(const SpaceTime &pos, const SpaceTime &vel) const { return STATICFIELD::d2_dt2(pos-origin, vel); }
// };


#endif
