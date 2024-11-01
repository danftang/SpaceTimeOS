#ifndef CONCEPTS_H
#define CONCEPTS_H

#include <functional>


// There are two fundamental orderings on the set of agent states:
//      - the agent trajectory ordering
//      - the agent execution zone [not an ordering] (an agent at the source state can send a message to an agent at the target)
// The union of the two must also be a partial ordering (or equivalently, state-transition and communication must be
// subsets of a partial ordering). The sphere of influence of an agent in state A is all the points
// that are greater than A in this ordering.
//
// If agent A has a channel to B, B must block if it touches the execution zone of A convolved with the trajectory
// cone.
//
// If one of the graphs is a superset of the other then at least we don't have to do the union.
//
// Without any structure to the space, calculating whether one agent is in the sphere of influence of the other
// requires clculating whether there exists a directed path between points, which can be computationally expensive
// but by embedding agents in a vector space and constraining the state-transition and communication graphs
// to efficiently calculated geometric structurs, then we make the problem much more efficient.
//
// In particular, if we require that the ordering conforms to the + and * operators of the vector space
// then we have a homogeneous space and graph union becomes convolution.
// 
// So, we are generalising a fully ordered, 1 dimensional time to any vector space 
// (and specialising arbitrary partial order to ordered vector spaces).
//
// Any ordered vector space is a fixpoint w.r.t. convolution of the ordering with itself so if both the state transition
// and communication graphs are countained within the ordering, then their convolution is also contained within
// that ordering, so we can safely block on the ordering. However, this is wider than necessary, especially if we
// don't require that agents at the same position in spacetime can communicate (e.g. if it still takes some time)
// The Move/Communicate set can then be convolved with itself repeatedly in order to get the information-flow set
// 




// A spacetime is simply an partially ordered set
// Note that since it is 
// template<class T>
// concept SpaceTime = requires(T::Position point) {
//     typename T::Position;

//     // Defines a parial order on positions. An agent's trajectory must be completely ordered w.r.t. this ordering
//     {point < point } -> std::same_as<bool>; 
    
//     // A position in spacetime that is in the future of all other positions (i.e. the spacetime position at which the universe ends).
//     { T::TOP } -> std::convertible_to<T>;
//     // A position in spacetime that is in the past of all other positions (i.e. the spacetime position at which the universe starts).
//     { T::BOTTOM } -> std::convertible_to<T>;
// };

// A spacetime that is a vector space. i.e. a homogeneous space wehere we can add/subtract spacetime positions
// to get displacements and we can multiply positions by scalars to get positions. If an agent has a position in such a space
// it makes sense to talk of its 4-velocity at any point on its trajectory. In discrete space, this will just be the
// difference between successive states (forward finite difference). In continuous space, this will be a tangent to the 
// trajectory at that point. With the addition of a metric, this can be a unit tangent. 
//
// Note that the + operation should form a group. i.e. it satisfies, for some unique origin O:
//      1)  for all X: X + O = X
//      2)  for all X,Y,Z: (X + Y) + Z = (X + Y) + Z
//      3)  for all X there exists a unique Y: X + Y = Y + X = O [note that this defines unary negation, which defines subtraction]
// 
// The Scalar should be a field (i.e. have + and * operations) which satisfies, for scalars a,b and vectors X,Y:
//      1) (X*b)*a = X*(b*a)
//      2) there exists a unique scalar 1 such that for all X: X*1=X
//      3) (X+Y)*a = X*a + Y*a
//      4) X*(a+b) = X*a + X*b
//
// We also add a cast operation to "lab time" which is an arbitrary (i.e. computationally convenient) 
// mapping to a complete ordering which preserves the partial ordering.
// 
// Note that any partial ordering on a vector space should satisfy:
//   1) X < Y implies X + C < Y + C for all X,Y,C
//   2) O < X implies O < Xt for all t>0 where O is the origin, 0 is the group identity element of the scalar and we assume the scalar is ordered
// Given this, a partial ordering on the vector space can be defined by a "positive cone" {X : O < X},
// where X < Y iff O < Y-X (i.e. iff Y-X is in the positive cone). This automatically satisfies constraint (1)
// but the positive cone should satisfy (2). Any set, V, induces a cone {X : there exists a t: t>0 and Xt is in V } 
// So, if we define a set of velocities, V, then this induces a cone which defines a partial order.
// It also defines a distance between any two ordered points X<Y s(X,Y) = t : Y-X = Zt for some Z in V.
// 
// However, while all partial orders induce a positive cone, not all positive cones induce a partial order
// since we need to ensure that there are no loops. For this we require that if X is in the positive cone then -X is not
// since if O < X then O - X < X-X so -X < O.
template<class T>
concept SpaceTime = requires(T position, T offset, T velocity, T::Time time) {
//    requires SpaceTime<T>;

    // An agent's velocity gives its change in spacetime position per unit local-time.
    // so defines the forward pointing tangent to the agent's trajectory.
    // By defining a separate type for velocity we can constrain the tangents that an
    // agent may have, and so constrain the set of possible trajectories.
    // By separating position and velocity we are comitting to a homogeneous space-time.
    // (inhomogeneiry must be via additional space-dependent constraints or boundaries)
  //  typename T::Velocity;

    // Time defines the local-time of an agent, with which we can refer to points on an agent's
    // trajectory, or distances along a trajectory.
    typename T::Time;

    { static_cast<typename T::Time>(position) }; // should return lab time i.e. L.X for some reference point L.

    { position + offset } -> std::convertible_to<decltype(position)>; // this is only necessary if we assume a homogeneous space

    { position - position } -> std::convertible_to<decltype(offset)>;

    // Should return the change in spacetime position when moving at a given velocity for a given local-time (assumes homogeneity of
    // spacetime, though we can constrain velocity based on position via a boundary)
    // [If this is a one-one function, this defines a metric given by the unique local time between points. It also defines a
    //  light cone given by the range of this function, i.e. { x : v*t=x for some v and t }, and future light cone if t>0,
    // and a set of points { x : x = velocity*1 for some velocity } which defines the set of possible velocities (after one second, though
    // a velocity need not be constant...probably worth distinguishing velocity from acceleration from trajectory...?)]
    { velocity * time } -> std::convertible_to<decltype(offset)>;

    // inner product. If V is the lab-frame origin at a labtime of 1, then V.V should equal 1.
    { position * position } -> std::convertible_to<typename T::Time>;

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


// A Field is a differentiable function over a vector space which is used to define a
// subset of points, F(x) > 0. 
// Note that a differentiable field can exist on a discrete domain via an imaginary
// mapping of discrete points into a continuous vector space.
//
// Note also that in a vector field we expect x < y implies x - c < y - c for all c (i.e. homogeneity of space)
// so x - x < y - x. So we can define a scalar field F(x) > 0 implies x > o. where o is the
// origin. Given this, then F(y-x)>0 implies y-x > o which implies y > x.
// So the field defines a partial ordering.
//
// [To be an ordering we require x < y < z implies x < z. This is guaranteed for all first order fields,
// but not all second order fields (convexity of space-like region...?)...]
// 
template<class T>
concept Field = requires(T field, T::SpaceTime x) {
    typename T::SpaceTime;
    requires SpaceTime<typename T::SpaceTime>;
    { field(x) };         // value of field at point, F(x)
};

// A field of the form F(X) = BX + c for some constant vector B and constant c.
template<class T>
concept FirstOrderField = requires(T field, T::SpaceTime x, T::SpaceTime v) {
    requires Field<T>;
    { field.d_dt(v) };    // dF(vt)/dt = dF(x)/dx dx/dt (should be independent of x and t if first order)
};


// A field of the form F(X) = XMX + BX + c for some matrix M, vector B and constant s.
// A second order field can define either one or two intervals on an agent's trajectory. If two, it is the
// later of the two that we consider as the defining region. (i.e. in both cases, the region begins 
// at the point on the agent's trajectory when the field crosses zero with a positive gradient along
// the agent's 4-velocity.) 
template<class T>
concept SecondOrderField = requires(T field, T::SpaceTime x, T::SpaceTime v) {
    requires Field<T>;
    { field.d_dt(x, v) };       // dF(x+vt)/dt at t=0
    { field.d2_d2t(v) };        // d^2F(vt)/dt^2 (should be independent of point and t)
//    { field.valAndDiffs(x,v) }; // returns value and first and second order differentials [may be quicker to compute all together]
};


template<class T>
concept HigherOrderField = requires(T field, T::SpaceTime x, T::SpaceTime v) {
    requires Field<T>;
    { field.d_dt(x, v) };       // dF(x+vt)/dt at t=0
    { field.d2_d2t(x, v) };     // d^2F(x+vt)/dt^2 at t=0
//    { field.valAndDiffs(x,v) }; // returns value and first and second order differentials [may be quicker to compute all together]
};

template<class T>
concept DifferentiableField = Field<T> && (FirstOrderField<T> || SecondOrderField<T> || HigherOrderField<T>); // For now, only these types of field


template<class T>
concept StaticField = requires(T::SpaceTime x, T::SpaceTime v) {
    typename T::SpaceTime;
    requires SpaceTime<typename T::SpaceTime>;
    { T::value(x) };         // value of field at point, F(x)
};


template<class T>
concept StaticFirstOrderField = requires(T::SpaceTime x, T::SpaceTime v) {
    requires StaticField<T>;
    { T::d_dt(v) };    // dF(vt)/dt = dF(x)/dx dx/dt (should be independent of x and t if first order)
};

template<class T>
concept StaticSecondOrderField = requires(T::SpaceTime x, T::SpaceTime v) {
    requires StaticField<T>;
    { T::d_dt(x, v) };       // dF(x+vt)/dt at t=0
    { T::d2_d2t(v) };        // d^2F(vt)/dt^2 (should be independent of point and t)
};

template<class T>
concept StaticHigherOrderField = requires(T::SpaceTime x, T::SpaceTime v) {
    requires StaticField<T>;
    { T::d_dt(x, v) };       // dF(x+vt)/dt at t=0
    { T::d2_d2t(x, v) };     // d^2F(x+vt)/dt^2 at t=0
};

template<class T>
concept StaticDifferentiableField = StaticField<T> && (StaticFirstOrderField<T> || StaticSecondOrderField<T> || StaticHigherOrderField<T>); // For now, only these types of field


// An inner product space is a spacetime that has an inner product with the properties
//   - (x+y).z = x.z + y.z
//   - x.y = y.x
//   - (v*t).x = t*(v.x) for any scalar t
//   - 0.0 = 0 where the LHS 0s are the default constructed spacetime positions and the RHS is zero.
// Note, we do not require x.y to be non-negative.
// template<class T>
// concept InnerProductSpace = requires(T::Position displacement, T::Velocity velocity) {
//     requires VectorSpace<T>;

//     // The inner product defines a distance measure x.x
//     // which defines the square of the local time for an agent to move distance x
//     // This defines velocities to be v.v=1 and ((Time)v)>0
//     // and x < y iff (x-y).(x-y) > 0 and ((Time)x) < ((Time)y)
//     // An inner product should satisfy x.y = y.x is real, and (x+y).z = x.z + y.z
//     // This constrains the form to XMY for some real matrix M.
//     // Since we only need to constrain XMX, M can always be made symmetric.
//     // Since we are free to choose the basis to represent this space in,
//     // we can always choose a basis such that M is diagonal with elements in {-1,+1}
//     // since a real, symmetrical matrix can always be eigendecomposed into the form M = Q^TDQ
//     // where D is diagonal.
//     // If we canonically put the positive elements first, then, up to linear transformations
//     // there are only N N-dimensional inner product spaces, N-1 if we insist on at least one
//     // timelike basis.
//     { displacement * displacement } -> std::convertible_to<typename T::Time>; // actually units of time^2: product of times for displacements in the frame of (either) one of the displacements 

//     // By distinguishing displacement from velocity, we can generalise inner product to any
//     // second order field, while maintaining linearity, such that
//     // X.(Y + Vt) = X.Y + (X.V)t
//     // If the ordering field can be expressed in the form F(X) = XMX then X.V is the same function as X.X
//     // but if F(X) = X^TMX + NX + C then
//     // F(X + Vt) = (X+Vt)M(X+Vt) + N(X+Vt) + C = XMX + NX + C + (VMX + XMV  + NV)t + t^2 = XMX + NX + C + (2XM  + N)Vt + t^2
//     // where we implicitly assume velocity is defined such that VMV = 1
//     // But
//     // (X + Vt).(X + Vt) = X.X + (V.X + X.V)t + V.Vt^2 = X.X + 2X.Vt + V.Vt^2
//     // so if we define
//     // X.Y = XMY + N(X+Y)/2 + C
//     // and
//     // X.V = V.X =  (VMX + XMV  + NV)/2 = XMV + NV/2  (i.e. X.V gives the gradient of F at X along V)
//     // and define velocity such that
//     // V.V = 1 (unitless) [or, perhaps we relax this to allow V.V to be any constant, in which case we can have VMV = 0 and this can signal a linear field]
//     // then F(X + Vt) = (X+Vt).(X+Vt)
//     // and
//     // X.(Y+Vt) = X.Y + X.Vt = XMY + N(X+Y)/2 + C + XMVt + NVt/2
//     //                       = XM(Y + Vt) + N(X+Y+Vt)/2 + C
//     //
//     // Vt must then be an offset within the field which has a dot product like velocity
//     // Gives the time to move displacement as measured in a frame moving at given velocity
// //    { displacement * velocity } -> std::convertible_to<typename T::Time>; // units of time since velocity is per unit time

//     // Alternatively, we could assume dot prod is of form XMY and have separate C and N fields.
//     // Given a second order field:
//     // F(X) = X^TMX + NX + C then
//     // F(X + Vt) = (X+Vt)^TM(X+Vt) + N(X+Vt) + C = X^TMX + NX + C + (V^TMX + X^TMV  + NV)t + V^TMVt^2 = F(X) + (2X^TM  + N)Vt + V^TMVt^2
//     // So all we really need to know is V^TMX + X^TMV  + NV as a function of X and V.
// };




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
//
// In order to represent timestepping agents we could execute autonomous time steps when they intersect with a loop
// field/channel. An autonomous step results in the emission of another step down the loop channel and a jump to a new spacetime 
// position where it will
// immediately intersect with the already emitted next step. The trajectory then becomes the zero velocity null trajectory
// where time to intersection is zero if current position is intersecting with a field or infinite (i.e. will never intersect)
// if not. The null trajectory has no requirement for the spacetime to be a vector space.
template<class T>
concept Trajectory = requires(T trajectory, T::SpaceTime point, T::Time time) {
    typename T::SpaceTime;
//    requires SpaceTime<typename T::SpaceTime>;

    typename T::Time;
    { time < time } -> std::same_as<bool>;
    { trajectory.timeToIntersection(point) } -> std::convertible_to<typename T::Time>;
    { trajectory.advanceBy(time) };
    { trajectory.jumpTo(point) };
    { trajectory.position() } -> std::convertible_to<typename T::SpaceTime>; // position at current time
};


template<class T>
concept Simulation = requires(T simulation, std::function<void()> runnable) {
    typename T::Time;
    typename T::SpaceTime;
    typename T::Trajectory;
    typename T::LambdaField;
    typename T::BlockingField;
    typename T::Metric;
    typename T::Executor;
    
    requires Trajectory<typename T::Trajectory>;

    { T::executor.submit(runnable) };
    { T::boundary };            // A boundary defines a global lambda that all agents execute on intersection but don't absorb
    // typename T::SpaceTime;

    // typename T::Trajectory;
};

#endif
