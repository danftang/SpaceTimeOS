# SpaceTimeOS

SpaceTimeOS presents a new paradigm for distributed, multi-agent computation.

In this paradigm, agents are ordinary C++ objects embued with a position and velocity in a shared space. Agents can communicate with each other by opening agent-to-agent communication channels, down which they can send lambda functions (code and data) to be executed on the remote agent. The lambda functions move at a finite speed so agents that are close to eachother can communicate more quickly than agents further away, this allows efficient, automated assignment of computational events to processing nodes, thus allowing simulations to make use of all cores of a single processor or many processors on a distributed network.

## A simple example

To make this more concrete, let's have a look at how we would set up a simple simulation of two agents that send messages back and forth between eachother.

First we define what kind of simulation we want to do. In this case we're doing a simple forward simulation, so we choose `ForwardSimulation`. This type takes two templetes, the first defines what kind of space the agents will inhabit, we choose a Minkowski space-time with three spatial dimensions and a time dimension. This describes the normal physical space we're all familiar with (though more on this later). The second template defines what we want to use to do the computation, here we choose a thread pool with two threads.
```
typedef ForwardSimulation<Minkowski<2>, ThreadPool<2>>      MySimulation;
```

Next we define an agent class that derives from Agent<T,S> where T is the type of the derived class (in the `curiously recurring design pattern') and S defines the simulation type that the agent belongs to.
```
class Ping : public Agent<Ping, MySimulation> {
public:
    ChannelWriter<Ping> other;

    Ping() : Agent<Ping,MySimulation>(Minkowski<2>(), Minkowski<2>(1)) {}

    void ping() {
        other.send([](Ping &otherAgent) {
            otherAgent.ping();
        });
    }
};
```
Each agent has an `other` channel, through which it can communicate with the other agent by using the `other.send(message)` method. The message is a lambda function that takes a reference to the other agent as its argument. When the function reaches the other agent, it will be executed with this agent as its argument.

Now all we need to do is initiate the simulation and set it going.
```
    Ping alice;
    Ping bob;

    alice.other = makeChannel(alice, bob);
    bob.other = makeChannel(bob, alice);

    alice.ping();

    MySimulation::setMaxTime(200);

    alice.submit();
    bob.submit();

    MySimulation::stop();

```
Here we create two agents, Alice and Bob, then connect them together using the `makeChannel(source,target)` helper function. This returns a `ChannelWriter` that can be used by `source` to send messages.

Finally, we initiate the communication by calling the `ping()` method on Alice. Notice that Bob can never directly get a smart pointer to Alice, he can only send lambda functions to Alice, which Alice will then execute on herself. This ensures that computation is localised.

We then set the maximum time we want to simulate until (in the laboratory frame of reference) using `MySimulation::setMaxTime(time)`, finally we start the simulation by getting Alice and Bob to submit themselves for computation.

To end the computation we call `MySimulation::stop()`, this ensures that all threads are ended.

## 


## Spacetime as a paradigm for distributed computation

In the simple example above we used a Minkowski spacetime. This is a natural choice when the agents represent objects in the real world, but users can create their own spacetimes and this allows a lot of flexibility to deal with any type of distributed simulation. We'll now show how spacetime can be used as an integral part of a definition of any multi-agent computation.

We define a spacetime as a vector space equipped with a `distance' function, |V|, which returns a scalar. To qualify as a vector space we should be able to add and subtract any two vectors to get another vector, and multiply a vector by a scalar to get another vector.

A computation begins with a set of *objects* and *lambda functions*. All objects and lambda functions have positions in spacetime. Objects move with given velocities whose speed must be below some global maximum which we call light-speed. Lambda functions travel at light-speed and radiate out in all directions from their point of creation (a bit like the ripples on the surface of a pond after a stone has been thrown in). Each lambda function takes a single argument and has at most one target object.

When a lambda function reaches its target object, it is absorbed by the target and executed at that point in spacetime with the target object as argument to the function. The result of execution is the modification or destruction of the target object, the creation of new objects and/or the emission of new lambda functions. Any new objects / lambda functions are created at the same point in spacetime as the target object. We call this an event.

So, at any point in the computation we have a set of objects and a set of lambda functions, all moving around a shared spacetime.