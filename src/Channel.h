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
template<class T, class ENV> class Agent;
template<class T> class ChannelWriter;
template<class T> class RemoteReference;

// The meeting point for ChannelReaders and ChannelWriters.
// Don't use this class directly
template<class T>
class Channel {
public:
    typedef typename T::SpaceTime SpaceTime;
protected:
    Channel(AgentBase<SpaceTime> &source) : source(&source) { }

    friend ChannelWriter<T>; // only ChannelWriter can construct a new channel.
public:

    AgentBase<SpaceTime> *              source = nullptr; // null if closed on either end
    ThreadSafeQueue<SpatialFunction<T>> buffer;

    Channel(const Channel<T> &other) = delete; // just don't copy channels
    Channel(Channel<T> &&) = delete; // just don't copy channels

    template<std::convertible_to<std::function<void(T &)>> LAMBDA>
    bool push(LAMBDA &&lambda) {
        if(source != nullptr) {
            buffer.emplace(source->position(), std::forward<LAMBDA>(lambda));
            return true;
        }
        return false;
    }

    // executes the next call on the target
    // returns true if not blocking
    bool executeNext(T &target) {
        if(buffer.empty()) return (source == nullptr);
        buffer.front()(target);
        buffer.pop();
        return true;
    }

//    bool empty() const { return buffer.empty(); }

};


// This is used by the Agent class to read lambdas from a channel
// Don't use these directly
template<class T>
class ChannelReader {
protected:
    ChannelReader(Channel<T> *channel) : channel(channel) { }
    friend ChannelWriter<T>;

public:
    typedef T::SpaceTime SpaceTime;
    typedef T::SpaceTime::Scalar Scalar;

    // Can't delete copy constructor as needed for capture in a std::function until C++23
    ChannelReader(const ChannelReader<T> &) {
        throw(std::runtime_error("Don't try to copy construct an ChannelReader. Use std::move instead"));
    };

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

    // Could type-delete by making this into a std::function at construction (and separating position and function into two buffers)
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
    inline void pushCallback(LAMBDA &&lambda) {
        assert(channel != nullptr);
        assert(channel->source != nullptr);
        channel->source->pushCallback(std::forward<LAMBDA>(lambda));
    }

    Scalar timeToIntersection(const SpaceTime &agentPosition, const SpaceTime &agentVelocity) const {
        assert(channel != nullptr);
        return (empty() ?
            (channel->source != nullptr ? (channel->source->position() - agentPosition) / agentVelocity : std::numeric_limits<Scalar>::max())
            : channel->buffer.front().timeToIntersection(agentPosition, agentVelocity));
    }


    friend std::ostream &operator <<(std::ostream &out, const ChannelReader<T> &in) {
        out << in.channel;
        return out;
    }

protected:
    Channel<T> *channel;
};


// This is the class used to send lambdas to other agents.
// T is the type of agent that is the target of the channel.
// Any channel has only one ChannelWriter, so this class has no copy constructor,
// you can only std::move a ChannelWriter.
// You also can't send a ChannelWriter over a channel, use the target() method
// to get a RemoteReference to the target, which can be sent over a channel.
// TODO: think of a way we can have a ChannelWriter to a derived type of the target
// (or some way to deal with polymorphism among agents)
//  - Channel could implement two interfaces (Read<T1> and Write<T2>)
//  - Channel has a base type which allows writing, and a derived type which allows reading and implements an interface
//    the writer has a ref to base type and the reader has a ref to derived type's interface
//   - runtime typesafe execution using std::any or wrap in std::variant, or have VariantChannelWriter (or VariantChannelReader)
//   - If the channel holds the target pointer, then the buffer can hold runnables and the reader can be type unaware
//     and we can merge AgentBase and Agent (though Agents would then need virtual destructors)
template<class T>
class ChannelWriter
{
public:
    typedef T::SpaceTime SpaceTime;
    friend class RemoteReference<T>;

    // a default writer indicates that the reader hasn't been generated yet
    ChannelWriter() : channel(nullptr) {} 

    ChannelWriter(const ChannelWriter<T> &) = delete; // use SendableChannelWriter

    ChannelWriter(ChannelWriter<T> &&moveFrom) : channel(moveFrom.channel) {
        moveFrom.channel = nullptr;
    }

    // create a new channel to a remote target
    ChannelWriter(AgentBase<SpaceTime> &source, const ChannelWriter<T> &target) {
        channel = new Channel<T>(source);
        target.send([reader = ChannelReader(channel)](T &obj) mutable {
            obj.attach(std::move(reader));
        });
    }

    // attach a source to a remote reference
    ChannelWriter(AgentBase<SpaceTime> &source, RemoteReference<T> &target) : ChannelWriter(target.attachSource(source)) { 
    }

    // create a new channel between two local agents
    ChannelWriter(AgentBase<SpaceTime> &source, T &target) requires std::derived_from<T,Agent<T,typename T::Environment>> {
        channel = new Channel<T>(source);
        target.attach(ChannelReader(channel));
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
        if(channel == nullptr) return false;
        return channel->push(std::forward<LAMBDA>(function));
    }

    // get a remote reference to the target of this channel
    RemoteReference<T> target() {
        return {*this};
    }


    const SpaceTime &sourcePosition() const {
        assert(channel != nullptr);
        return channel->source->pos;
    }


    friend std::ostream &operator <<(std::ostream &out, const ChannelWriter<T> &chan) {
        out << chan.channel;
        return out;
    }


protected:
    Channel<T> *channel;
};


// Use this class to capture references to remote agents in lambda functions.
// In this way, we can send references between agents.
// This is necessary because an agent may need to block on a reference while it is in transit. 
// This is implemented as a channel connected to a dummy source located at
// the point of creation of the reference.
template<class T>
class RemoteReference {
public:
    typedef T::SpaceTime SpaceTime;

    // create a new channel with a given target and a stub as source
    RemoteReference(const ChannelWriter<T> &target) : 
        outChannel(*new AgentBase<SpaceTime>(target.sourcePosition()), target) { }

    // create a new channel to a local target
    RemoteReference(T &target) : 
        outChannel(*new AgentBase<SpaceTime>(target.position()), target) { } // If we have a raw reference to target it must be in same position

    RemoteReference(RemoteReference<T> &&other) : outChannel(std::move(other.outChannel)) { }

    // Can't delete copy constructor as needed for capture in a std::function until C++23
    RemoteReference(const RemoteReference<T> &dummy) {
        // TODO: Could make a copy the whole channel...?
        throw(std::runtime_error("Don't try to copy construct an RemoteReference. Use std::move instead"));
    }

    ~RemoteReference() {
        if(outChannel.channel != nullptr) { // channel is still connected to a stub object
            delete(outChannel.channel->source);
        }
    }

    ChannelWriter<T> attachSource(AgentBase<SpaceTime> &source) {
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

#endif
