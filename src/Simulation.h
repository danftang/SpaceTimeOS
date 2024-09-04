#ifndef SIMULATION_H
#define SIMULATION_H

#include <concepts>
#include <functional>
#include "spacetime/SpaceTime.h"

template<class T>
concept Simulation = requires(std::function<void()> task, T::SpaceTime position) {
    typename T::SpaceTime;
//    typename T::Frame;

    requires SpaceTime<typename T::SpaceTime>;

//    { T::submit(task) };
//    { T::template spawnAt<int>(position) };// -> std::same_as<SpaceTimePtr<int,T>>;
};

#endif
