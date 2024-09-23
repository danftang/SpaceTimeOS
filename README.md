# SpaceTimeOS

SpaceTimeOS is a new paradigm for distributed, multi-agent computation.

In this paradigm, agents are ordinary C++ objects embued with a position and velocity in a shared space. Agents communicate with each other by sending lambda functions (code and data) between themselves. The lambda functions move at a finite speed through the simulated space so agents that are close to eachother can communicate more quickly than agents further away. This allows efficient, automated assignment of computational events to processing nodes, thus allowing simulations to be distributed across all cores of a single processor or many processors on a distributed network. 

## A simple example

To show how this works in practice, let's build a simple simulation of two agents that send messages back and forth between eachother.

First we choose a class that defines what kind of simulation we want to do. In this case we want a simple forward simulation, so we choose the `ForwardSimulation` class. This class takes two templete classes: a spacetime for the agents to inhabit and a resource that will perform the computations. As a spacetime we choose a Minkowski space with three spatial dimensions and a time dimension (this describes the normal physical space we're all familiar with, but without gravitation). To do the computation we choose a thread pool with two threads.
```
typedef ForwardSimulation<Minkowski<4>, ThreadPool<2>>      MySimulation;
```

Next we define a class that will define our agents. Every agent should derive from Agent<T,S> where T is the type of the derived class (in the [Curiously Recurring Template Pattern](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern)) and S defines the simulation type that the agent belongs to:
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
(a class that doesn't inherit from Agent can be made into an agent using the `AgentWrapper` class).

On construction, our `Ping` agent takes an object that provides a position in spacetime to create the agent, this can be from another agent or a point on the simulation's boundary. Our Ping agent can send a lambda function to the other agent using `channelToOther.send(...)`. The lambda function should take a reference to the other agent as its sole argument. When the lambda reaches the other agent, it will be executed with that agent as its argument. In our case it will execute a `ping()` which will send a lambda back...and so on...

Now all we need to do is initiate the simulation and set it going.
```
    Ping *alice = new Ping({0,0,0,0});
    Ping *bob   = new Ping({0,0,0,1});

    alice->channelToOther = ChannelWriter(*alice, *bob);
    bob->channelToOther   = ChannelWriter(*bob, *alice);

    alice->ping();

    MySimulation::start(100);
```
Here we create two agents, Alice and Bob, at given positions in spacetime. 

Next, we create new channels to connect Alice and Bob using `ChannelWriter(<source>,<target>)` and initiate the exchange by calling the `ping()` method on Alice. This emits an initial lambda, which will be delivered to Bob when we start the computation. To start the computation, we call `MySimulation::start(...)` with the time (in the laboratory frame) that the simulation should end. Each agent is deleted when it reaches the end of the simulation, so although we don't explicitly see the deletion, there is no memory leakage (the user can define a different behaviour at the boundary by specifying a different boundary type in the simulation).

The whole program can be found in [`main.cpp`](src/main.cpp) in this repository.

## Birth and death of agents

An agent can spawn new agents during a simulation by using the `new` operator in the normal way. A specific position and velocity can be specified, but the position must be in the future light-cone of the creating agent. If we don't specify a position and/or velocity, it will take the same value as the agent performing the creation.

Before a simulation is started, any code is executed on a unique "initialising agent" which resides at the beginning of time, so all positions are in this agent's future.

An agent can die at any time by calling its `die()` method. At this point in spacetime all the agent's channels are closed and the agent is deleted.

## Sending channels to other agents

If Bob has a channel to Alice (let's call it `channelToAlice`), and a channel to Carol (called `channelToCarol`), he may want to introduce Alice to Carol. To do this Bob needs to get a `RemoteReference` to Alice, which he does by calling `channelToAlice.target()`. He can then send the remote reference to Carol, who can then use it to create a channel to Alice. The code to do that looks like this:
```
channelToCarol.send([remoteReferenceToAlice = channelToAlice.target()](Carol &carol) mutable {
    ChannelWriter channelToAlice(carol, remoteReferenceToAlice);
    ...
});
``` 

Note that Bob should never send Carol a raw pointer or reference to Alice because a reference at one spacetime location is not valid at any other location (e.g. Bob and Carol may not even reside on the same physical computer). He also shouldn't send a `ChannelWriter` directly, as a channel is between two fixed agents (to prevent this, `ChannelWriter` doesn't have a copy constructor so can't be captured in a `std::function`. To send a `ChannelWriter` around locally, it must be moved rather than copied).

## Spacetime as a means to distributing multi-agent computation

Any multi-agent computation can be thought of in terms of a set of agents on which events occur. An agent can respond to an event by performing internal computations, creating new agents, destroying existing agents and/or causing more events. Our aim is to perform such a computation across distributed processors in such a way that the output of the computation is deterministic (i.e. there are no race conditions).

We assume that any two events on the same agent are non-commutative (i.e. executing them in a different order leads to a different result) and that any two events on different agents are commutative (i.e. we can execute them in either order, and the result is the same). Without loss of generality, we assume an event is defined as the execution of a lambda function that takes a mutable reference to the agent as parameter, and so an agent emits a lambda funtion which is then executed on another agent.

A simple implementation of this is to timestamp every event with a point in simulated time. Agents cause new events by submitting them to a priority queue which executes them in order of simulated-time. Unfortunately this leaves no opportunity for parallelisation as the event currently being executed may cause another event that comes before (and causally affects) the next event on the queue, so we must wait for the current event to finish execution before we know for sure what the next event will be.

One solution to this problem would be to require that agents have a minimum "reaction time" between the time an agent submits an event and the time it is executed. We can then safely parallelise any events whose timestamp falls within a sliding window of duration equal to the minimum reaction time. This is particularly efficient if the agents are timestepping (i.e. their events are synchronised, so can all be executed in parallel). However, in most multi-agent computations, an event on one agent can only cause events on a small proportion of other agents in the simulation. The sliding-window algorithm doesn't account for this, so it misses opportunities for parallelisation that arise from the agent's limited opportunities for interaction.

Notice that by timestamping events we impose a complete ordering on events, but this is a stronger constraint than is required to define a deterministic, multi-agent computation. All we really need is to define a complete ordering of events *on each individual agent*. Events on different agents only need to be ordered when one event conditionally causes another. So, we'd like to replace the priority queue of timestamped events with a distributed algorithm that imposes a complete ordering of events on each individual agent (to ensure a deterministic result), and a causal ordering so that if one event causes another, then the cause comes first (to allow the computation to be performed by a physical computer). SpacetimeOS achieves this by replacing the timestamp of an event with a point in spacetime.

We define a spacetime to be a vector space equipped with an inner product. Every vector, $x$, has a "length" which we define as $\sqrt{x.x}$ and so every pair of vectors $x$, $y$ have a distance bwtween them which we define as $\sqrt{(x-y).(x-y)}$. Every agent has a 4-velocity $v(t)$ which depends on its local time, $t$ (the time shown on a clock carried by the agent). An agent follows a trajectory through spacetime defined by 
$$x(t) = x_0 + \int_0^t v(t') dt$$
where $x_0$ is it's spacetime position when local-time $t=0$. An agent's 4-velocity will remain constant until it experiences an event that changes it. By definition, a 4-velocity has unit length so $v.v = 1$ (note that this does *not* mean that all agents move at the same speed: 4-velocity is not the same as 3-velocity). Given a vector $x$ and a velocity $v$ we define the *future light cone* of $x$ to be the set of points $y$ such that $(y-x).(y-x) /ge 0$ and $v.(y-x) > 0$. We arbitrarily choose a reference velocity $v_{r}$ which we use to define the future light cone and require that every agent's velocity satisfies $v.v_r > 0$. This ensures that all agents have the same "arrow of time" and agree on their future light cone.

If an agent "emits" a lambda at point $x$ then it is "absorbed" by the target agent at the earliest point on the its trajectory that is in the future light cone of $x$. When an agent absorbs a lambda function, the agent executes it with a reference to itself as argument. A lambda function may modify the target agent's state or velocity or destroy it compleletly, it may create new objects and/or emit new lambda functions. Any changes occur after a fixed reaction time following absorbtion. Any lambda functions are created at the point in spacetime of the target agent after this reaction time, and new objects must be created in (or on) the future light cone of the creating agent.

For example, consider Minkowski space whose inner product is $x.y = x_0y_0 - (x_1y_1 + x_2y_2 + x_3y_3)$. Note the difference between this and the Euclidian inner product, $x.y = x_0y_0 + x_1y_1 + x_2y_2 + x_3y_3$. Importantly, with the Minkowski inner product there is a manifold of points $x$ such that $x.x=0$, this is very different from the Euclidean case for which only the origin has this property. This is the way lambda functions transmit information from one place to another: although the distance between the emission and absorbtion points is zero, by definition, there is a whole manifold of possible absorbtion points that have zero distance from a given emission point.

An agent must open a channel to another agent before sending it a lambda. The opening of a channel consists of three events: the creation of the channel, the attachment of the channel to the sender and the attachment of the channel to the receiver, each event occuring at a point in spacetime. An agent cannot receive lambdas through a channel until after it has been attached. Attachment events can themselves be sent down existing channels. Newly created agents can be initialised with channel attachment events. Once attached, either agent can close the channel, this too is an event at a point on an agent's trajectory.

This defines our required partial ordering on events: events on the same agent are completely ordered by the agent's local time of receipt, while causal ordering is ensured by defining the emission of a lambda as before its absorbtion. 

With this, the central priority queue of events can be replaced with separate queues on each individual agent. Each lambda on an agent's queue has a "local time to absorbtion" which is defined as the earliest time $t$ such that $x + vt$ is in the lambda's future light cone. The agent should choose the lambda with the earliest local time to absorbtion, move to the point of absorbtion, and execute the lambda (by submitting the event to any convenient, local processing unit). This absorbtion proceess can proceed until the agent reaches a point such that there exists an attached channel from agent $A$ to itself such that this agent is within $A$'s future light cone by a local time equal to its reaction time. At this point the agent must pause itself because it is possible that $A$ may emit a lambda which will end up on this agent's queue.

A little thought shows that it is impossible for there to be a loop of agents each waiting on the other, so there must always be at least one agent that isn't waiting on any other agent so the computation is guaranteed to move forward.
