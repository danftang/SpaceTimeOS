#ifndef MINKOWSKIFIELD_H
#define MINKOWSKIFIELD_H

#include "TupleSpace.h"
#include "Velocity.h"


// The positive cone as defined by the Minkowski metric:
//  X_0*X_0 - sum_d=1^N X_d*X_d > offset and dF/dt > 0
template<class... TYPES>
class MinkowskiField {
public:
    typedef TupleSpace<TYPES...> SpaceTime;
    typedef decltype(value(std::declval<SpaceTime>())) Range;

    // TupleSpace<TYPES...> origin;

    // inline auto operator()(const SpaceTime &pos) {
    //     return value(pos-origin);
    // }

    // inline auto d_dt(const SpaceTime &pos, const SpaceTime &vel) {
    //     return innerProd(pos-origin,vel)*2;
    // }

    // inline auto d2_dt2(const SpaceTime &vel) {
    //     return innerProd(vel,vel);
    // }


    inline static auto value(const SpaceTime &pos) {
        return innerProd(pos,pos);
    }

    inline static auto d_dt(const SpaceTime &pos, const SpaceTime &vel) {
        return innerProd(pos,vel)*2;
    }

    inline static auto d2_dt2(const SpaceTime &vel) {
        return innerProd(vel,vel);
    }

    inline static constexpr Range d2_dt2(const Velocity<MinkowskiField<TYPES...>> &) { return 1; }

    // inline auto valAndDiffs(const SpaceTime &pos, const SpaceTime &vel) {
    //     auto displacement = pos-origin;
    //     return std::tuple(innerProd(displacement,displacement), innerProd(displacement,vel)*2, innerProd(vel,vel));
    // }

    template<stze_t...SPACEINDICES>
    inline static auto innerProd(const TupleSpace<Indices<0,SPACEINDICES...>,TYPES...> &x, const TupleSpace<TYPES...> &y) {
        return ((std::get<0>(x) * std::get<0>(y)) - ... - (std::get<SPACEINDICES>(x) * std::get<SPACEINDICES>(y)));
    }

};


#endif
