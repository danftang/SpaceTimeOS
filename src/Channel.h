// Whenever a new channel is created, In and Out smart-pointers should be created that point to it.
// The channel will be deleted as soon as both In and Out objects are deleted.
// At creation, the source and target are the object that spawned the channel (or the intended object). If a
// In or Out pointer is deleted, it sets the channel's source/target pointer to nullptr. 

#ifndef CHANNEL_H
#define CHANNEL_H


#include <deque>
#include <functional>
#include <exception>

#include "SpatialFunction.h"
//#include "SpaceTimeObject.h"
#include "ThreadPool.h"
#include "spacetime/ReferenceFrame.h"

template<SpaceTime SPACETIME> class SpaceTimeBase;
template<class T, ReferenceFrame FRAME> class SpaceTimeObject;
template<class T, ReferenceFrame FRAME> class SpaceTimePtr;

template<class T, ReferenceFrame FRAME>
class Channel {
public:
    typedef typename FRAME::SpaceTime SpaceTime;

    SpaceTimeBase<SpaceTime> *              source = nullptr; // null if closed (target will block after processing the last item in the buffer)
    SpaceTimeObject<T,FRAME> *              target = nullptr;
    std::deque<SpatialFunction<T,FRAME>>    buffer;
    bool                                    isBlocking = false; // signals that this Channel is in the source's callback (so don't delete this)

    // returns true if this channel is slated for deletion
    inline bool unblock() {
        isBlocking = false;
        if(target != nullptr) {
            target->step();
            return false;
        }
        return source == nullptr;
    }

    // tell source to callback target on move
    void setBlockingCallback() {
        assert(source != nullptr); // we should never block on a closed channel
        isBlocking = true;
        source->callbackOnMove([channel = this]() { 
            if(channel->unblock()) delete(channel);
        });
    }

    Channel(SpaceTimeBase<SpaceTime> &source, SpaceTimeObject<T,FRAME> &target) : source(&source), target(&target) { }

    Channel(const Channel<T,FRAME> &other) = delete; // just don't copy channels

};

template<class T, ReferenceFrame FRAME> class ChannelWriter;
template<class T, ReferenceFrame FRAME> class UnattachedChannelWriter;

template<class T, ReferenceFrame FRAME>
class ChannelReader {
protected:
    ChannelReader(Channel<T, FRAME> *channel) : channel(channel) { }

public:
    typedef FRAME::SpaceTime SpaceTime;
    friend class ChannelWriter<T,FRAME>;

    ChannelReader(const ChannelReader<T,FRAME> &dummy) { // here to allow capture in std::function
        throw(std::runtime_error("Channels can only be moved"));
    }

    ChannelReader(ChannelReader<T,FRAME> &&moveFrom) : channel(moveFrom.channel) {
        moveFrom.channel = nullptr;
    }

    ~ChannelReader() {
        if(channel != nullptr) {
            channel->target = nullptr;
            if(channel->source == nullptr && !channel->isBlocking) {
                delete(channel);
            } else {
                channel->buffer.clear(); // delete any captured channels
            }
        }
    }


    ChannelReader &operator=(ChannelReader<T,FRAME> &&moveFrom) {
        channel = moveFrom.channel;
        moveFrom.channel = nullptr;
        return *this;
    }


    // executes the next call on the target
    // returns true if an event was processed
    bool executeNext() const {
        assert(channel != nullptr);
        if(empty()) {
            if(channel->source != nullptr) channel->setBlockingCallback();
            return false;
        }
        assert(channel->target != nullptr);
        channel->buffer.front()(*channel->target);
        channel->buffer.pop_front();
        return true;
    }

    // position of the front of the queue, or source if empty
    // nullptr if closed and empty
    const SpaceTime &position() const {
        return (empty() ?
            (channel->source != nullptr ? channel->source->position : SpaceTime::TOP) 
            : channel->buffer.front().position);
    }

    bool empty() const { return channel->buffer.empty(); }

    // A channel is closed for the reader as soon as there can be no
    // more calls on this channel.
    bool isClosed() const { return channel->source == nullptr && empty(); }

    // State state() {
    //     if(channel.source == nullptr) {
    //         return (channel.flags & has_connected_out_flag) ? CLOSED : CONNECTING;
    //     }
    //     return OPEN;
    // }

protected:
    Channel<T, FRAME> *channel;
};




template<class T, ReferenceFrame FRAME>
class ChannelWriter
{
public:
    typedef FRAME::SpaceTime SpaceTime;
    friend class UnattachedChannelWriter<T,FRAME>;

    ChannelWriter() : channel(nullptr) {}

    // open a channel on the source side
    ChannelWriter(SpaceTimeBase<SpaceTime> &source, const ChannelWriter<T,FRAME> &target) {
        channel = new Channel<T,FRAME>(source, *target.channel->target);
        target.send([inChannel = ChannelReader<T,FRAME>(channel)](SpaceTimePtr<T,FRAME> obj) mutable {
            obj.attach(std::move(inChannel));
        });
    }

    ChannelWriter(SpaceTimePtr<T,FRAME> source, const ChannelWriter<T,FRAME> &target) : ChannelWriter(source.ptr, target) { }

    ChannelWriter(SpaceTimeBase<SpaceTime> &source, SpaceTimeObject<T, FRAME> &target) {
        channel = new Channel<T, FRAME>(source, target);
        target.attach(ChannelReader<T,FRAME>(channel));
    }

    // move a channel from another to this
    ChannelWriter(ChannelWriter<T,FRAME> &&moveFrom) : channel(moveFrom.channel) {
        moveFrom.channel = nullptr;
    }

    ~ChannelWriter() {
        if(channel != nullptr) {
            channel->source = nullptr;
            if(channel->target == nullptr && !channel->isBlocking) delete(channel);
        }
    }

    ChannelWriter &operator=(ChannelWriter<T,FRAME> &&moveFrom) {
        channel = moveFrom.channel;
        moveFrom.channel = nullptr;
        return *this;
    }

    template <class LAMBDA>
    bool send(LAMBDA &&function) const {
        if(channel == nullptr) return false;
        channel->buffer.emplace_back(channel->source->position, std::forward<LAMBDA>(function));
        return true;
    }

    // void attachSource(SpaceTimeBase<SpaceTime> &source) const {
    //     assert(channel->source == nullptr);
    //     channel->source = &newSource;
    // }

    const SpaceTime &sourcePosition() const {
        return channel->source->position;
    }

    // create a new channel whose target is the same as this, by sending a message down this channel.
    UnattachedChannelWriter<T,FRAME> unattachedCopy() const { return UnattachedChannelWriter<T,FRAME>(*this); }

protected:
    Channel<T, FRAME> *channel;
};


// Represents the write-end of a channel that hasn't yet been connected to an object
// This provides a dummy object to connect to in the meantime.
template<class T, ReferenceFrame FRAME>
class UnattachedChannelWriter : public SpaceTimeBase<typename FRAME::SpaceTime> {
public:
    typedef FRAME::SpaceTime SpaceTime;

    // create a new channel with a given target
    UnattachedChannelWriter(const ChannelWriter<T,FRAME> &target) : SpaceTimeBase<SpaceTime>(target.sourcePosition()), outChannel(*this, target) { }

    UnattachedChannelWriter(UnattachedChannelWriter<T,FRAME> &&other) : SpaceTimeBase<SpaceTime>(other), outChannel(std::move(other.outChannel)) { }

    UnattachedChannelWriter(const UnattachedChannelWriter<T,FRAME> &dummy) : SpaceTimeBase<SpaceTime>(dummy.outChannel.sourcePosition()) {
        throw(std::runtime_error("Don't try to copy construct an unattached Channel. Use std::move instead"));
    }

    ChannelWriter<T,FRAME> &&attachSource(SpaceTimeBase<SpaceTime> &source) {
        assert(outChannel.channel != nullptr);
        outChannel.channel->source = &source;
        return std::move(outChannel);
    }

protected:
    ChannelWriter<T,FRAME> outChannel;
};


#endif
