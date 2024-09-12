#ifndef CONCEPTS_H
#define CONCEPTS_H

#include <functional>

template<class T>
concept SpaceTime = requires(T position, T::ScalarType distance, std::function<void()> task, int dim) {
    // ScalarType is the type used to measure each dimension
    typename T::ScalarType;

    // A spacetime should be a vector space
    { position + position } -> std::convertible_to<T>;
    { position - position } -> std::convertible_to<T>;
    { position * distance } -> std::convertible_to<T>;

    // A position in spacetime that is in the future of all other positions (i.e. the spacetime position at which the universe ends).
    { T::TOP } -> std::convertible_to<T>;
};

template<class T>
concept DefinesSpaceTime = requires() {
    typename T::SpaceTime;
    requires SpaceTime<typename T::SpaceTime>;
};

template<class T>
concept Executor = requires(T executor, std::function<void()> runnable) {
    { executor.submit(runnable) };
    { executor.join() }; 
};


template<class T>
concept Simulation = requires(std::function<void()> task, T::SpaceTime position) {
    typename T::SpaceTime;
    requires SpaceTime<typename T::SpaceTime>;
};

#endif
