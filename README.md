# SpaceTimeOS

SpaceTimeOS is a new paradigm for distributed, multi-agent computation.

In this paradigm, agents are ordinary C++ objects embued with a trajectory through a shared, simulated space. Agents communicate by sending lambda functions (code and data) to each other. The lambda functions move at a finite speed through the simulated space so agents that are close to eachother can communicate more quickly (in simulated time) than agents further away. This allows efficient, automated assignment of computations to processing nodes, thus allowing simulations to be distributed across all cores of a single processor or many processors on a distributed network.

This paradigm fits naturally into simulations of agents moving around a spatial environment. However, by thinking of spacetime as a generalisation of time, we show how this paradigm is also useful even for simulations of agents that aren't initially considered in spatial terms.

## A simple example

To show how this works in practice, let's build a simple simulation of two agents that send messages back and forth between each other.

First we choose a class that defines what kind of simulation we want to do. Let's start with a simple forward simulation, so we choose the `ForwardSimulation` class. This class takes two templates: a trajectory type that defines the agents' trajectories between events and a resource that will perform the computations. Let's choose a linear trajectory in a Minkowski spacetime with three spatial dimensions and a time dimension (this describes the normal physical space we're all familiar with, but without gravitation) and a thread pool with two threads to do the computation. We'll define a type `MySimulation` for convenience:
```
typedef ForwardSimulation<LinearTrajectory<Minkowski<4>>, ThreadPool<2>>      MySimulation;
```

Next we make a class to define our agent's behaviour. Every agent should derive from `Agent<S>` where S defines the simulation type that the agent belongs to:
```
class Ping : public Agent<MySimulation> {
public:
    Channel<Ping> channelToOther;

    void ping() {
        std::cout << "Ping from " << position() << std::endl;
        channelToOther.send([](Ping &otherAgent) {
            otherAgent.ping();
        });
    }
};
```
If you already have a class that doesn't inherit from `Agent`, it can be made into an agent using the `AgentWrapper` class.

Our Ping agent has just one member function which sends a lambda function to the other agent using `channelToOther.send(...)`. The lambda function takes a reference to the other agent as its sole argument. When the lambda reaches the other agent, it will be executed with that agent as its argument, which will cause the other agent to send a lambda back...and so on...

Now all we need to do is initiate the simulation, connect the agents and set things going:
```
    Ping *alice = new Ping();
    Ping *bob   = new Ping();

    alice->jumpTo({0,0,0,0});
    bob->jumpTo({0,0,0,1});

    alice->channelToOther = Channel(*alice, *bob);
    bob->channelToOther   = Channel(*bob, *alice);

    alice->ping();

    MySimulation::start(100);
```

Here we create two agents, Alice and Bob, and move them to initial positions in spacetime (we don't specify velocity, so they both take on the velocity of the default reference frame). Next, we create new channels to connect Alice and Bob using `Channel(<source>,<target>)` and initiate the exchange by calling the `ping()` method on Alice. This emits an initial lambda, which will be delivered to Bob when we start the simulation. To do that we call `MySimulation::start(...)` with the time (in the laboratory frame) that the simulation should end. Each agent is deleted when it reaches the end of the simulation, so although we don't explicitly see the deletion, there is no memory leakage (the user can define a different behaviour at the boundary by specifying a different boundary type in the simulation).

The whole program can be found in [`main.cpp`](src/main.cpp) in this repository.

## Birth and death of agents

An agent can spawn new agents during a simulation by using the `new` operator in the normal way. A new agent is always created at the same position as the agent doing the creation. Once created, an agent can jump to any position that is in its future (see below for a definition of an agent's future).

Before a simulation is started, any code is executed on a unique "initialising agent" which resides at the beginning of time, so all positions are in this agent's future.

An agent can die at any time if it calls its `die()` method. At this point in spacetime all the agent's channels are closed and the agent is deleted.

## Sending channels to other agents

If Bob has a channel to Alice (let's call it `channelToAlice`), and a channel to Carol (`channelToCarol`), he may want to introduce Alice to Carol. To do this Bob needs to get a `RemoteReference` to Alice, which he does by calling `channelToAlice.target()`. He can then send the `RemoteReference` to Carol, who can then use it to create a channel to Alice. For example:
```
channelToCarol.send([remoteReferenceToAlice = channelToAlice.target()](Carol &carol) mutable {
    carol.channelToAlice = Channel(carol, remoteReferenceToAlice);
    ...
});
``` 

Note that Bob should never send Carol a raw pointer or reference to Alice because a reference at one spacetime location is not valid at any other location (e.g. Bob and Carol may not even reside on the same physical computer). He also shouldn't send a `Channel` directly, as a channel is between two fixed agents (to prevent this, `Channel` doesn't have a copy constructor so can't be captured in a `std::function`. To pass a `Channel` to a method, it must be moved rather than copied).

# Theory

Any multi-agent computation can be thought of in terms of a set of agents on which events occur. An agent can respond to an event by performing internal computations, creating new agents, destroying existing agents and/or causing more events. Our aim is to perform such a computation efficiently across multiple processors in such a way that the output of the computation is deterministic and independent of the number and nature of the processors (i.e. there are no race conditions and any randomness is taken from pseudo-random number generators with well defined seeds).

We assume that any two events on the same agent are non-commutative (i.e. executing them in a different order may lead to a different result) and that any two events on different agents are commutative (i.e. we can execute them in either order, and the result is the same - this comes naturally since an event on one agent can't directly affect another agent: it can only cause a separate event on another agent). 

A simple implementation of this is to timestamp every event with a point in simulated time. Agents cause new events by submitting them to a priority queue which executes them in order of simulated-time. Unfortunately this leaves no opportunity for parallelisation as the event currently being executed may cause another event that comes before (and causally affects) the next event on the queue, so we must wait for the current event to finish execution before we start the next event.

One solution to this problem would be to require that an event can only cause another event after a minimum "reaction time". We can then safely parallelise any events whose timestamp falls within a sliding window of duration equal to this minimum reaction time. This is particularly efficient if the agents are timestepping (i.e. their events are synchronised, so can all be executed in parallel). However, in most multi-agent computations, an event on one agent can only cause events on a small proportion of other agents in the simulation. The sliding-window algorithm doesn't account for this, so it misses opportunities for parallelisation that arise from the agent's limited opportunities for interaction (even in the timestepping case, as we otherwise need to wait for the most computationally intensive agent at every timestep).

We can solve this by keeping track of which agents can cause events on which other agents at any point in a computation. This can be done by allowing an agent to cause an event on another agent only if there is a communication channel between them. Given this, we can do away with the central event queue completely and have separate event queues on each agent. Every agent, $A$, processes their own events in order of simulated time until there is an agent that has a channel to $A$ and is lagging behind $A$ by more than the reaction time. If we implement the opening and closing of channels as events that must be sent over existing channels then this paradigm can also deal with runtime changes to the channels. An agent can use any convenient local processor to compute its events and should release the processor when it reaches a point where it "blocks" on another agent. Since an agent can only block on an agent that is at a simulated time earlier than it, there must always be at least one agent that isn't blocked and we can never reach a "gridlock" situation where there is a loop of agents each blocking on the other.

## Spacetime as a means to distributing multi-agent computation

The above algorithm goes a long way to our goal but we may get situations when one agent is blocking on another because it may potentially cause an event, but the other agent doesn't really care if the processing of the event is delayed for a while in simulated time, so there's no need to block. We generalise the above algorithm by noticing that by timestamping events we define a complete ordering on events, but this is a stronger constraint than is required to define a deterministic, multi-agent computation. All we really need to define is a complete ordering of events *on each individual agent*. Between agents, we only need to ensure that there can never be "gridlock" (i.e. a cycle of events each causing the other) so that the computation can always proceed. This is garanteed if we ensure that one event can only cause another event that has a timestamp greater than its own. However, these requirements can be satisfied with a "timestamp" taken from a set that is only [partially ordered](https://en.wikipedia.org/wiki/Partially_ordered_set), we'll call such a set a "spacetime". So, up until now, we've been stamping events with a fully-ordered, 1-dimensional spacetime (i.e. simulated time) and during a calcuation every agent has been at some point in simulated time, but now we allow "time" to be any partially ordered set. For example, we could use a 2-dimensional spacetime where each event is "timestamped" with two numbers. Given a point in spacetime, $x$, we call the set of points $\lbrace y : y > x \rbrace$ the *future* of $x$ and the set $\lbrace y : y < x \rbrace$ the *past* of $x$. Note that, since there is only a partial ordering on points, there may be points that aren't in $x$'s future or past.

In the 1-dimensional case, all agents simply moved forward in time, but now we've moved over to a partially ordered spacetime, an agent can follow different "trajectories" through spacetime. We define a trajectory as a subset of spacetime points which is completely ordered. This ensures there is a complete ordering of events on each individual agent. To ensure causal ordering between events, we specify that if an event at point $x$ (i.e. with timestap $x$) causes an event on a target agent $A$ then the caused event will occur at the earliest point on $A$ 's trajectory that is after $x$ in the partial ordering (if no such point exists, no event will be caused). If multiple events occur at the same point on an agent's trajectory, then the events are ordered by the order in which their channels were attached to that agent, starting with the most recently attached.

Any vector space with an inner product can be thought of as a spacetime. Given a reference unit vector $r$ (which defines an "arrow of time") we can define a partial ordering over the vector space as $y > x$ iff $y = x + vt$ for some scalar $t>0$ and some unit vector, $v$, such that $r.v > 0$. This is satisfied iff $(y-x).r > 0$ and $(y-x).(y-x) > 0$. 

An agent's trajectory can then be expressed as
$$
x(t) = x_0 + \int_0^t v(t') dt'
$$
where $t$ is a scalar we call the agent's "local time", $v(t)$ is a vector (which we call the 4-velocity) that satisfies $v(t).v(t) = 1$ and $v(t).r > 0$ for all t, and $x_0$ is that agent's position in spacetime at $t=0$. If local time is discrete, the above integral is replaced with a sum. These definitions ensure that all trajectories are fully ordered even though the spacetime itself may only be partially ordered. Notice that this definition does not require trajectories to be continuous - an agent may jump from one point to another at an event, as long as the jump preserves total ordering. This is useful if local time is continuous but the spacetime contains discrete variables (in which case, velocity should span only the real variables and any changes to discrete variables should be done by jumps at events).

A convenient way to define $v(t)$ is to specify that an agent's 4-velocity can only be changed by events. So between events, an agent follows a trajectory
$$
x(t) = x_0 + vt
$$
where $x_0$ is the position of the previous event and $t$ is the agent's local time since the last event. In this case, 

An example of a useful vector space is a Minkowski space whose inner product is $x.y = x_0y_0 - (x_1y_1 + x_2y_2 + x_3y_3)$ in 4-dimensions. Note the change in sign between this and the Euclidian inner product, $x.y = x_0y_0 + x_1y_1 + x_2y_2 + x_3y_3$. This small difference has a profound effect; the future of a point in Minkowski space describes a sphere centered on that point whose radius grows linearly with time. This is very different from the future of a Euclidean point which is a linear partitioning of spacetime. So, when an agent emits a lambda function in Minkowski space, it creates a sphere at the point of emission which grows until it touches the target agent and is absorbed. In this way lambda functions transmit information between agents at a finite speed.

## Boundary conditions

We define the boundary conditions of a simulation as a lambda function that is executaed by any agent that touches it. The contour of the boundary is defined by the zero points of a scalar field that fills the spacetime. A boundary can collect information from agents that touch it and can inject new agents or delete agents as appropriate.

## Implementation

To allow the runtime creation of new channels while ensuring that lambdas can never be sent into an agent's past we distinguish three events: the creation of the channel, the attachment of the channel to the sender and the attachment of the channel to the receiver. A channel is like an arrow through spacetime, so is associated with two trajectories: that of its source-end and that of its destination-end. When a channel is created, its source and destination ends are both at the point of creation. An unattached end of a channel can be sent down another channel (i.e. captured in a lambda), but its position is taken to be its creation point until it is attached to an agent. Once attached to an agent, a channel's end point trajectory follows that of the agent until the agent closes the channel.

In order to improve efficiency even more, we note that the order of emission of lambdas down a given channel is the same as the order of absorbtion at the target, so each channel can be implemented as a deque, and the target can identify the event with the earlist time to absorbtion by looking only at the front lambda on the deque of each incomming channel.


