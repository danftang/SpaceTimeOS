# SpaceTimeOS

SpaceTimeOS presents a new paradigm for distributed, multi-agent computation.

In this paradigm, agents are ordinary C++ objects embued with a position and velocity in a shared space. Agents can communicate with each other by opening agent-to-agent communication channels, down which they can send lambda functions (code and data) to be executed on the remote agent. The lambda functions move at a finite speed so agents that are close to eachother can communicate more quickly than agents further away. This allows efficient, automated assignment of computational events to processing nodes, thus allowing simulations to make use of all cores of a single processor or many processors on a distributed network.

## A simple example

Let's start with a simple simulation of two agents that send messages back and forth between eachother.

First we define what kind of simulation we want to do. In this case we're doing a simple forward simulation, so we choose `ForwardSimulation`. This type takes two templete classes: a spacetime for the agents to inhabit and a resource to do computations on. Here we choose a Minkowski spacetime with three spatial dimensions and a time dimension, this describes the normal physical space we're all familiar with (though more on this later). To do the computation we choose a thread pool with two threads.
```
typedef ForwardSimulation<Minkowski<4>, ThreadPool<2>>      MySimulation;
```

Next we define an agent class that derives from Agent<T,S> where T is the type of the derived class (in the `curiously recurring design pattern') and S defines the simulation type that the agent belongs to.
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
Each agent has an `channelToOther` channel, through which it can communicate with the other agent by using the `send(...)` method. The message is a lambda function that takes a reference to the other agent as its argument. When the function reaches the other agent, it will be executed with that agent as its argument.

Now all we need to do is initiate the simulation and set it going.
```
    Ping *alice = new Ping(BoundaryPosition({0,0,0}));
    Ping *bob   = new Ping(BoundaryPosition({0,0,1}));

    alice->channelToOther = ChannelWriter(*alice, *bob);
    bob->channelToOther   = ChannelWriter(*bob, *alice);

    alice->ping();

    MySimulation::start(100);
```
Here we create two agents, Alice and Bob. Whenever we create a new agent we need to specify a point in spacetime that the creation occurs. Since we're defining the initial state of the simulation, any new agents should be on the boundary (i.e. at the starting time in the laboratory frame), so we supply a 3D spatial position, and tag it as a coordinate on the simulation's boundary.

Next, we create channels between Alice and Bob using `ChannelWriter(<source>,<target>)`.

The exchange is initialised by calling the `ping()` method on Alice. 

The whole simulation is started by calling `MySimulation::start(...)` with the time (in the laboratory frame) that the simulation should end. Each agent is deleted when it reaches the end of the simulation, so although we don't explicitly see the deletion, there is no memory leakage.

## Birth and death of agents

An agent can spawn new agents during a simulation by using the `new` operator in the normal way. The underlying `Agent` object must be supplied with a reference to a parent Agent, and the new agent will be constructed with the same spacetime position and velocity as its parent. Notice that an agent can't spawn a new agent at any arbitrary position in spacetime, as this may contravene causality.

An agent can die by calling its ``die()`` method. At this point in spacetime all the agent's channels are closed and the agent is deleted.

## Sending channels to other agents

If Bob has a channel to Alice (let's call it `channelToAlice`), and a channel to Carol (called `channelToCarol`), he may want to introduce Carol to Alice. To do this he should do the following:
```
channelToCarol.send([aliceChannel = SendableWriter(channelToAlice)](Carol &carol) mutable {
    ChannelWriter channelToAlice = aliceChannel.attachSource(carol);
    ...
});
```
Since a channel is between two named agents, Bob can't just send Carol a copy of his `channelToAlice`, instead he creates a `SendableWriter` from `channelToAlice` and then sends that to carol. A `SendableWriter` is a channel that is attached to a target but isn't yet attached to a source. When Carol receives the `SendableWriter`, she attaches herself using `attachSource` in order to create into a regular `ChannelWriter` which Carol can then use to communicate with Alice.

Note that Bob should never send Carol a raw pointer or reference to Alice because a reference at Bob's location is not guaranteed to be valid at Carol's location and could contravene causality.

## Spacetime as a paradigm for distributed computation

In the simple example above we used a a 4-dimensional Minkowski spacetime, which describes real-world spacetime in the absence of gravitation. This is a natural choice when the agents represent objects in the real world, but users can create their own spacetimes and this allows a lot of flexibility to deal with any type of distributed simulation. We'll now show how spacetime can be used as an integral part of a definition of any multi-agent computation.

We define a spacetime as a vector space equipped with a partial ordering. To qualify as a vector space we should be able to add any two vectors to get another vector and multiply a vector by a scalar to get another vector. If an agent is at position, $p$, and has a velocity, $v$, then it follows the trajectory $p + v*t$, where t is a scalar describing the time in the agent's reference frame. $v$ must be such that if $t'>t$ then $p + vt' > p + vt$.

If an agent emits a lambda function at point $q$, then the target agent receives the lambda function at the earliest point on its trajectory that is not before q (i.e. the point $p$ such that p>=q and there is no $r$ such that $r\ge q$ and $r \lt p$). When a lambda function reaches its target object, it is absorbed by the target and executed at that point in spacetime with the target object as argument to the function. A lambda functionmay modify the target agent's state or velocity or destroy it compleletly, it may create new objects and/or emit new lambda functions. Any new objects / lambda functions are created at the same point in spacetime as the target object. We call this an event.

A computation begins with a set of *objects* and *lambda functions* on the simulation's boundary, where the objects have velocities. This defines a set of events which constitute the simulation.

Since any computation can be expressed as a partial ordering of atomic computations then it turns out that any computation can be expressed as agents moving aroung an appropriate space. [TODO: prove this]


