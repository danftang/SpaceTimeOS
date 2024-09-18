# SpaceTimeOS

SpaceTimeOS is a new paradigm for distributed, multi-agent computation.

In this paradigm, agents are ordinary C++ objects embued with a position and velocity in a shared space. Agents communicate with each other by sending lambda functions (code and data) to be executed on remote agents. The lambda functions move at a finite speed through the simulated space so agents that are close to eachother can communicate more quickly than agents further away. This allows efficient, automated assignment of computational events to processing nodes, thus allowing simulations to be distributed across all cores of a single processor or many processors on a distributed network.

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

    template<class P>
    Ping(P &&position) : Agent<Ping,MySimulation>(std::forward<P>(position)) {}

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
    Ping *alice = new Ping(BoundaryPosition({0,0,0}));
    Ping *bob   = new Ping(BoundaryPosition({0,0,1}));

    alice->channelToOther = ChannelWriter(*alice, *bob);
    bob->channelToOther   = ChannelWriter(*bob, *alice);

    alice->ping();

    MySimulation::start(100);
```
Here we create two agents, Alice and Bob. Whenever we create a new agent we need to specify a point in spacetime that the creation occurs. Since we're defining the initial state of the simulation, we put the new agents on the boundary (i.e. at the simulation's start time in the laboratory frame), so we supply a 3D spatial position and tag it as a coordinate on the simulation's boundary (note that once the simulation has started, the boundary moves to the end of the simulation, so if an agent creates a new agent on the boundary during a simulation, it will be created at the end of the simulaiton).

Next, we create new channels to connect Alice and Bob using `ChannelWriter(<source>,<target>)` and initiate the exchange by calling the `ping()` method on Alice. This sends the initial lambda to Bob, this will be delivered when we start the computation. To start the computation, we call `MySimulation::start(...)` with the time (in the laboratory frame) that the simulation should end. Each agent is deleted when it reaches the end of the simulation, so although we don't explicitly see the deletion, there is no memory leakage (the user can define a different behaviour at the boundary by specifying a different boundary type in the simulation).

The whole program can be found in [`main.cpp`](src/main.cpp) in this repository.

## Birth and death of agents

An agent can spawn new agents during a simulation by using the `new` operator in the normal way. The underlying `Agent` object must be supplied with a reference to a parent Agent which will construct the new agent at the same spacetime position and velocity as its parent. Notice that an agent can't spawn a new agent at any arbitrary position in spacetime, as this may contravene causality.

An agent can die at any time by calling its `die()` method. At this point in spacetime all the agent's channels are closed and the agent is deleted.

## Sending channels to other agents

If Bob has a channel to Alice (let's call it `channelToAlice`), and a channel to Carol (called `channelToCarol`), he may want to introduce Alice to Carol. To do this Bob needs to get a `RemoteReference` to Alice, which he does by calling `channelToAlice.target()`. He can then send the remote reference to Carol, who can then use it to create a channel to Alice. The code to do that looks like this:
```
channelToCarol.send([remoteReferenceToAlice = channelToAlice.target()](Carol &carol) mutable {
    ChannelWriter channelToAlice(carol, remoteReferenceToAlice);
    ...
});
``` 

Note that Bob should never send Carol a raw pointer or reference to Alice because a reference at Bob's location will not generally be valid at Carol's location (e.g. Bob and Carol may not reside on the same physical computer, so may not share any memory). He also shouldn't send a `ChannelWriter` directly, as a channel is between two fixed agents (to prevent this, `ChannelWriter` doesn't have a copy constructor so can't be captured in a `std::function`. To send a `ChannelWriter` around locally, it must be moved rather than copied).

## Spacetime as a paradigm for distributed computation

In the example above we used a a 4-dimensional Minkowski spacetime, which describes real-world spacetime in the absence of gravitation. This is a natural choice when the agents represent objects in the real world, but users can create their own spacetimes and this allows enough flexibility to deal with any type of distributed computation. We'll now show how spacetime can be used as a conceptual tool to define multi-agent computation.

We define a spacetime to be a vector space equipped with a distance measure, $|.|$. If an agent is at spacetime point $p$ with 4-velocity $v$ then it will follow trajectory $p(t) = p + vt$ until it absorbs a lambda function that changes its velocity. By definition, 4-velocity, $v$, is a unit vector so $|v| = 1$ and $t$ measures the time experienced by a clock moving with the agent.

If an agent emits a lambda function at point $q$, then the target agent receives the lambda function at the earliest point on its trajectory where the distance between $q$ and the agent goes from imaginary to real. When an agent receives a lambda function, it is absorbed by the agent and, after a fixed restitution time, the agent executes the function, sending a reference to itself as argument. A lambda function may modify the target agent's state or velocity or destroy it compleletly, it may create new objects and/or emit new lambda functions. Any new objects / lambda functions are created at the same point in spacetime as the target object. The execution itself takes zero simulated time, we call this an event.

To get a better understanding of this, consider Minkowski space whose distance measure is $\sqrt{t^2 - (x_0^2 + x_1^2 + x_2^2)}$. Note that this is different from the Euclidian distance as it can take on imaginary as well as real values. Note also that there is a manifold of points $q$ such that $|p-q|=0$, this is very different from the Euclidean distance for which the only point with zero distance from $p$ is itself.

An agent that finds itself at spacetime point $p$ with velocity $v$ need only concern itself with the existance of any lambda functions on the manifold of points, $q$, such that $|p-q|=0$ and $|p+v-q|>0$. We call this the "past light cone" of $p$.

Since any computation can be expressed as a partial ordering of atomic computations then it turns out that any computation can be expressed as agents moving aroung an appropriate space. [TODO: prove this]


