# SpaceTimeOS

SpaceTimeOS is a new paradigm for distributed, multi-agent computation.

In this paradigm, agents are ordinary C++ objects embued with a position and velocity in a shared, simulated space. Agents communicate with each other by sending lambda functions (code and data) between themselves. The lambda functions move at a finite speed through the simulated space so agents that are close to eachother can communicate more quickly than agents further away. This allows efficient, automated assignment of computations to processing nodes, thus allowing simulations to be distributed across all cores of a single processor or many processors on a distributed network.

This paradigm fits naturally into simulations of agents in a simulated spatial environment. However, by thinking of spacetime as a generalisation of time, we show how this paradigm is also useful even for simulations of agents that aren't initially considered in spatial terms.

## A simple example

To show how this works in practice, let's build a simple simulation of two agents that send messages back and forth between eachother.

First we choose a class that defines what kind of simulation we want to do. Let's start with a simple forward simulation, so we choose the `ForwardSimulation` class. This class takes two templates: a spacetime for the agents to inhabit and a resource that will perform the computations. Let's choose a Minkowski spacetime with three spatial dimensions and a time dimension (this describes the normal physical space we're all familiar with, but without gravitation) and a thread pool with two threads to do the computation. We'll define a type `MySimulation` for convenience:
```
typedef ForwardSimulation<Minkowski<4>, ThreadPool<2>>      MySimulation;
```

Next we make a class to define our agent's behaviour. Every agent should derive from `Agent<T,S>` where T is the type of the derived class (in the [Curiously Recurring Template Pattern](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern)) and S defines the simulation type that the agent belongs to:
```
class Ping : public Agent<Ping, MySimulation> {
public:
    ChannelWriter<Ping> channelToOther;

    Ping(const MySimulation::SpaceTime &position) : Agent<Ping,MySimulation>(position) { }

    void ping() {
        std::cout << "Ping from " << position() << std::endl;
        channelToOther.send([](Ping &otherAgent) {
            otherAgent.ping();
        });
    }
};
```
If you already have a class that doesn't inherit from `Agent`, it can be made into an agent using the `AgentWrapper` class.

Since every agent has a position in spacetime, to construct a `Ping` agent we send it a position for construction. Our Ping agent has just one member function which sends a lambda function to the other agent using `channelToOther.send(...)`. The lambda function takes a reference to the other agent as its sole argument. When the lambda reaches the other agent, it will be executed with that agent as its argument, which will cause the other agent to send a lambda back...and so on...

Now all we need to do is initiate the simulation, connect the agents and set things going:
```
    Ping *alice = new Ping({0,0,0,0});
    Ping *bob   = new Ping({0,0,0,1});

    alice->channelToOther = ChannelWriter(*alice, *bob);
    bob->channelToOther   = ChannelWriter(*bob, *alice);

    alice->ping();

    MySimulation::start(100);
```

Here we create two agents, Alice and Bob, at given positions in spacetime (we don't specify velocity, so they both take on the velocity of the default reference frame). Next, we create new channels to connect Alice and Bob using `ChannelWriter(<source>,<target>)` and initiate the exchange by calling the `ping()` method on Alice. This emits an initial lambda, which will be delivered to Bob when we start the simulation. To do that we call `MySimulation::start(...)` with the time (in the laboratory frame) that the simulation should end. Each agent is deleted when it reaches the end of the simulation, so although we don't explicitly see the deletion, there is no memory leakage (the user can define a different behaviour at the boundary by specifying a different boundary type in the simulation).

The whole program can be found in [`main.cpp`](src/main.cpp) in this repository.

## Birth and death of agents

An agent can spawn new agents during a simulation by using the `new` operator in the normal way. A specific position and velocity can be specified, but the position must be in the future light-cone of the creating agent (see later for a definition of light-cone). If we don't specify a position and/or velocity, it will take the same value as the agent performing the creation.

Before a simulation is started, any code is executed on a unique "initialising agent" which resides at the beginning of time, so all positions are in this agent's future light-cone.

An agent can die at any time if it calls its `die()` method. At this point in spacetime all the agent's channels are closed and the agent is deleted.

## Sending channels to other agents

If Bob has a channel to Alice (let's call it `channelToAlice`), and a channel to Carol (`channelToCarol`), he may want to introduce Alice to Carol. To do this Bob needs to get a `RemoteReference` to Alice, which he does by calling `channelToAlice.target()`. He can then send the `RemoteReference` to Carol, who can then use it to create a channel to Alice. For example:
```
channelToCarol.send([remoteReferenceToAlice = channelToAlice.target()](Carol &carol) mutable {
    carol.channelToAlice = ChannelWriter(carol, remoteReferenceToAlice);
    ...
});
``` 

Note that Bob should never send Carol a raw pointer or reference to Alice because a reference at one spacetime location is not valid at any other location (e.g. Bob and Carol may not even reside on the same physical computer). He also shouldn't send a `ChannelWriter` directly, as a channel is between two fixed agents (to prevent this, `ChannelWriter` doesn't have a copy constructor so can't be captured in a `std::function`. To pass a `ChannelWriter` to a method, it must be moved rather than copied).

## Spacetime as a means to distributing multi-agent computation

Any multi-agent computation can be thought of in terms of a set of agents on which events occur. An agent can respond to an event by performing internal computations, creating new agents, destroying existing agents and/or causing more events. Our aim is to perform such a computation efficiently across multiple processors in such a way that the output of the computation is deterministic (i.e. there are no race conditions).

We assume that any two events on the same agent are non-commutative (i.e. executing them in a different order may lead to a different result) and that any two events on different agents are commutative (i.e. we can execute them in either order, and the result is the same - this comes naturally since an event on one agent can't directly affect another agent: it can only cause a separate event on another agent). 

A simple implementation of this is to timestamp every event with a point in simulated time. Agents cause new events by submitting them to a priority queue which executes them in order of simulated-time. Unfortunately this leaves no opportunity for parallelisation as the event currently being executed may cause another event that comes before (and causally affects) the next event on the queue, so we must wait for the current event to finish execution before we start the next event.

One solution to this problem would be to require that an event can only cause another event after a minimum "reaction time". We can then safely parallelise any events whose timestamp falls within a sliding window of duration equal to this minimum reaction time. This is particularly efficient if the agents are timestepping (i.e. their events are synchronised, so can all be executed in parallel). However, in most multi-agent computations, an event on one agent can only cause events on a small proportion of other agents in the simulation. The sliding-window algorithm doesn't account for this, so it misses opportunities for parallelisation that arise from the agent's limited opportunities for interaction (even in the timestepping case, as we otherwise need to wait for the most computationally intensive agent at every timestep).

We can solve this by keeping track of which agents can cause events on which other agents at any point in a computation. This can be done by allowing an agent to cause an event on another agent only if there is a communication channel between them. Given this, we can do away with the central event queue completely and have separate event queues on each agent. Every agent, $A$, processes their own events in order of simulated time until there is an agent that has a channel to $A$ and is lagging behind $A$ by more than the reaction time. If we implement the opening and closing of channels as events that must be sent over existing channels then this paradigm can also deal with runtime changes to the channels. An agent can use any convenient local processor to compute its events and should release the processor when it reaches a point where it "blocks" on another agent. Since an agent can only block on an agent that is at a simulated time earlier than it, there must always be at least one agent that isn't blocked and we can never reach a "gridlock" situation where there is a loop of agents each blocking on the other.

### Spacetime

The above algorithm goes a long way to our goal but we may get situations when one agent is blocking on another because it may potentially cause an event, but the other agent doesn't really care if the processing of the event is delayed for a while in simulated time, so there's no need to block. We generalise the above algorithm by noticing that by timestamping events we define a complete ordering on events, but this is a stronger constraint than is required to define a deterministic, multi-agent computation. All we really need to define is a complete ordering of events *on each individual agent*. Between agents, we only need to ensure that there can never be "gridlock" (i.e. a cycle of events each causing the other) so that the computation can always proceed. This is garanteed if we ensure that one event can only cause another event that has a timestamp greater than its own. However, these requirements can be satisfied with a "timestamp" that is taken from a set that has only a partial ordering, we'll call such a set a "spacetime". So, up until now, we've been stamping events with a 1-dimensional spacetime (i.e. simulated time) and during a calcuation every agent has been at some point in simulated time, but now we allow time to be any partially ordered domain, for example, we could use a 2-dimensional spacetime where each event is "timestamped" with two numbers.

In the 1-dimensional case, an agent simply moved forward in time. In the more general case, an agent follows a "trajectory" which is a subset of spacetime points which happen to be completely ordered. To ensure causal ordering, we specify that an event at $x$ (i.e. with timestap $x$) can only cause an event on agent $A$ at the eariest point on $A$s trajectory that is after $x$.

Any vector space equipped with an inner product is a spacetime. Given a reference vector $r$ (which defines an "arrow of time") the inner product defines a partial ordering on points in the vector space so that $y > x$ iff $(y-x).(y-x) \ge 0$ and $r.(y-x) > 0$. Given a point $x$, we call the set of points $\left\{y : y > x\right\}$ the *future light cone* of $x$.

For any point $x$ on an agent's trajectory we define the 4-velocity to be the unit vector that is tangent to the trajectory at $x$ and in $x$'s future light cone. An agent's 4-velocity remains constant until it experiences an event that changes it, so between events, an agent follows a trajectory through spacetime defined by
$$x(t) = x_0 + vt$$
where $x_0$ is the position of the previous event and $t$ is the agent's local time. Given any two local times $t_1 > t_0$, an agent's trajectory should satisfy $x(t_1) > x(t_0)$.

For example, consider Minkowski space whose inner product is $x.y = x_0y_0 - (x_1y_1 + x_2y_2 + x_3y_3)$. Note the change in sign between this and the Euclidian inner product, $x.y = x_0y_0 + x_1y_1 + x_2y_2 + x_3y_3$. This small difference has a profound effect; the future light cone of the Minkowski inner product describes a sphere centered on the origin whose radius grows linearly with time. This is very different from the Euclidean light cone which is just a point at the origin which appears and disappears at time 0. So, when an agent emits a lambda function in Minkowski space, it creates a sphere at the point of emission which grows until it touches the target agent and is absorbed. In this way lambda functions transmit information between agents: although the distance in spacetime between the emission and absorbtion points is zero, as defined by the inner product, these points may be separated in space and time.

### Implementation

To allow the runtime creation of new channels while ensuring that lambdas can never be sent into an agent's past we distinguish three events: the creation of the channel, the attachment of the channel to the sender and the attachment of the channel to the receiver. A channel is like an arrow through spacetime, so is associated with two trajectories: that of its source-end and that of its destination-end. When a channel is created, its source and destination ends are both at the point of creation. An unattached end of a channel can be sent down another channel (i.e. captured in a lambda), but its position is taken to be its creation point until it is attached to an agent. Once attached to an agent, a channel's end point trajectory follows that of the agent until the agent closes the channel.

In order to improve efficiency even more, we note that the order of emission of lambdas down a given channel is the same as the order of absorbtion at the target, so each channel can be implemented as a deque, and the target can identify the event with the earlist time to absorbtion by looking only at the front lambda on the deque of each incomming channel.


