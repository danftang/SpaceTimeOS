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

Any multi-agent computation can be thought of in terms of a set of agents on which lambda functions are executed (we'll call the execution of a lambda function on an agent an "event"). A lambda function can change an agent's state, destroy it, create new agents and/or cause other events on itself or other agents. So, an initial set of agents and lambda functions defines a computation consisting of a cascade of events. Our aim is to perform such a computation efficiently across multiple processors in such a way that the output of the computation is deterministic and independent of the number and nature of the processors (i.e. there are no race conditions and any randomness is taken from pseudo-random number generators with well defined seeds).

We assume that any two lambdas on the same agent are non-commutative (i.e. we cannot guarantee that executing them in reverse order will lead to the same result) and that any two lambdas on different agents are commutative (i.e. we know a-priori that we can execute them in either order, and the result will be the same - this comes naturally since a lambda on one agent can't directly change another agent's state: it can only cause a separate event on another agent).

A simple implementation of this is to timestamp every lambda with a point in simulated time. A lambda can cause a new event by submitting a lambda to a priority queue along with a target agent and a timestamp greater than its own. The priority queue then executes lambdas in timestamp order. Unfortunately this leaves no opportunity for parallelisation as the current event may cause another event that is timestamped before (and causally affects) the next event currently on the queue, so we must wait for the current event to finish execution before we start the next.

A simple solution to this problem would be to require that when a lambda submits another lambda, the new lambda's timestamp must be at least its own plus a minimum "reaction time". We can then safely parallelise any lambdas on the queue whose timestamp falls within a sliding window of duration equal to this minimum reaction time. This is particularly efficient if the agents are timestepping (i.e. their events are synchronised, so can all be executed in parallel). However, in most multi-agent computations, an event on one agent can only cause events on a small proportion of other agents in the simulation. The sliding-window algorithm doesn't account for this, so it misses opportunities for parallelisation that arise from the agent's limited opportunities for interaction (this is true even in the timestepping case, where all processors must otherwise wait for the most computationally intensive agent's timestep to complete at every timestep).

We can solve this by keeping track of which agents can cause events on which other agents at any point in a computation. This can be done by requiring agents to open a communication channel with another agent before it can pass lambdas to it. Given this, we can do away with the central event queue completely and have separate event queues on each agent. Every agent, $A$, can processes their own events in order of simulated time until one of its incoming channels is from an agent that is lagging behind it by more than the reaction time. By implementing the opening and closing of channels as events that must be sent over existing channels then this paradigm can deal with runtime changes to the channels. An agent can use any convenient local processor to compute its events and should release the processor when it reaches a point where it "blocks" on another agent. Since an agent can only block on an agent that is at a simulated time earlier than it, there must always be at least one agent that isn't blocked and we can never reach a "gridlock" situation where there is a loop of agents each blocking on the other.

## Spacetime as a means to distributing multi-agent computation

The above algorithm goes a long way to our goal but we can go even further by allowing agents to dynamically alter the relationship between a calling-agent's sending of a lambda and the receiving-agent's execution of it. However, we have to do this carefully to ensure we can still easily calculate when an agent can move forward without fear of blocking. To understand how we do this we first notice that by timestamping events we define a complete ordering on events, but this is a stronger constraint than is required to define a deterministic, multi-agent computation. All we really need is a complete ordering of events *on each individual agent*. Between agents, we only need to ensure that there can never be "gridlock" (i.e. a cycle of events each causing the other) so that the computation can always proceed. This is guaranteed if we ensure that one event can only cause another event that has a timestamp greater than its own. However, these requirements can be satisfied with a "timestamp" taken from a set that is only [partially ordered](https://en.wikipedia.org/wiki/Partially_ordered_set). As our paritally ordered replacement for time, we choose an N-dimensional Minkowski space.

We define a partial ordering on a Minkowski space as $Y > X$ if and only if $L.(Y-X) > 0$ and $(Y-X).(Y-X) > 0$ where $L=(1,0,0,0...)$ defines the "arrow of time" and the inner product is defined as
$$
(x_1...x_N).(y_1...y_N) = x_1y_1 - \sum_{d=2}^N x_dy_d
$$
Given a point in spacetime, $X$, we call the set of points $\lbrace Y : Y > X \rbrace$ the *future* of $X$ and the set $\lbrace Y : Y < X \rbrace$ the *past* of $X$. Notice that there are pairs of distinct points, $X$ $Y$, such that $Y$ is not in $X$'s future or past (i.e. neither $X>Y$ nor $Y>X$, for example $(0,0,0,...)$ and $(1,2,0,...)$). So the ordering of points in Minkowski space is only a partial order. Such pairs of points we say are "space-like separated".

So, if we "timestamp" every event with a point in spacetime, rather than just a time, then a computation is well defined as long as we ensure that:

 1) Agent ordering: any two events on the same agent must be ordered (i.e. one must be in the future of the other, no pair is space-like separated)
 2) Causal orderint: an event at point $X$ may only cause an event at point $Y$ if $Y>X$ (i.e. if $Y$ is in $X$'s future).

To ensure agent ordering we define the relationship between consecutive events on the same agent as
$$
X_1 = X_0 + V\Delta t
$$
where $X_0$ is the position of the previous event, $X_1$ is the position of the next event, $V$ is a vector (which we call the agent's 4-velocity) that satisfies $V.V = 1$ and $V.L > 0$ and $\Delta t>0$ is a scalar giving the local time that the agent experiences between events.

The 1-dimensional case just reduces to normal time. There is only one possible velocity that satisfies $V.V=1$ and $V.L>0$: forward in time. In the multi-dimensional case, an agent can choose between different velocities after each event, this gives the agent freedom to dynamically modify its "distance" to other agents, and so the ordering of events caused on it and events that it causes.

To ensure causal ordering, if an event at point $X_a$ causes an event at point $X_b$ then we define the distance between those points $\Delta X = X_b - X_a$ as
$$
(\Delta X - R).(\Delta X - R) - c = 0
$$
and
$$
\Delta X . L >= 0
$$
for some constant vector $R >= 0$ and constant scalar $c>=0$. If multiple events occur at the same point in spacetime on the same agent, then the events are ordered pseudo-randomly.

We can visualise this by thinking of an agent "emitting" a lambda function from its current position, $X_a$, creating a field in spacetime of the form $F(X) = (X - X_a - R).(X - X_a - R) - c$. As the target agent moves along its trajectory $X(t) = X_0 + Vt$ it "absorbs" (executes) the lambda at the point that its trajectory intersects the zero crossing of $F$. We can now see why Minkowski space is a good fit for our needs: when an agent emits a lambda function in Minkowski space, the zero crossing of $F$ describes a sphere centred on the point of emission which grows until it touches the target agent and is absorbed. In this way the transmission of information around spaceimte has a finite simulated speed, spacetime is local, and this is what aids us in parallelising the simulation.

To actually perform the calculation, an agent A must block on agent B if a lambda emitted by B would be absorbed at A's current position.

In full generality, agent B may also move to another point, Y, and then emit a lambda, so there are three fields to consider: the "movement field", $M(X) = X.X$, describing all points an agent at the origin may move to, the "lambda field", $F(X) = (X-R).(X-R) - c$, describing the points a labda emitted at the origin may be absorbed and a "blocking field", $B$, that describes the points that an agent at the origin may deliver a lambda to by some combination of movement and emission. So, for an agent at the origin, $B$ should be dfined so that $B(X)>0$ if and only if there exists a $Y$ such that $M(Y)>=0$ and $F(X-Y)>0$.
So, if there exists a $Y$ and $X$ such that
$$
Y.Y >= 0
$$
and
$$
(X-Y-R).(X-Y-R) - c > 0
$$
adding these gives
$$
(X-Y-R).(X-Y-R) + Y.Y - c > 0
$$
but in the case of Minkowski space if can be shown that for two timelike vectors $A$ and $B$, $(A+B).(A+B) > A.A + B+B$. Since $c>0$, $X-Y-R$ must be timelike so
$$
(X-R).(X-R) - c > 0
$$
Conversely if $(X-R).(X-R) - c > 0$ then there exists a $Y$, namely the origin, such that $Y.Y >=0$ and $(X-Y-R).(X-Y-R) - c > 0$. So, $B(X) = F(X)$.

## Boundary conditions

We define the boundary conditions of a simulation as a set of linear fields of the form $B_n(X) = R_n.X + c+n$ for a set of constant vectors $R_n$ and constants $c_n$. This describes a polyhedron, inside which the computation will play out. Each constraint is associated with a lambda function. If an agent intersects a boundary (i.e. its trajectory touches a zero of the boundary field) then the boudary's lambda function is executaed on that agent. Boundary fields are fixed and are not absorbed.

## Implementation

### Channel creation
To allow the runtime creation of new channels while ensuring that lambdas can never be sent into an agent's past we distinguish three events: the creation of the channel, the attachment of the channel to the sender and the attachment of the channel to the receiver. A channel is like an arrow through spacetime, so is associated with two trajectories: that of its source-end and that of its destination-end. When a channel is created, its source and destination ends are both at the point of creation. An unattached end of a channel can be sent down another channel (i.e. captured in a lambda), but its position is taken to be its creation point until it is attached to an agent. Once attached to an agent, a channel's end point trajectory follows that of the agent until the agent closes the channel.

In order to improve efficiency even more, we note that the order of emission of lambdas down a given channel is the same as the order of absorbtion at the target, so each channel can be implemented as a deque, and the target can identify the event with the earlist time to absorbtion by looking only at the front lambda on the deque of each incomming channel.


