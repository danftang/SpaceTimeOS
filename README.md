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

Any multi-agent computation can be thought of in terms of a set of agents on which events occur. An agent can respond to an event by performing internal computations, creating new agents, destroying existing agents and/or causing more events.

A simple way to implement this is to require that every event occurs at a particular simulated time. Agents can cause new events by submitting them to a priority queue which executes the events in time order. This is fine if there is only one processor, but it leaves no opportunity for parallelisation as the currently processing event may cause another event that comes before the next event on the queue and may affect any agent, so we must wait for the current event to finish before we can start processing the next event.

The problem stems from the fact that giving a timestamp to events imposes a globally complete ordering on events, but this is a stronger constraint than is required. All we really need is to define a complete ordering of events *on each individual agent*. Events on different agents only need be ordered if one event depends on information computed by the other. So, what we want is some mechanism to replace the priority queue of timestamped events with some distributed mechanism that is guaranteed to impose a partial ordering on events. SpacetimeOS achieves this by replacing the time of an event with a point in spacetime.

We define a spacetime to be a vector space equipped with an inner product (we'll call the square root of the dot product of a vector with itself its length). If an agent is at spacetime point $p$ with 4-velocity $v$ then it will follow trajectory $p(t) = p + vt$, where $t$ is the time experienced by the agent. An agent's 4-velocity will remain constant until it absorbs a lambda function that changes its velocity. By definition, a 4-velocity is a unit vector in the spacetime so $v.v = 1$.

If an agent emits a lambda function at point $q$, then the target agent receives the lambda function at the earliest point, $p$, on its trajectory where $(p-q).(p-q) = 0$ and $v.(p-q) > 0$ (i.e. $p$ is where the trajectory enters the future light-cone of $q$). When an agent receives a lambda function, it is absorbed by the agent and the agent executes the function, sending a reference to itself as argument. A lambda function may modify the target agent's state or velocity or destroy it compleletly, it may create new objects and/or emit new lambda functions. Any changes occur after a fixed reaction time following absorbtion. Any new objects / lambda functions are created at the point in spacetime of target agent after this reaction time.

To get a better understanding of this, consider Minkowski space whose inner product is $x.y = x_0y_0 - (x_1y_1 + x_2y_2 + x_3y_3)$. Note the difference between this and the Euclidian inner product, $x.y = x_0y_0 + x_1y_1 + x_2y_2 + x_3y_3$. Importantly, with the Minkowski inner product there is a manifold of points $x$ such that $x.x=0$, this is very different from the Euclidean case for which only the origin has this property. This is the way lambda functions transmit information from one place to another: although the distance between the emission and absorbtion points is zero, by definition, there is a whole manifold of possible absorbtion points for any given emission point.

A little thought shows that this definition imposes an ordering on events that have positive squared distance between eachother, but not on events that have negative squared distance. This gives us the partial ordering we're after.

So, an agent that finds itself at spacetime point $p$ with velocity $v$ need only concern itself with the existance of any lambda functions on the manifold of points $\left\{q : (p-q).(p-q)=0 \wedge v.(p-q)>0 \right\}$. We call this the "past light cone" of $p$.

TODO: finish this...

