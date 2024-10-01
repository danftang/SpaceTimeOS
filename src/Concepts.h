#ifndef CONCEPTS_H
#define CONCEPTS_H

#include <functional>

// TODO: Should a SpaceTime define its own boundaries by defining a + b to return TOP if it is out
// of bounds? Any point on a boundary can be classified as time-like-incoming (agents can only enter the simulation at that point)
// time-like-outgoing (agents can only leave the simulation at that point) and space-like
// (an agent can be either incoming or outgoing, depending on its velocity)

template<class T>
concept SpaceTime = requires(T position, T offset, T::Scalar time, T::Velocity velocity) {


    // Scalar is the type used to measure local time (only related to position via * and /)
    typename T::Scalar;     

    // Velocity is the type used to measure the change in spacetime position per unit local-time
    // so defines a trajectory through spacetime parameterised by time.
    typename T::Velocity;   

    { position + offset } -> std::convertible_to<decltype(position)>;

    { position - position } -> std::convertible_to<decltype(offset)>;

    // Should return the change in spacetime position when moving at a given velocity for a given local-time (assumes homogeneity of
    // spacetime, though we can constrain velocity based on position via a boundary)
    // [If this is a one-one function, this defines a metric given by the unique local time between points. It also defines a
    //  light cone given by the range of this function, i.e. { x : v*t=x for some v and t }, and future light cone if t>0,
    // and a set of points { x : x = velocity*1 for some velocity } which defines the set of possible velocities (after one second, though
    // a velocity need not be constant...probably worth distinguishing velocity from acceleration from trajectory...?)]
    { velocity * time } -> std::convertible_to<decltype(offset)>;

    // Given velocity, v, and offset, x, should return the minimum time, t, such that
    // there exists a v' and t'>0 such that v * t = x + v' * t'
    // (i.e. defines when an agent with velocity v will first intersect a force carrier emitted at offset from its current position).
    // This assumes the type of v is the same as that of v', but that need not be the case.
    // so we can imagine multiple velocity types...however, to constrain informaiton flow we need to specify the union of all
    // these, which is the one that should be defined in the spacetime.
    { offset / velocity } -> std::convertible_to<decltype(time)>;

    // A position in spacetime that is in the future of all other positions (i.e. the spacetime position at which the universe ends).
    { T::TOP } -> std::convertible_to<decltype(position)>;
    // A position in spacetime that is in the past of all other positions (i.e. the spacetime position at which the universe starts).
    { T::BOTTOM } -> std::convertible_to<decltype(position)>;
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

// A trajectory defines a map, T(s), from a fully ordered scalar, s, to points in a spacetime
// such that T(s_2) > T(s_1) iff s_2 > s_1.
// The scalar gives the distance along the trajectory from an origin in some
// arbitrary measure (perhaps the spacetime's dot product, as this ensures ordering
// and also defines the local time of the agent).
// Given a partition of space, there may exist a point at which the
// trajectory first intersects a given partition (i.e. lowest non-negative distance).
// If we define a partition as the points at which a scalar field, M, satisfy M(X)>0
// then the intersection point becomes finding the roots of M(T(s)),
// which can be done with Newton descent...?
//
// [Perhaps we could add the requirement that the
// partition is convex - or at least a set of disjoint convex sets
// or even more generally, perhaps we have a notion of any type of trajectory/partition
// pair on which a timeToIntersection is defined.
// ...or replace partition with a
// probability-rate of interaction (per unit dot product) field, and timeToInteraction becomes a sample from a 
// poisson process ]
template<class T, class MANIFOLD>
concept Trajectory = requires(T trajectory, MANIFOLD manifold, T::Scalar time) {
    { trajectory.timeToIntersection(manifold) } -> std::convertible_to<typename T::Scalar>;
    { trajectory.moveOriginTo(time) };
    { trajectory.origin() } -> std::convertible_to<typename T::SpaceTime>;
};


// A field over a space is a function from points in the spacetime to some value
// This can be used to define a partition by partitioning the space into positive
// and non-positive regions of the field.
// A metric is also a field.
template<class T, class RANGE>
concept Field = requires(T field, T::SpaceTime point, T::SpaceTime::Velocity velocity) {
    // { manifold(point) } -> std::convertible_to<RANGE>;

    // Returns the smallest non-negative time such that field(point + velocity*time) > 0.
    // or infinity if non existant. [does the field or the velocity know this best?!]
    { field.timeToIntersection(point, velocity) } -> std::convertible_to<typename T::SpaceTime::Scalar>;
};


#endif
