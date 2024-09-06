# SpaceTimeOS

SpaceTimeOS presents a new paradigm for distributed, multi-agent computation.

In this paradigm, agents are ordinary C++ objects embued with a position and velocity in a shared space. Agents can communicate with each other by opening agent-to-agent communication channels, down which they can send lambda functions (code and data) to be executed on the remote agent. The lambda functions move at a finite speed so agents that are close to eachother can communicate more quickly than agents further away, this allows efficient, automated assignment of computational events to processing nodes, thus allowing simulations to make use of all cores of a single processor or many processors on a distributed network.

## A simple example

To make this more concrete, let's have a look at how we would set up a simulation of two agents that send messages back and forth between eachother.

First we define what kind of simulation we want to do. In this case we're doing a simple forward simulation, so we choose `ForwardSimulation`. This type takes two templetes, the first defines what kind of space the agents will inhabit, we choose a Minkowski space-time with three spatial dimensions and a time dimension. This describes the normal physical space we're all familiar with (though more on this later). The second template defines what we want to use to do the computation, here we choose a thread pool with two threads.
```
typedef ForwardSimulation<Minkowski<2>, ThreadPool<2>>      MySimulation;
```

Next we define an agent class that sends and receives messages.
```
class Agent {
public:
    ChannelWriter<Agent,MySimulation> other;

    void ping() {
        other.send([](SpaceTimePtr<Ping,MySimulation> pOther) {
            pOther->ping();
        });
    }
};
```
Each agent has an `other` channel, with which it can send a message to the other agent by using the `other.send(message)` method. The agent sends a lambda function that takes a smart pointer to the other agent. When the function reaches the other agent, it will be executed with this pointer pointing to the other agent.

Now all we need to do is initiate the simulation and set it going. To create the two agents we use `MySimulation::spawnAt(position)` which creates a new object at a given position in space-time and returns a smart pointer to the new object. We then connect them together using the `openChannelTo(remoteAgent)` method of the smart pointer.

Finally, we initiate the communication by calling the `ping()` method on Alice. An underlying object can be accessed by deferencing the smart pointer, but notice that Bob can never directly get a smart pointer to Alice, he can only send lambda functions to Alice, which Alice will then execute with a smart pointer to herself.

To begin the simulation, we call `simulateUntil(time)` which simulates the agents forward until a given time (in the laboratory frame of reference). 
```
    SpaceTimePtr alice = MySimulation::spawnAt<Agent>({0.0,-1.0});
    SpaceTimePtr bob   = MySimulation::spawnAt<Agent>({0.0, 1.0});

    alice->other = alice.openChannelTo(bob);
    bob->other   = bob.openChannelTo(alice);

    alice->ping();

    MySimulation::simulateUntil(100);
```

## Spacetime as part of a paradigm for computation

In the simple example above we used a Minkowski spacetime. This is a natural choice when the agents represent real objects in the world, but users can create their own spacetimes and this allows a lot of flexibility to deal with any type of simulation. We'll now show how spacetime can be used as an integral part of a definition of any multi-agent computation. 

At its most abstract, a spacetime is a vector space equipped with a `distance' function which tells us the length of any vector. We can add and subtract any two vectors to give another vector, or we can multiply a vector by a scalar to give another vector. A computation consists of a set of events, where each event is the execution of a lambda function on a given agent at a specific point in spacetime. To be a valid computation these events must have very specific relationships between them.

