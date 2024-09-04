// Whenever a new channel is created, In and Out smart-pointers should be created that point to it.
// The channel will be deleted as soon as both In and Out objects are deleted.
// At creation, the source and target are the object that spawned the channel (or the intended object). If a
// In or Out pointer is deleted, it sets the channel's source/target pointer to nullptr. 

#ifndef CHANNEL_H
#define CHANNEL_H


#include <deque>
#include <functional>
#include <exception>
#include <mutex>

#include "SpatialFunction.h"
// #include "SpaceTimeObject.h"
#include "ThreadPool.h"
#include "ThreadSafeQueue.h"
#include "Simulation.h"

template<SpaceTime SPACETIME> class SpaceTimeBase;
template<class T, Simulation SIM> class SpaceTimeObject;
template<class T, Simulation SIM> class SpaceTimePtr;

template<class T, Simulation SIM>
class Channel {
public:
    typedef typename SIM::SpaceTime SpaceTime;

    SpaceTimeBase<SpaceTime> *              source = nullptr; // null if closed (target will block after processing the last item in the buffer)
    SpaceTimeObject<T,SIM> *                target = nullptr;
    ThreadSafeQueue<SpatialFunction<T,SIM>> buffer;
    bool                                    isBlocking = false; // signals that this Channel is in the source's callback (so don't delete this)


    Channel(SpaceTimeBase<SpaceTime> &source, SpaceTimeObject<T,SIM> &target) : source(&source), target(&target) {
//        std::cout << "Creating channel at " << this << std::endl;
     }

    Channel(const Channel<T,SIM> &other) = delete; // just don't copy channels
    Channel(Channel<T,SIM> &&) = delete; // just don't copy channels

    template<std::convertible_to<std::function<void(SpaceTimePtr<T,SIM>)>> LAMBDA>
    void push(const SpaceTime &position, LAMBDA &&lambda) {
        buffer.emplace(position, std::forward<LAMBDA>(lambda));
    }   

    // executes the next call on the target
    // returns true if not blocking
    bool executeNext() {
//        std::cout << "Executing message on channel " << this << " with buffer size " << buffer.size() << std::endl; 
        if(buffer.empty()) {
            if(source != nullptr) {
                setBlockingCallback();
                return false;
            }
            return true;
        }
        assert(target != nullptr);
        buffer.front()(*target);
        buffer.pop();
        return true;
    }

    // returns true if this channel is slated for deletion
    inline bool unblock() {
        isBlocking = false;
        if(target != nullptr) {
            SIM::step(*target);
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
};

template<class T, Simulation SIM> class ChannelWriter;
template<class T, Simulation SIM> class UnattachedChannelWriter;

template<class T, Simulation SIM>
class ChannelReader {
protected:
    ChannelReader(Channel<T, SIM> *channel) : channel(channel) { }

public:
    typedef SIM::SpaceTime SpaceTime;
    friend class ChannelWriter<T,SIM>;

    ChannelReader(const ChannelReader<T,SIM> &) = delete;

    ChannelReader(ChannelReader<T,SIM> &&moveFrom) : channel(moveFrom.channel) {
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

    inline bool executeNext() const {
        assert(channel != nullptr);
        return channel->executeNext();
    }

    ChannelReader &operator=(ChannelReader<T,SIM> &&moveFrom) {
        channel = moveFrom.channel;
        moveFrom.channel = nullptr;
        return *this;
    }

    // position of the front of the queue, or source if empty
    // nullptr if closed and empty
    const SpaceTime &position() const {
        assert(channel != nullptr);
        return (empty() ?
            (channel->source != nullptr ? channel->source->position : SpaceTime::TOP) 
            : channel->buffer.front().position);
    }

    bool empty() const { return channel->buffer.empty(); }

    // A channel is closed for the reader as soon as there can be no
    // more calls on this channel.
    bool isClosed() const { return channel->source == nullptr && empty(); }

    friend std::ostream &operator <<(std::ostream &out, const ChannelReader<T,SIM> &in) {
        out << in.channel->target;
        return out;
    }

protected:
    Channel<T, SIM> *channel;
};




template<class T, Simulation SIM>
class ChannelWriter
{
public:
    typedef SIM::SpaceTime SpaceTime;
    friend class UnattachedChannelWriter<T,SIM>;

    ChannelWriter() : channel(nullptr) {}

    // open a channel on the source side
    ChannelWriter(SpaceTimeBase<SpaceTime> &source, const ChannelWriter<T,SIM> &target) {
        channel = new Channel<T,SIM>(source, *target.channel->target);
        target.send([chan = channel](SpaceTimePtr<T,SIM> obj) {
//            std::cout << "*** Attaching channel to target " << obj << std::endl;
            obj.attach(ChannelReader(chan));
        });
    }

    ChannelWriter(SpaceTimeBase<SpaceTime> &source, SpaceTimeObject<T, SIM> &target) {
        channel = new Channel<T, SIM>(source, target);
        target.attach(ChannelReader<T,SIM>(channel));
    }

    ChannelWriter(const ChannelWriter<T,SIM> &) = delete;

    // move a channel from another to this
    ChannelWriter(ChannelWriter<T,SIM> &&moveFrom) : channel(moveFrom.channel) {
        moveFrom.channel = nullptr;
    }

    ~ChannelWriter() {
        if(channel != nullptr) {
            channel->source = nullptr;
            if(channel->target == nullptr && !channel->isBlocking) delete(channel);
        }
    }

    ChannelWriter &operator=(ChannelWriter<T,SIM> &&moveFrom) {
        channel = moveFrom.channel;
        moveFrom.channel = nullptr;
        return *this;
    }

    template<std::convertible_to<std::function<void(SpaceTimePtr<T,SIM>)>> LAMBDA>
    void send(LAMBDA &&function) const {
        assert(channel != nullptr);
        channel->push(channel->source->position, std::forward<LAMBDA>(function));
    }


    const SpaceTime &sourcePosition() const {
        assert(channel != nullptr);
        return channel->source->position;
    }

    // create a new channel whose target is the same as this, by sending a message down this channel.
    UnattachedChannelWriter<T,SIM> unattachedCopy() const { return UnattachedChannelWriter<T,SIM>(*this); }

    friend std::ostream &operator <<(std::ostream &out, const ChannelWriter<T,SIM> &chan) {
        out << chan.channel->target;
        return out;
    }


protected:
    Channel<T, SIM> *channel;
};


// Represents the write-end of a channel that hasn't yet been connected to an object
// This provides a dummy object to connect to in the meantime.
template<class T, Simulation SIM>
class UnattachedChannelWriter {
public:
    typedef SIM::SpaceTime SpaceTime;

    // create a new channel with a given target and a stub as source
    UnattachedChannelWriter(const ChannelWriter<T,SIM> &target) : 
        outChannel(*new SpaceTimeBase<SpaceTime>(target.sourcePosition()), target) { 
            std::cout << outChannel.channel->source << " = unattached stub" << std::endl;
        }

    UnattachedChannelWriter(UnattachedChannelWriter<T,SIM> &&other) : outChannel(std::move(other.outChannel)) {
        std::cout << this << " Moving Unattached channel " << std::endl;
    }

    UnattachedChannelWriter(const UnattachedChannelWriter<T,SIM> &dummy) {
        throw(std::runtime_error("Don't try to copy construct an UnattachedChannelWriter. Use std::move instead"));
    }

    ~UnattachedChannelWriter() {
        if(outChannel.channel != nullptr) { // channel is still connected to a stub object
            delete(outChannel.channel->source);
        }
    }

    ChannelWriter<T,SIM> attachSource(SpaceTimeBase<SpaceTime> &source) && {
        std::cout << this << " Attaching to source " << &source << std::endl;
        assert(outChannel.channel != nullptr);
        assert(outChannel.channel->source != nullptr);
        delete(outChannel.channel->source); // delete stub object (this will cause callbacks to be submitted)
        outChannel.channel->source = &source;
        return std::move(outChannel);
    }

protected:
    ChannelWriter<T,SIM> outChannel;
};


#endif
