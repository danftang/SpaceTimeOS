// A ptr can be captured in a lambda or held in an object. If captured in a lambda the blocking position should be
// the position of emission until called, at which point it should be the position of the target object,
// if held in an object, the blocking position should be that of the object.

// User interface:
//   - open a new channel to the target of an existing channel
//   - send a closed outChannel to the target of another channel

#ifndef OUTCHANNEL_H
#define OUTCHANNEL_H

#include <functional>
#include "Channel.h"
#include "SpaceTimeObject.h"
#include "SpaceTimePtr.h"
//#include "InChannel.h"

// template<class T, ReferenceFrame FRAME>
// class OutChannel {
// public:
//     OutChannel() : channel(nullptr) {}

//     // create a new channel to a given target
//     OutChannel(OutChannel<T,FRAME> &targetChannel) {
//         channel = new Channel<T,FRAME>();
//         InChannel inChannel(channel);
//         // send connection code to target
//         targetChannel.send([inChan = std::move(inChannel)](SpaceTimePtr<T,FRAME> targetObj) mutable {
//             targetObj.connect(std::move(inChan));
//         });
//     }

//     // create a new channel to a given target
//     OutChannel(SpaceTimeObject<T,FRAME> &target) {
//         channel = new Channel<T,FRAME>();
//         target.connect(InChannel(channel));
//     }

//     // move a channel from another to this
//     OutChannel(OutChannel<T,FRAME> &&moveFrom) : channel(moveFrom.channel) {
//         moveFrom.channel = nullptr;
//     }

//     ~OutChannel() {
//         if(channel != nullptr) {
//             channel->source = nullptr;
//             if(channel->target == nullptr) delete channel;
//         }
//     }


//     OutChannel<T,FRAME> &operator =(OutChannel<T,FRAME> &&moveFrom) {
//         channel = moveFrom.channel;
//         moveFrom.channel = nullptr;
//         return *this;
//     }

//     // SpaceTimePtr(const SpaceTimeBase<SPACETIME> &source, SpaceTimeObject<T,SPACETIME> &target) :
//     // channel(target.addChannel(source)) { }


//     template<class LAMBDA>
//     void send(LAMBDA &&function) {
//         channel->buffer.emplace_back(channel->source->position, std::forward<LAMBDA>(function));
//     }

//     void open(SpaceTimeBase<FRAME> &source) { channel->source = &source; }
//     void close() { 
//         channel->source = nullptr;
//         send([](SpaceTimePtr<T,FRAME> ptr) {
//             ptr.
//         });
//     }



    
// protected:
//     Channel<T,FRAME> *  channel;
// };


#endif
