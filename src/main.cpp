
#include <functional>
#include <memory>


#include <iostream>
#include "spacetime/IntertialFrame.h"
#include "spacetime/Minkowski.h"
#include "spacetime/ReferenceFrame.h"
#include "SpaceTimePtr.h"
#include "Laboratory.h"

typedef spacetime::IntertialFrame<spacetime::Minkowski<2, 100.0>> MyFrame;

class Pong;

class Ping {
public:
    int pingCount = 0;
    Channel<Pong,MyFrame>::Out other;
    void ping();
};

class Pong {
public:
    Channel<Ping,MyFrame>::Out other;
    void pong();
};

void Ping::ping() {
    ++pingCount;
    std::cout << pingCount << std::endl;
    if(pingCount < 100) {
        other.send([](SpaceTimePtr<Pong,MyFrame> pOther) {
           pOther->pong();
        });
    }
}

void Pong::pong() {
    other.send([](SpaceTimePtr<Ping,MyFrame> pOther) {
        pOther->ping();
    });
}


int main() {


    
    // First decide what spacetime and frame of reference we
    // are using, and create a frame in which the laboratory will exist.
    // Here we use the default frame of reference.
    MyFrame laboratoryFrame;

    // Now create a laboratory object that exists in the laboratory frame.
    Laboratory laboratory(laboratoryFrame); 

    // The laboratory can spawn objects to set up our experiment.
    auto pingOut = laboratory.spawn<Ping>();
    auto pongOut = laboratory.spawn<Pong>();

    // Connect Ping and Pong together
    pingOut.send([&pongChan = pongOut.newChannel()](SpaceTimePtr<Ping,MyFrame> pingPtr) {
        pingPtr->other = pingPtr.connectOut(pongChan);
    });

    pongOut.send([&pingChan = pingOut.newChannel()](SpaceTimePtr<Pong,MyFrame> pongPtr) {
        pongPtr->other = pongPtr.connectOut(pingChan);
    });

    // Initiate the ping-pong
    pingOut.send([](SpaceTimePtr<Ping,MyFrame> objPtr) {
        objPtr->ping();
    });

    laboratory.simulateFor(50.0);

    pingOut.send([](SpaceTimePtr<Ping,MyFrame> pingPtr) {
        pingPtr.kill();
    });
    pingOut.close();
    pongOut.close();

 
    std::cout << "Goodbye world" << std::endl;
    return 0;
}