// A ChannelCourier is an object that connects to the read or write end of a channel
// and can be captured and sent in a lambda/std::function to be sent down another
// channel.

#ifndef CHANNELCOURIER_H
#define CHANNELCOURIER_H

#include "SpaceTimeObject.h"

template<class SPACETIME, class CHANNELREF>
class ChannelCourier : public SpaceTimeBase<SPACETIME> {
public:
    CHANNELREF channelRef;

    ChannelCourier(CHANNELREF &&channelRefToMove) : channelRef(std::move(channelRefToMove)) {}

    ChannelCourier(ChannelCourier<CHANNELREF> &&other) channelRef(std::move(other.channelRef)) { }

    // this needs to be present because std::function can only capture copy constructible objects
    ChannelCourier(const ChannelCourier<CHANNELREF> &dummy) { 
        throw(std::runtime_error("Don't try to copy construct a ChannelCourier. Use std::move instead"));
    }

    CHANNELREF &&get(SpaceTimeBase<>) { return std::move(channelRef); }
};

#endif