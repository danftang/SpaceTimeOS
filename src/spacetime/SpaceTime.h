#include <concepts>
#include <functional>

template<class T>
concept SpaceTime = requires(T position) {
    // A spacetime can be bounded by returning false to isWithinBounds()
    { position.isWithinBounds() } -> std::same_as<bool>;
};
