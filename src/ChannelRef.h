// A ptr can be captured in a lambda or held in an object. If captured in a lambda the blocking position should be
// the position of emission until called, at which point it should be the position of the target object,
// if held in an object, the blocking position should be that of the object.

#ifndef CHANNELREF_H
#define CHANNELREF_H

#include <functional>
#include "Channel.h"
#include "SpaceTimeObject.h"
#include "SpaceTimePtr.h"

template<class T, ReferenceFrame FRAME>
class ChannelRef {
public:
    ChannelRef(const ChannelRef &)=delete; // no copy constructor
    // SpaceTimePtr(const SpaceTimeBase<SPACETIME> &source, SpaceTimeObject<T,SPACETIME> &target) :
    // channel(target.addChannel(source)) { }

    ChannelRef(Channel<T,FRAME> &chan) : channel(chan) {}

    void send(SpatialFunction<T,FRAME> &&event) { channel.send(std::forward(event)); }

    template<class T2>
    void open(SpaceTimePtr<T2,FRAME> &source) { channel.open(*source.ptr); }
    void close() { channel.close(); }

    
protected:
    Channel<T,FRAME> &  channel;
//    SpaceTimeObject<T,SPACETIME> &       target;
};


#endif
