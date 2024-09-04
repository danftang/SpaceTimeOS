#ifndef SPACETIME_H
#define SPACETIME_H

#include <concepts>
#include <functional>

template<class T>
concept SpaceTime = requires(T position, T::ScalarType distance, std::function<void()> task, int dim) {
    // ScalarType is the type used to measure each dimension
    typename T::ScalarType;

    // Returns the co-ordinate in the dim'th dimension in the laboratory frame.
    // the 0'th dimension is time.
//    { position[dim] } -> std::convertible_to<typename T::ScalarType>;

    // Set the coord in a given dimension
//    { position[dim] = distance };

    // A spacetime should be a vector space
    { position + position } -> std::convertible_to<T>;
    { position - position } -> std::convertible_to<T>;
    { position * distance } -> std::convertible_to<T>;

    // A position in spacetime that is in the future of all other positions (i.e. the spacetime position at which the universe ends).
    { T::TOP } -> std::convertible_to<T>;
};

#endif