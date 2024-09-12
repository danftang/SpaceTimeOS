
#include <iostream>

#include "Minkowski.h"
#include "ForwardSimulation.h"
#include "Channel.h"
#include "Agent.h"

// First create a simulation type that defines the spacetime and the means of
// executing events. Here we choose a 2 dimensional Minkowski spacetime
// and a thread-pool consisting of 2 threads.
typedef ForwardSimulation<Minkowski<2> , ThreadPool<2>>      MySimulation;

// Now create a class derived from Agent to exist within the simulation.
// This class just sends a ping to another agent.
class Ping : public Agent<Ping, MySimulation> {
public:
    ChannelWriter<Ping> other;

    Ping() : Agent<Ping,MySimulation>(Minkowski<2>(), Minkowski<2>(1)) {}

    ~Ping() { std::cout << "Deleting Ping " << std::endl; }

    void ping() {
        std::cout << "Ping from " << position() << std::endl;
        // "other" is a channel to the other agent, down which we can send lambda
        // functions which, on arrival, will be executed by the remote agent.
        other.send([](Ping &otherAgent) {
            otherAgent.ping();
        });
    }
};


int main() {
    // First create two agents. Agents delete themselves so we can use new without worrying about memory leaks.
    Ping *alice = new Ping();
    Ping *bob = new Ping();

    // now set the agent's member pointers to point to the other agent.
    alice->other = ChannelWriter(*alice, *bob);
    bob->other   = ChannelWriter(*bob, *alice);

    // Initialise the ping-pong by calling ping()
    alice->ping();

    // Now start the simulation, here we simply set a max-time in the laboroatory frame
    MySimulation::start(100);

    // Stopping the simulation ensures that all threads are finished.
    MySimulation::stop();

    return 0;
}