
#include <functional>
#include <memory>

int main() {
        std::unique_ptr<char> up(new char());

    auto lambda = [upc = std::move(up)]() mutable {
        
    };

    std::function<void()> f(std::move(lambda));

}

// #include <iostream>
// #include "spacetime/IntertialFrame.h"
// #include "spacetime/Minkowski.h"
// #include "spacetime/ReferenceFrame.h"
// #include "SpaceTimePtr.h"
// #include "Laboratory.h"

// typedef spacetime::IntertialFrame<spacetime::Minkowski<2, 100.0>> MyFrame;

// class Pong;

// class Ping {
// public:
//     int pingCount = 0;
//     OutChannel<Pong,MyFrame> other;
//     void ping();
// };

// class Pong {
// public:
//     OutChannel<Ping,MyFrame> other;
//     void pong();
// };

// void Ping::ping() {
//     ++pingCount;
//     std::cout << pingCount << std::endl;
//     if(pingCount < 100) {
//         other.send([](SpaceTimePtr<Pong,MyFrame> pOther) {
//            pOther->pong();
//         });
//     }
// }

// void Pong::pong() {
//     other.send([](SpaceTimePtr<Ping,MyFrame> pOther) {
//         pOther->ping();
//     });
// }


// int main() {


    
//     // First decide what spacetime and frame of reference we
//     // are using, and create a frame in which the laboratory will exist.
//     // Here we use the default frame of reference.
//     MyFrame laboratoryFrame;

//     // Now create a laboratory object that exists in the laboratory frame.
//     Laboratory laboratory(laboratoryFrame); 

//     // The laboratory can spawn objects to set up our experiment.
//     OutChannel pingChan = laboratory.spawn<Ping>();
//     OutChannel pongChan = laboratory.spawn<Pong>();

//     // Connect Ping and Pong together
//     pingChan.send([c = OutChannel(pongChan)](SpaceTimePtr<Ping,MyFrame> objPtr) mutable {
//         objPtr->other = std::move(c);
//     });

//     pongChan.send([c = OutChannel(pingChan)](SpaceTimePtr<Pong,MyFrame> objPtr) mutable {
//         objPtr->other = std::move(c);
//     });

//     // Initiate the ping-pong
//     pingChan.send([](SpaceTimePtr<Ping,MyFrame> objPtr) {
//         objPtr->ping();
//     });

//     // set the agents free
//     pingChan.close();
//     pongChan.close();

 
//     std::cout << "Goodbye world" << std::endl;
//     return 0;
// }