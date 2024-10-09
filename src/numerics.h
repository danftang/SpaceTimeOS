#ifndef NUMERICS_H
#define NUMERICS_H

#include <numeric>
#include <concepts>
#include <cmath>

// deltaIncrement sets a numeric value to the smallest value that is
// strictly greater than it.
template<std::integral T>
T & deltaPreIncrement(T &i) { return ++i; }

template<std::floating_point T>
T & deltaPreIncrement(T &f) {
    return f += delta(f);
}

template<std::integral T>
T deltaPostIncrement(T &i) { return i++; }

template<std::floating_point T>
T deltaPostIncrement(T &f) {
    T originalF = f;
    f += delta(f);
    return std::move(originalF);
}

// delta gives the difference between a value and the smallest value that is strictly greater than it.
template<std::floating_point T>
T delta(const T &f) {
    return std::scalbn(std::numeric_limits<T>::epsilon(), std::ilogb(f));
}

template<std::integral T>
T delta(const T &f) {
    return 1;
}

#endif
