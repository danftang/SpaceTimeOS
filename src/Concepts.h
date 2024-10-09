#ifndef CONCEPTS_H
#define CONCEPTS_H

#include <functional>


// A spacetime is simply an partially ordered set
// Note that since it is 
template<class T>
concept SpaceTime = requires(T point) {
    {point < point } -> std::same_as<bool>;
    
    // A position in spacetime that is in the future of all other positions (i.e. the spacetime position at which the universe ends).
    { T::TOP } -> std::convertible_to<T>;
    // A position in spacetime that is in the past of all other positions (i.e. the spacetime position at which the universe starts).
    { T::BOTTOM } -> std::convertible_to<T>;
};

// A vector spacetime 
template<class T>
concept VectorSpaceTime = requires(T position, T offset, T::Time time, T::Velocity velocity) {
    requires SpaceTime<T>;

    // Velocity is the type used to measure the change in spacetime position per unit local-time
    // so defines a trajectory through spacetime parameterised by time.
    // or more generally, is just an encoding of an agent's event-free trajectory
    typename T::Velocity; 
    typename T::Time;

    { position.labTime() } -> std::convertible_to<typename T::Time>;

    { position + offset } -> std::convertible_to<decltype(position)>; // this is only necessary if we assume a homogeneous space

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
    // [Note that this only makes sense when Velocity is a subset of spacetime points or a different type entirely, otherwise t is always 0
    //  in this way, the Velocity type sneaks in the inner product (or at least a distance measure) by imposing |v.v| = 1]
    // This assumes the type of v is the same as that of v', but that need not be the case.
    // so we can imagine multiple velocity types...however, to constrain informaiton flow we need to specify the union of all
    // these, which is the one that should be defined in the spacetime.
//    { offset / velocity } -> std::convertible_to<decltype(time)>;

};


// An inner product space is a spacetime that has an inner product with the properties
//   - (x+y).z = x.z + y.z
//   - x.y = y.x
//   - (v*t).x = t*(v.x) for any scalar t
//   - 0.0 = 0 where the LHS 0s are the default constructed spacetime positions.
// Note, we do not require x.y to be non-negative.
template<class T>
concept InnerProductSpace = requires(T point) {
    requires VectorSpaceTime<T>;

    { point * point } -> std::convertible_to<typename T::Time>;
};


template<class T>
concept Executor = requires(T executor, std::function<void()> runnable) {
    { executor.submit(runnable) };
    { executor.join() }; 
};


// template<class T, class AGENT>
// concept ForceCarrier = requires(T forceCarrier, AGENT agent) {
//     { forceCarrier.timeToIntersection(agent.position(), agent.velocity()) } -> std::convertible_to<typename T::Scalar>;
//     { forceCarrier.execute(agent) };
// };


// Note that a differentiable field can exist on a discrete domain.
template<class T>
concept SecondOrderField = requires(T field, T::SpaceTime point, T::SpaceTime velocity) {
    requires VectorSpaceTime<typename T::SpaceTime>;

    { field(point) };                   // value at point
    { field.d_dt(point, velocity) };    // dF/dt along x(t) = point + velocity*t at t=0
    { field.d2_d2t(velocity) };         // d2F/dt2 along x(t) = point + velocity*t (should be independent of point and t)
};

// Note that a differentiable field can exist on a discrete domain.
template<class T>
concept FirstOrderField = requires(T field, T::SpaceTime point, T::SpaceTime velocity) {
    requires VectorSpaceTime<typename T::SpaceTime>;

    { field(point) };            // value at point
    { field.d_dt(velocity) };    // dF/dt along x(t) = point + velocity*t (should be independent of point and t)
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
template<class T>
concept Trajectory = requires(T trajectory, T::SpaceTime point, T::Time time) {
    typename T::SpaceTime;
    requires SpaceTime<typename T::SpaceTime>;

    typename T::Time;
    { time < time } -> std::same_as<bool>;
    { trajectory.timeToIntersection(point) } -> std::convertible_to<typename T::Time>;
    { trajectory.advanceBy(time) };
    { trajectory.jumpTo(point) };
    { trajectory.position() } -> std::convertible_to<typename T::SpaceTime>; // position at current time
};


template<class T>
concept Simulation = requires(T simulation, std::function<void()> runnable) {
    typename T::Trajectory;
    requires Trajectory<typename T::Trajectory>;

    { T::submit(runnable) };
    { T::boundary };            // A boundary defines a global lambda that all agents execute on intersection but don't absorb
    // typename T::SpaceTime;

    // typename T::Trajectory;
};

#endif
