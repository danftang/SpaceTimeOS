#ifndef INCHANNEL_H
#define INCHANNEL_H

#include "Channel.h"

template<class T, class FRAME>
class InChannel {
public:
    InChannel(Channel<T,FRAME> *channel) : channel(channel) {
    }

    InChannel(InChannel<T,FRAME> &&moveFrom) : channel(moveFrom.channel) {
        moveFrom.channel = nullptr;
    }

    ~InChannel() {
        if(channel != nullptr) {
            channel->target = nullptr;
            if(channel->source == nullptr) delete(channel);
        }
    }

    void pop() { channel->buffer.pop_front(); }
    const SpatialFunction<T,FRAME> &front() { return channel->buffer.front(); }

    // position of the front of the queue, or source if empty
    // error if closed and empty
    FRAME::SpaceTime position() const {
        return channel->buffer.empty()?channel->source->position : channel->buffer.front().position;
    }

//    bool isOpen() const { return channel->source != nullptr; }

//    bool empty() const { return channel->buffer.empty(); }

    void open(SpaceTimeObject<T,FRAME> &target) { channel->target = &target; }

    // tell source to callback target on move
    void setBlockingCallback() {
        channel->source->callbackOnMove(*channel->target);
    }


protected:
    Channel<T,FRAME> *channel;
};
#endif
