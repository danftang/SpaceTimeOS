#ifndef TUPLESPACE_H
#define TUPLESPACE_H

#include <concepts>
#include <tuple>
#include <ostream>

template<class...TYPES>
using IndicesFor = decltype(std::index_sequence_for<TYPES...>());

template<size_t...INDICES>
using Indices = std::index_sequence<INDICES...>;

/// 

/// @brief An element of a vector space stored as a tuple.
/// Dimensions can be of different types, but each should define the + and - operators with themselves
/// and multiplication by the first type, which is taken to be the scalar type of the vector field.
/// @tparam ...TYPES of each dimension. First of which defines the scalar of the vector space.
template<class... TYPES>
class MinkowskiSpace : public MinkowskiSpace<IndicesFor<TYPES...>,TYPES...> {
public:
    typedef std::tuple_element_t<0,std::tuple<TYPES...>> Time;

    typedef MinkowskiSpace<IndicesFor<TYPES...>,TYPES...> Indexed;

    // We assume that the first element is time-like and defines lab-time
    MinkowskiSpace() = default;
    MinkowskiSpace(Indexed::Time t) : MinkowskiSpace() { 
        std::get<0>(*this) = t;
    }
    MinkowskiSpace(TYPES &&... values) : Indexed(std::tuple<TYPES...>(std::forward<TYPES>(values)...)) { }
    MinkowskiSpace(std::tuple<TYPES...> && tuple) : Indexed(std::move(tuple)) { }
    MinkowskiSpace(const std::tuple<TYPES...> & tuple) : Indexed(std::tuple<TYPES...>(tuple)) { }

};


template<size_t...INDICES, class... DIMTYPES>
class MinkowskiSpace<std::index_sequence<INDICES...>,DIMTYPES...> : public std::tuple<DIMTYPES...> {
public:
    typedef std::tuple_element_t<0,std::tuple<DIMTYPES...>> Time;

    static constexpr size_t DIMENSIONS = sizeof...(DIMTYPES);

    MinkowskiSpace() = default;
    MinkowskiSpace(std::tuple<DIMTYPES...> && tuple) : std::tuple<DIMTYPES...>(std::move(tuple)) { }


    // Convert to lab time.
    explicit operator const Time &() const {
        return std::get<0>(*this);
    }

    const Time &labTime() const {
        return std::get<0>(*this);
    }

    MinkowskiSpace<DIMTYPES...> operator +(const MinkowskiSpace<DIMTYPES...> &other) const {
        return MinkowskiSpace<DIMTYPES...>((std::get<INDICES>(*this) + std::get<INDICES>(other))...);
    }

    MinkowskiSpace<DIMTYPES...> operator -(const MinkowskiSpace<DIMTYPES...> &other) const {
        return MinkowskiSpace<DIMTYPES...>((std::get<INDICES>(*this) - std::get<INDICES>(other))...);
    }

    MinkowskiSpace<DIMTYPES...> operator *(Time t) const {
        return MinkowskiSpace<DIMTYPES...>((std::get<INDICES>(*this) * t)...);
    }

    MinkowskiSpace<DIMTYPES...> &operator *=(Time t) {
        ((std::get<INDICES>(*this) *= t), ...);
        return *this;
    }

    MinkowskiSpace<DIMTYPES...> &operator +=(const MinkowskiSpace<DIMTYPES...> &other) {
        ((std::get<INDICES>(*this) += std::get<INDICES>(other)), ...);
        return *this;
    }

    MinkowskiSpace<DIMTYPES...> &operator -=(const MinkowskiSpace<DIMTYPES...> &other) {
        ((std::get<INDICES>(*this) -= std::get<INDICES>(other)), ...);
        return *this;
    }

    // We can always cast this to a Tuple without indices.
    operator MinkowskiSpace<DIMTYPES...> &() {
        return *static_cast<MinkowskiSpace<DIMTYPES...> *>(this);
    }

    // Assume Minkowski for now. Split this off later.
    // If we cast the inner product to Time type we ensure there are vectors of length 1 even when time is integer
    // and every transition has a well defined distance.
    template<size_t... SPACEINDICES>
    Time operator *(const MinkowskiSpace<std::index_sequence<0,SPACEINDICES...>, DIMTYPES...> &other) const {
        return ((std::get<0>(*this) * std::get<0>(other)) - ... - (std::get<SPACEINDICES>(*this) * std::get<SPACEINDICES>(other)));
    }

    bool operator <(const MinkowskiSpace<DIMTYPES...> &other) {
        auto DX = *this - other;
        return labTime() < other.labTime() && DX*DX >= 0;
    }

    bool operator <=(const MinkowskiSpace<DIMTYPES...> &other) {
        auto DX = *this - other;
        return labTime() <= other.labTime() && DX*DX >= 0;
    }

    friend std::ostream &operator <<(std::ostream &out, const MinkowskiSpace<std::index_sequence<INDICES...>,DIMTYPES...> &tSpace) {
        out << "( ";
        ((out << std::get<INDICES>(tSpace) << " "),...);
        out << ")";
        return out;
    }

};


// calculates sum_i lhs_i*rhs_i
template<size_t... INDICES, class... TYPES>
auto dotProduct(const MinkowskiSpace<std::index_sequence<INDICES...>, TYPES...> &lhs, const MinkowskiSpace<TYPES...> &rhs) {
    return ((std::get<INDICES>(lhs) * std::get<INDICES>(rhs)) + ...);
}

template<size_t... INDICES, class... TYPES>
MinkowskiSpace<TYPES...> elementwiseProduct(const MinkowskiSpace<std::index_sequence<INDICES...>, TYPES...> &lhs, const MinkowskiSpace<TYPES...> &rhs) {
    return MinkowskiSpace<TYPES...>((std::get<INDICES>(lhs) * std::get<INDICES>(rhs))...);
}

template<size_t... INDICES, class... TYPES>
MinkowskiSpace<TYPES...> elementwiseProduct(MinkowskiSpace<std::index_sequence<INDICES...>, TYPES...> &&lhs, const MinkowskiSpace<TYPES...> &rhs) {
    ((std::get<INDICES>(lhs) *= std::get<INDICES>(rhs)), ...);
    return std::move(lhs);
}


// template<size_t SIZE, class LAMBDA, class... ARGS>
// inline auto callWithIndices(LAMBDA &&templatedLambda, ARGS &&... args) {
//     return callWithTemplates(std::make_index_sequence<SIZE>(), std::forward<LAMBDA>(templatedLambda), std::forward<ARGS>(args)...);
// }

// template<class... TYPES, class LAMBDA, class... ARGS>
// inline auto callWithIndicesFor(LAMBDA &&templatedLambda, ARGS &&... args) {
//     return callWithTemplates(std::index_sequence_for<TYPES...>(), std::forward<LAMBDA>(templatedLambda), std::forward<ARGS>(args)...);
// }

// template<class LAMBDA, size_t...INDICES, class... ARGS>
// inline auto callWithTemplates(std::index_sequence<INDICES...>, LAMBDA &&templatedLambda, ARGS &&...args) {
//     return templatedLambda.template operator()<INDICES...>(std::forward<ARGS>(args)...);
// }

#endif
