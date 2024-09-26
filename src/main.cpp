
#include <iostream>

#include "Minkowski.h"
#include "ForwardSimulation.h"
#include "Channel.h"
#include "Agent.h"


// First create a simulation type that defines the spacetime and the means of
// executing events. Here we choose a 2 dimensional Minkowski spacetime
// and a thread-pool consisting of 2 threads.
typedef ForwardSimulation<Minkowski<1> , ThreadPool<2>>      MySimulation;

// Now create a class derived from Agent to exist within the simulation.
// This class just sends a ping to another agent.
class Ping : public Agent<Ping, MySimulation> {
public:
    ChannelWriter<Ping> channelToOther;

    Ping(const MySimulation::SpaceTime &position) : Agent<Ping,MySimulation>(position) { }

    void ping() {
        std::cout << "Ping from " << position() << std::endl;
        // "other" is a channel to the other agent, down which we can send lambda
        // functions which, on arrival, will be executed by the remote agent.
        channelToOther.send([](Ping &otherAgent) {
            otherAgent.ping();
        });
    }
};


int main() {


    // First create two agents. Agents delete themselves so we can use new without worrying about memory leaks.
    // Ping *alice = new Ping({0,0});
    // Ping *bob = new Ping({0,1});
    Ping *alice = new Ping(0);
    Ping *bob = new Ping(0);

    // now set the agent's member pointers to point to the other agent.
    alice->channelToOther = ChannelWriter(*alice, *bob);
    bob->channelToOther   = ChannelWriter(*bob, *alice);

    // Initialize the ping-pong by calling ping()
    alice->ping();

    //Now start the simulation and set a the end of the simulation to be at time 100 in the laboroatory frame
    MySimulation::start(100);

    return 0;
}