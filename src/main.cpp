
#include <iostream>

#include "Minkowski.h"
#include "ForwardSimulation.h"
#include "Channel.h"
#include "Agent.h"


// First create a simulation type that defines the spacetime and the means of
// executing events. Here we choose a 2 dimensional Minkowski spacetime
// and a thread-pool consisting of 2 threads.
typedef ForwardSimulation<Minkowski<2> , ThreadPool<0>>      MySimulation;

// Now create a class derived from Agent to exist within the simulation.
// This class just sends a ping to another agent.
class Ping : public Agent<Ping, MySimulation> {
public:
    ChannelWriter<Ping> channelToOther;

    template<class P>
    Ping(P &&position) : Agent<Ping,MySimulation>(std::forward<P>(position)) {}

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
    // TODO: If we create a new Laboratory at Bottom and allow any agent to construct new agents any point in its
    // future, and set up any channels between them, then we can
    // initiate by sending lambdas to the laboratory (or using its raw reference) all initiation can be causal.
    // Or we can allow spawnAt anywhere if it returns a channel to the new agent (only a problem if we allow 
    // non channel-based agent discovery).
    // This stops the user ever getting raw references to agents.
    //
    // First create two agents. Agents delete themselves so we can use new without worrying about memory leaks.
    Ping *alice = new Ping(BoundaryCoordinate({0}));
    Ping *bob = new Ping(BoundaryCoordinate({1}));

    // now set the agent's member pointers to point to the other agent.
    alice->channelToOther = ChannelWriter(*alice, *bob);
    bob->channelToOther   = ChannelWriter(*bob, *alice);

    // Initialize the ping-pong by calling ping()
    alice->ping();

    // Now start the simulation and set a the end of the simulation to be at time 100 in the laboroatory frame
    MySimulation::start(100);

    return 0;
}