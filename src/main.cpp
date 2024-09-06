
#include <iostream>

#include "Minkowski.h"
#include "ForwardSimulation.h"
#include "Channel.h"

// First decide what spacetime the objects should exist in
typedef Minkowski<2>                 MySpaceTime;

// Now create a simulation type that will allow the objects to access the spacetime
// and to submit tasks
typedef ForwardSimulation<MySpaceTime, ThreadPool<0>>      MySimulation;

// template<Simulation T> class MyClass {};
// typedef MyClass<MySimulation> test;

// Now create some objects to exist within the spacetime.
// These don't need to provide anything special, so can be
// any old object.
class Pong;

class Ping {
public:
    int pingCount = 0;
    ChannelWriter<Pong,MySimulation> other;
    void ping();
};

class Pong {
public:
    ChannelWriter<Ping,MySimulation> other;
    void pong();
};

void Ping::ping() {
    ++pingCount;
    std::cout << "Ping " << pingCount << std::endl;
    if(pingCount < 100) {
        // other is a channel to the other agent, down which we can send
        // lambda functions which, on arrival, will be executed by the
        // remote agent.
        other.send([](SpaceTimePtr<Pong,MySimulation> pOther) {
           pOther->pong();
        });
    }
}

void Pong::pong() {
    std::cout << "Pong" << std::endl;
    other.send([](SpaceTimePtr<Ping,MySimulation> pOther) {
        pOther->ping();
    });
}


int main() {
    // To create a new object within a simulation, use spawnAt and provide
    // a location. Any arguments after the location will be sent to the
    // constructor of the created object.
    // This returns a SpatialObjectPtr which is a smart pointer that points
    // to the new type
    SpaceTimePtr ping = MySimulation::spawnAt<Ping>(MySpaceTime{{0.0,-1.0}});
    SpaceTimePtr pong = MySimulation::spawnAt<Pong>(MySpaceTime{{0.0, 1.0}});

    // spatial objects communicate via channels. To create a new channel between
    // two SpatialObjectPtrs use openChannelTo.
    ping->other = ping.openChannelTo(pong);
    pong->other = pong.openChannelTo(ping);

    // Deferencing a SpatialObjectPtr returns the underlying object.
    // Here we initialise the ping-pong by calling ping()
    ping->ping();

    // Once the simulation is set up, it can be started using sumulateUntil
    // which will simulate the objects until they reach the given time
    // in the laboratory (default) reference frame.
    MySimulation::simulateUntil(100);

    return 0;
}