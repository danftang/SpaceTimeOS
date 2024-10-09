
#include <iostream>

#include "Minkowski.h"
#include "ForwardSimulation.h"
#include "Channel.h"
#include "Agent.h"
#include "LinearTrajectory.h"

#include "TupleSpace.h"

// First create a simulation type that defines the spacetime and the means of
// executing events. Here we choose a 2 dimensional Minkowski spacetime
// and a thread-pool consisting of 2 threads.
typedef ForwardSimulation<LinearTrajectory<Minkowski<2>> , ThreadPool<0>>      MySimulation;

// Now create a class derived from Agent to exist within the simulation.
// This class just sends a ping to another agent.
class Ping : public Agent<MySimulation> {
public:
    Channel<Ping> channelToOther;

    void ping() {
        std::cout << "Ping from " << position() << std::endl;
        // "other" is a channel to the other agent, down which we can send lambda
        // functions which, on arrival, will be executed by the remote agent.
        channelToOther.send([](Ping &otherAgent) {
            otherAgent.ping();
        });
    }
};


class MyClass {};
class MyClass2 : public MyClass {
    int x;
};

int main() {

    // First create two agents. Agents delete themselves so we can use new without worrying about memory leaks.
    Ping *alice = new Ping();
    Ping *bob   = new Ping();

    // set up the agent's initial positions
    alice->jumpTo({0,0});
    bob->jumpTo({0,1});

    // now set the agent's member pointers to point to the other agent.
    alice->channelToOther = Channel(*alice, *bob);
    bob->channelToOther   = Channel(*bob, *alice);

    // Initialize the ping-pong by calling ping()
    alice->ping();

    //Now start the simulation and set a the end of the simulation to be at time 100 in the laboroatory frame
    MySimulation::start(100);

    return 0;
}