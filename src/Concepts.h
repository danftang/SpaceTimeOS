#ifndef CONCEPTS_H
#define CONCEPTS_H

#include <functional>

// TODO: Should a SpaceTime define its own boundaries by defining a + b to return TOP if it is out
// of bounds? Any point on a boundary can be classified as time-like-incoming (agents can only enter the simulation at that point)
// time-like-outgoing (agents can only leave the simulation at that point) and space-like
// (an agent can be either incoming or outgoing, depending on its velocity)

template<class T>
concept SpaceTime = requires(T position, T offset, T::Scalar time, T::Velocity velocity) {
    // Scalar is the type used to measure each dimension
    typename T::Scalar;
    typename T::Velocity;

    { position + offset } -> std::convertible_to<T>;

//    { position - position } -> std::convertible_to<T>;

    { velocity * time } -> std::convertible_to<T>;

    { offset / velocity } -> std::convertible_to<typename T::Scalar>;

    // A position in spacetime that is in the future of all other positions (i.e. the spacetime position at which the universe ends).
    { T::TOP } -> std::convertible_to<T>;
    // A position in spacetime that is in the past of all other positions (i.e. the spacetime position at which the universe starts).
    { T::BOTTOM } -> std::convertible_to<T>;
};

template<class T>
concept IsSpatial = requires() {
    typename T::SpaceTime;
    requires SpaceTime<typename T::SpaceTime>;
};

template<class T>
concept Executor = requires(T executor, std::function<void()> runnable) {
    { executor.submit(runnable) };
    { executor.join() }; 
};

// template<class T>
// concept AgentEnvironment = requires(T::SpaceTime x, std::function<void()> runnable) {
//     typename T::SpaceTime;
//     requires SpaceTime<typename T::SpaceTime>;

//     { T::submit(runnable) };
//     { T::timeToIntersection(x,x) };
// //    { T::laboratory } -> std::derived_from<AgentBase<typename T::SpaceTime>>;
// };

template<class T, class AGENT>
concept ForceCarrier = requires(T forceCarrier, AGENT agent) {
    { forceCarrier.timeToIntersection(agent.position(), agent.velocity()) } -> std::convertible_to<typename T::Scalar>;
    { forceCarrier.execute(agent) };
};


template<class T, class MANIFOLD>
concept Trajectory = requires(T trajectory, MANIFOLD manifold, T::Scalar time) {
    { trajectory.timeToIntersection(manifold) } -> std::convertible_to<typename T::Scalar>;
    { trajectory.moveOriginTo(time) };
    { trajectory.origin() } -> std::convertible_to<typename T::SpaceTime>;
};

template<class T>
concept Manifold = requires(T manifold, T::Spacetime point) {
    { manifold.distanceTo(point) } -> std::convertible_to<typename T::Scalar>;
};


#endif
