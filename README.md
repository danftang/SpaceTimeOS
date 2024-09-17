# SpaceTimeOS

SpaceTimeOS presents a new paradigm for distributed, multi-agent computation.

In this paradigm, agents are ordinary C++ objects embued with a position and velocity in a shared space. Agents can communicate with each other by opening agent-to-agent communication channels, down which they can send lambda functions (code and data) to be executed on the remote agent. The lambda functions move at a finite speed so agents that are close to eachother can communicate more quickly than agents further away. This allows efficient, automated assignment of computational events to processing nodes, thus allowing simulations to be distributed across all cores of a single processor or many processors on a distributed network.

## A simple example

Let's start with a simple simulation of two agents that send messages back and forth between eachother.

First we define what kind of simulation we want to do. In this case we're doing a simple forward simulation, so we choose `ForwardSimulation`. This type takes two templete classes: a spacetime for the agents to inhabit and a resource that will perform the computations. Here we choose a Minkowski spacetime with three spatial dimensions and a time dimension, this describes the normal physical space we're all familiar with (though more on this later). To do the computation we choose a thread pool with two threads.
```
typedef ForwardSimulation<Minkowski<4>, ThreadPool<2>>      MySimulation;
```

Next we define a class that will define our agents. Every agent should derive from Agent<T,S> where T is the type of the derived class (in the `curiously recurring template pattern') and S defines the simulation type that the agent belongs to:
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
On construction, our `Ping` agent takes an object that provides a position in spacetime at which it should be created, this can be another agent or a point on the boundary. Our agent can send a lambda function to the other agent using `channelToOther.send(...)`. The lambda function should take a reference to the other agent as its sole argument. When the lambda reaches the other agent, it will be executed with that agent as its argument. In our case it will execute a `ping()` which will send a lambda back...

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

Next, we create channels to connect Alice and Bob using `ChannelWriter(<source>,<target>)` and initiate the exchange by calling the `ping()` method on Alice. This sends the initial lambda to Bob, but as yet we haven't actually started the computation, so it won't be delivered yet. To start the computation, we call `MySimulation::start(...)` with the time (in the laboratory frame) that the simulation should end. Each agent is deleted when it reaches the end of the simulation, so although we don't explicitly see the deletion, there is no memory leakage (the user can define a vdifferent behaviour at the boundary by specifying a different boundary type in the simulation).

## Birth and death of agents

An agent can spawn new agents during a simulation by using the `new` operator in the normal way. The underlying `Agent` object can be supplied with a reference to a parent Agent which will construct the new agent at the same spacetime position and velocity as its parent. Notice that an agent can't spawn a new agent at any arbitrary position in spacetime, as this may contravene causality.

An agent can die at any time by calling its `die()` method. At this point in spacetime all the agent's channels are closed and the agent is deleted.

## Sending channels to other agents

If Bob has a channel to Alice (let's call it `channelToAlice`), and a channel to Carol (called `channelToCarol`), he may want to introduce Alice to Carol. To do this he should do the following:
```
channelToCarol.send([aliceChannel = SendableWriter(channelToAlice)](Carol &carol) mutable {
    ChannelWriter channelToAlice = aliceChannel.attachSource(carol);
    ...
});
```
Since a channel is between two named agents, Bob can't just send Carol a copy of his `channelToAlice` (note that `ChannelWriter` has no copy constructor in order to ensure this doesn't happen, we can only move `ChannelWriter`s). Instead Bob constructs a `SendableWriter` from his `channelToAlice`. This creates a new channel that is attached to Alice as its target but isn't yet attached to a source. When Carol receives this, she attaches herself using `attachSource`, which constructs a regular `ChannelWriter` which Carol can then use to communicate with Alice. 

Note that Bob should never send Carol a raw pointer or reference to Alice because a reference at Bob's location will not generally be valid at Carol's location (e.g. Bob and Carol may not reside on the same physical computer, so may not share any memory).

## Spacetime as a paradigm for distributed computation

In the example above we used a a 4-dimensional Minkowski spacetime, which describes real-world spacetime in the absence of gravitation. This is a natural choice when the agents represent objects in the real world, but users can create their own spacetimes and this allows enough flexibility to deal with any type of distributed computation. We'll now show how spacetime can be used as a conceptual tool to define multi-agent computation.

We define a spacetime to be a vector space equipped with a distance measure, $|.|$. If an agent is at spacetime point $p$ with 4-velocity $v$ then it will follow trajectory $p(t) = p + vt$ until it absorbs a lambda function that changes its velocity. By definition, 4-velocity, $v$, is a unit vector so $|v| = 1$ and $t$ measures the time experienced by a clock moving with the agent.

If an agent emits a lambda function at point $q$, then the target agent receives the lambda function at the earliest point on its trajectory where the distance between $q$ and the agent goes from imaginary to real. When an agent receives a lambda function, it is absorbed by the agent and, after a fixed restitution time, the agent executes the function, sending a reference to itself as argument. A lambda function may modify the target agent's state or velocity or destroy it compleletly, it may create new objects and/or emit new lambda functions. Any new objects / lambda functions are created at the same point in spacetime as the target object. The execution itself takes zero simulated time, we call this an event.

To get a better understanding of this, consider Minkowski space whose distance measure is $\sqrt{t^2 - (x_0^2 + x_1^2 + x_2^2)}$. Note that this is different from the Euclidian distance as it can take on imaginary as well as real values. Note also that there is a manifold of points $q$ such that $|p-q|=0$, this is very different from the Euclidean distance for which the only point with zero distance from $p$ is itself.

An agent that finds itself at spacetime point $p$ with velocity $v$ need only concern itself with the existance of any lambda functions on the manifold of points, $q$, such that $|p-q|=0$ and $|p+v-q|>0$. We call this the "past light cone" of $p$.

Since any computation can be expressed as a partial ordering of atomic computations then it turns out that any computation can be expressed as agents moving aroung an appropriate space. [TODO: prove this]


