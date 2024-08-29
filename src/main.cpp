
#include <functional>
#include <memory>


#include <iostream>
#include "spacetime/IntertialFrame.h"
#include "spacetime/Minkowski.h"
#include "spacetime/ReferenceFrame.h"
#include "SpaceTimePtr.h"
#include "Laboratory.h"
#include "Channel.h"

typedef spacetime::IntertialFrame<spacetime::Minkowski<2, 100.0>> MyFrame;

class Pong;

class Ping {
public:
    int pingCount = 0;
    ChannelWriter<Pong,MyFrame> other;
    void ping();
    void test() { std::cout << "Executing test" << std::endl; }
};

class Pong {
public:
    ChannelWriter<Ping,MyFrame> other;
    void pong();
};

void Ping::ping() {
    ++pingCount;
    std::cout << "Ping " << pingCount << std::endl;
    if(pingCount < 100) {
        other.send([](SpaceTimePtr<Pong,MyFrame> pOther) {
           pOther->pong();
        });
    }
}

void Pong::pong() {
    std::cout << "Pong" << std::endl;
    other.send([](SpaceTimePtr<Ping,MyFrame> pOther) {
        pOther->ping();
    });
}


int main() {
    std::cout << "Hello world" << std::endl;
    // TODO: Sort out initiation of callbacks.
    // Once started, each agent should either be blocking on a single other agent
    // or be submitted as a task.
    // On executio of a step, an agent should move forward until it blocks, then
    // register itself as a callback on the blocking agent.
    // Initially, the Laboratory object should be the only object that
    // isn't blocking. So, to start the simulation we move the laboratory forward and call all its callbacks
    //
    // A simulation can be:
    //      - Calculation of the state of a given agent at a given time in its local frame.
    //      - Calculation of all agents until no more messages are being passed (equilibrium).
    //      - Calculation until all agents fall off the edge of spacetime.
    // 
    // At spawning, the parent agent can always be the blocking agent of the child (since there is always a channel between them).
    // If a ChannelWriter is deleted and the channel is blocking it should unblock the channel immediately [and not unblock it when the agent next moves].
    // 
    //  
    
    // First decide what spacetime and frame of reference we
    // are using, and create a frame in which the laboratory will exist.
    // Here we use the default frame of reference.
    MyFrame laboratoryFrame;

    // Now create a laboratory object that exists in the laboratory frame.
    Laboratory laboratory(laboratoryFrame); 

    // The laboratory can spawn objects to set up our experiment.
    auto pingOut = laboratory.spawn<Ping>();
    auto pongOut = laboratory.spawn<Pong>();

    std::cout << "------" << std::endl;
    std::cout << laboratory << " = Laboratory" << std::endl;
    std::cout << pingOut << " = Ping"  << std::endl;
    std::cout << pongOut << " = Pong" << std::endl;

    // Connect Ping and Pong together
    pingOut.send([pongChan = pongOut.unattachedCopy()](SpaceTimePtr<Ping,MyFrame> pingPtr) mutable {
        std::cout << "*** Connecting ping to pong" << std::endl;
        pingPtr->other = pingPtr.attach(std::move(pongChan));
    });

    pongOut.send([pingChan = pingOut.unattachedCopy()](SpaceTimePtr<Pong,MyFrame> pongPtr) mutable {
        std::cout << "*** Connecting pong to ping" << std::endl;
        pongPtr->other = pongPtr.attach(std::move(pingChan));
        // pongPtr->other.send([](SpaceTimePtr<Ping,MyFrame> objPtr) {
        //     std::cout << "*** Executing initial ping" << std::endl;
        //     objPtr->ping();
        // });
    });

    // laboratory.simulateFor(1.0);

    // executor.start();

    // Initiate the ping-pong
    pingOut.send([](SpaceTimePtr<Ping,MyFrame> objPtr) {
        std::cout << "*** Executing initial ping" << std::endl;
        objPtr->ping();
    });



    laboratory.simulateFor(49.0);

    executor.start(); // NB: Only need this for non-threaded exec.

//    executor.submit([]() { std::cout << "Hello world" << std::endl; });

    // pingOut.send([](SpaceTimePtr<Ping,MyFrame> pingPtr) {
    //     pingPtr.kill();
    // });

    // pingOut.close();
    // pongOut.close();
 
    std::cout << "Goodbye world" << std::endl;
    return 0;
}