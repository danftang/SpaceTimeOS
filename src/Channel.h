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

#include "Concepts.h"
#include "SpatialFunction.h"
#include "ThreadPool.h"
#include "ThreadSafeQueue.h"

template<SpaceTime SPACETIME> class AgentBase;


// T must define a spacetime, have submit()
template<class T>
class Channel {
public:
    typedef typename T::SpaceTime SpaceTime;

    AgentBase<SpaceTime> *              source = nullptr; // null if closed on either end
    ThreadSafeQueue<SpatialFunction<T>> buffer;


    Channel(AgentBase<SpaceTime> &source) : source(&source) {
//        std::cout << "Creating channel at " << this << std::endl;
     }

    Channel(const Channel<T> &other) = delete; // just don't copy channels
    Channel(Channel<T> &&) = delete; // just don't copy channels

    template<std::convertible_to<std::function<void(T &)>> LAMBDA>
    void push(const SpaceTime &position, LAMBDA &&lambda) {
        buffer.emplace(position, std::forward<LAMBDA>(lambda));
    }


    // // returns true if this channel is slated for deletion
    // inline bool unblock() {
    //     isBlocking = false;
    //     if(target != nullptr) {
    //         target->submit();
    //         return false;
    //     }
    //     return source == nullptr;
    // }

    // // tell source to callback target on move
    // void setBlockingCallback() {
    //     assert(source != nullptr); // we should never block on a closed channel
    //     isBlocking = true;
    //     source->callbackOnMove([channel = this]() { 
    //         if(channel->unblock()) delete(channel);
    //     });
    // }


    // executes the next call on the target
    // returns true if not blocking
    bool executeNext(T &target) {
        if(buffer.empty()) return (source == nullptr);
        buffer.front()(target);
        buffer.pop();
        return true;
    }

};

template<class T> class ChannelWriter;
template<class T> class RemoteReference;


template<class T>
class ChannelReader {
protected:
    ChannelReader(Channel<T> *channel) : channel(channel) { }

public:
    typedef T::SpaceTime SpaceTime;
    typedef T::SpaceTime::Scalar Scalar;
    friend class ChannelWriter<T>;

    ChannelReader(const ChannelReader<T> &) = delete;

    ChannelReader(ChannelReader<T> &&moveFrom) : channel(moveFrom.channel) {
        moveFrom.channel = nullptr;
    }

    ~ChannelReader() {
        if(channel != nullptr) {
            if(channel->source == nullptr) {
                delete(channel);
            } else {
                channel->buffer.clear(); // delete any captured channels
                channel->source = nullptr; // signal reader closure
            }
        }
    }

    inline bool executeNext(T &target) const {
        assert(channel != nullptr);
        return channel->executeNext(target);
    }

    ChannelReader &operator=(ChannelReader<T> &&moveFrom) {
        channel = moveFrom.channel;
        moveFrom.channel = nullptr;
        return *this;
    }

    // position of the front of the queue, or source if empty
    // nullptr if closed and empty
    const SpaceTime &position() const {
        assert(channel != nullptr);
        return (empty() ?
            (channel->source != nullptr ? channel->source->position() : SpaceTime::TOP) 
            : channel->buffer.front().position());
    }

    bool empty() const { return channel->buffer.empty(); }

    // A channel is closed for the reader as soon as there can be no
    // more calls on this channel.
    bool isClosed() const { return channel->source == nullptr && empty(); }

    template<std::invocable LAMBDA>
    inline void callbackOnMove(LAMBDA &&lambda) {
        assert(channel != nullptr);
        assert(channel->source != nullptr);
        channel->source->callbackOnMove(std::forward<LAMBDA>(lambda));
    }

    Scalar timeToIntersection(const SpaceTime &agentPosition, const SpaceTime &agentVelocity) const {
        assert(channel != nullptr);
        return (empty() ?
            (channel->source != nullptr ? (channel->source->position() - agentPosition) / agentVelocity : std::numeric_limits<Scalar>::infinity())
            : channel->buffer.front().timeToIntersection(agentPosition,agentVelocity));
    }


    friend std::ostream &operator <<(std::ostream &out, const ChannelReader<T> &in) {
        out << in.channel->target;
        return out;
    }

protected:
    Channel<T> *channel;
};




template<class T>
class ChannelWriter
{
public:
    typedef T::SpaceTime SpaceTime;
    friend class RemoteReference<T>;

    ChannelWriter() : channel(nullptr) {} // a null writer indicates that the reader hasn't been generated yet

    ChannelWriter(const ChannelWriter<T> &) = delete; // use SendableChannelWriter

    // move a channel from another to this
    ChannelWriter(ChannelWriter<T> &&moveFrom) : channel(moveFrom.channel) {
        moveFrom.channel = nullptr;
    }

    // open a channel on the source side
    ChannelWriter(AgentBase<SpaceTime> &source, const ChannelWriter<T> &target) {
        channel = new Channel<T>(source);
        target.send([chan = channel](T &obj) {
//            std::cout << "*** Attaching channel to target " << obj << std::endl;
            obj.attach(ChannelReader(chan));
        });
    }

    ChannelWriter(AgentBase<SpaceTime> &source, T &target) {
        channel = new Channel<T>(source);
        target.attach(ChannelReader<T>(channel));
    }


    ~ChannelWriter() {
        if(channel != nullptr) {
            if(channel->source == nullptr) {
                delete(channel);
            } else {
                channel->source = nullptr;
            }
        }
    }

    ChannelWriter &operator=(ChannelWriter<T> &&moveFrom) {
        channel = moveFrom.channel;
        moveFrom.channel = nullptr;
        return *this;
    }

    template<std::convertible_to<std::function<void(T &)>> LAMBDA>
    bool send(LAMBDA &&function) const {
        assert(channel != nullptr);
        if(channel->source != nullptr) {
            channel->push(channel->source->position(), std::forward<LAMBDA>(function));
            return true;
        }
        return false;
    }


    const SpaceTime &sourcePosition() const {
        assert(channel != nullptr);
        return channel->source->pos;
    }

    // // create a new channel whose target is the same as this, by sending a message down this channel.
    // RemoteReference<T> unattachedCopy() const { return RemoteReference<T>(*this); }

    friend std::ostream &operator <<(std::ostream &out, const ChannelWriter<T> &chan) {
        out << chan.channel->target;
        return out;
    }


protected:
    Channel<T> *channel;
};


// Represents
// Implemented as a channel that is connected to a dummy source that has position
// 
template<class T>
class RemoteReference {
public:
    typedef T::SpaceTime SpaceTime;

    // create a new channel with a given target and a stub as source
    RemoteReference(const ChannelWriter<T> &target) : 
        outChannel(*new AgentBase<SpaceTime>(target.sourcePosition()), target) { }

    RemoteReference(AgentBase<SpaceTime> source, T &target) : 
        outChannel(*new AgentBase<SpaceTime>(source.position()), target) { }

    RemoteReference(RemoteReference<T> &&other) : outChannel(std::move(other.outChannel)) { }

    // Can't delete copy constructor as needed for capture in a std::function until C++23
    RemoteReference(const RemoteReference<T> &dummy) {
        throw(std::runtime_error("Don't try to copy construct an RemoteReference. Use std::move instead"));
    }

    ~RemoteReference() {
        if(outChannel.channel != nullptr) { // channel is still connected to a stub object
            delete(outChannel.channel->source);
        }
    }

    ChannelWriter<T> attachSource(AgentBase<SpaceTime> &source) && {
//        std::cout << this << " Attaching to source " << &source << std::endl;
        assert(outChannel.channel != nullptr);
        assert(outChannel.channel->source != nullptr);
        delete(outChannel.channel->source); // delete stub object (this will cause callbacks to be submitted)
        outChannel.channel->source = &source;
        return std::move(outChannel);
    }

protected:
    ChannelWriter<T> outChannel;
};


// template<class T>
// ChannelWriter<T> makeChannel(AgentBase<typename T::SpaceTime> &source, T &target) { return { source, target }; }

// template<class T>
// RemoteReference<T> makeChannel(const ChannelWriter<T> &target) { return { target }; }

// template<class T>
// RemoteReference<T> makeChannel(T &target) { return { target }; }

#endif
