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
#include "SourceAgent.h"
#include "predeclarations.h"
#include "ShiftedField.h"

template<Environment ENV> 
class ChannelBuffer : public ThreadSafeQueue<SpatialFunction<ENV, typename Simulation<ENV>::TranslatedLambdaField>> {
public:
    typedef typename ENV::SpaceTime SpaceTime;
protected:
    ChannelBuffer(CallbackChannel<ENV> &source) : source(&source) { }

    template<class T> requires std::same_as<typename T::Envoronment, ENV> friend class Channel; // only Channel can construct a new channel.
public:

    CallbackChannel<ENV> *              source = nullptr; // null if closed on either end

    ChannelBuffer(const ChannelBuffer<ENV> &other) = delete; // just don't copy channels
    ChannelBuffer(ChannelBuffer<ENV> &&) = delete; // just don't copy channels

};


// This is used by the Agent class to read lambdas from a channel
// Don't use these directly
template<Environment ENV>
class ChannelExecutor {
public:
    typedef ENV::SpaceTime SpaceTime;
    typedef ENV::SpaceTime::Time Time;

    // User can't construct a ChannelBuffer so no need to hide this.
    ChannelExecutor(ChannelBuffer<ENV> *channel) : buffer(channel) { }

    // Can't delete copy constructor as needed for capture in a std::function until C++23
    ChannelExecutor(const ChannelExecutor<ENV> &) {
        throw(std::runtime_error("Don't try to copy construct an ChannelReader. Use std::move instead"));
    };

    ChannelExecutor(ChannelExecutor<ENV> &&moveFrom) : buffer(moveFrom.buffer) {
        moveFrom.buffer = nullptr;
    }

    ~ChannelExecutor() {
        if(buffer != nullptr) {
            if(buffer->source == nullptr) {
                delete(buffer);
            } else {
                buffer->clear(); // delete any captured channels
                buffer->source = nullptr; // signal reader closure
            }
        }
    }

    // Could type-delete by making this into a std::function at construction (and separating position and function into two buffers)
    inline bool executeNext(Agent<ENV> &agent) const {
        if(buffer == nullptr) return false;
        if(buffer->empty()) return (buffer->source == nullptr);
        buffer->front()(agent); // do execution
        buffer->pop();
        return true;
    }

    inline bool discardNext() {
        if(buffer == nullptr) return false;
        if(buffer->empty()) return (buffer->source == nullptr);
        buffer->pop();
        return true;
    }

    ChannelExecutor &operator=(ChannelExecutor<ENV> &&moveFrom) {
        buffer = moveFrom.buffer;
        moveFrom.buffer = nullptr;
        return *this;
    }

    // position of the front of the queue, or source if empty
    // nullptr if closed and empty
    // TODO: Don't ask position, ask timeToIntersection
    // SpaceTime position() const {
    //     assert(buffer != nullptr);
    //     return (empty() ?
    //         (buffer->source != nullptr ? buffer->source->position() : SpaceTime(std::numeric_limits<Time>::max())) 
    //         : buffer->front().position());
    // }

    bool empty() const { return buffer->empty(); }

    // A channel is closed for the reader as soon as there can be no
    // more calls on this channel.
    bool isClosed() const { return buffer->source == nullptr && empty(); }

    // inline void pushCallback(Time callAfterLabTime, Agent<ENV> *agentToCallback) {
    //     assert(buffer != nullptr);
    //     assert(buffer->source != nullptr);
    //     buffer->source->pushCallback(callAfterLabTime, agentToCallback);
    // }


    // A non-empty Channel has a lambda field which is the field on the front lambda
    const auto &asLambdaField() {
        assert(buffer != nullptr);
        assert(!buffer->empty());
        return buffer->front().asField();
    }

    // A Channel has a blocking field defined by the channel source
    std::shared_ptr<CallbackField<ENV>> getCallbackField() {
        assert(buffer != nullptr);
        assert(buffer->source != nullptr);
        return buffer->source->getCallbackField();
    }

    // Channel is a field [but what kind of field!?]...

    // template<class TRAJECTORY>
    // Time timeToIntersection(const TRAJECTORY &trajectory) {
    //     assert(buffer != nullptr);
    //     return empty()?
    //         (buffer->source != nullptr ? trajectory.timeToIntersection(buffer->source->blockingField()) : std::numeric_limits<Time>::max())
    //         : trajectory.timeToIntersection(buffer->front().asField());
    // }


    friend std::ostream &operator <<(std::ostream &out, const ChannelExecutor<ENV> &in) {
        out << in.buffer;
        return out;
    }

protected:
    ChannelBuffer<ENV> *buffer;
};


// This is the class used to send lambdas to other agents.
// T is the type of agent that is the target of the channel.
// Any channel has only one Channel, so this class has no copy constructor,
// you can only std::move a Channel.
// You also can't send a Channel over a channel, use the target() method
// to get a RemoteReference to the target, which can be sent over a channel.
// TODO: think of a way we can have a Channel to a derived type of the target
// (or some way to deal with polymorphism among agents)
//  - Channel could implement two interfaces (Read<T1> and Write<T2>)
//  - Channel has a base type which allows writing, and a derived type which allows reading and implements an interface
//    the writer has a ref to base type and the reader has a ref to derived type's interface
//   - runtime typesafe execution using std::any or wrap in std::variant, or have VariantChannelWriter (or VariantChannelReader)
//   - If the channel holds the target pointer, then the buffer can hold runnables and the reader can be type unaware
//     and we can merge AgentBase and Agent (though Agents would then need virtual destructors)
// T is Target type, should be derived from Agent<ENV>
template<class T>
class Channel
{
public:
    typedef T::SpaceTime SpaceTime;
    typedef T::Environment Environment;
    friend class RemoteReference<T>;

    // a default writer indicates that the reader hasn't been generated yet
    Channel() : buffer(nullptr) {} 

    Channel(const Channel<T> &) = delete; // use SendableChannelWriter

    Channel(Channel<T> &&moveFrom) : buffer(moveFrom.buffer) {
        moveFrom.buffer = nullptr;
    }

    // create a new channel to a remote target
    Channel(CallbackChannel<Environment> &source, const Channel<T> &target) {
        buffer = new ChannelBuffer<Environment>(source);
        target.send([reader = ChannelReader(buffer)](T &obj) mutable {
            obj.attach(std::move(reader));
        });
    }

    // attach a source to a remote reference
    Channel(CallbackChannel<Environment> &source, RemoteReference<T> &target) : Channel(target.attachSource(source)) { 
    }

    // create a new channel between two local agents
    Channel(CallbackChannel<Environment> &source, T &target) {
        buffer = new ChannelBuffer<Environment>(source);
        target.attach(ChannelExecutor(buffer));
    }


    ~Channel() {
        if(buffer != nullptr) {
            if(buffer->source == nullptr) {
                delete(buffer);
            } else {
                buffer->source = nullptr;
            }
        }
    }

    Channel &operator=(Channel<T> &&moveFrom) {
        buffer = moveFrom.buffer;
        moveFrom.buffer = nullptr;
        return *this;
    }

    template<std::convertible_to<std::function<void(T &)>> LAMBDA>
    bool send(LAMBDA &&function) const {
        if(buffer == nullptr) return false;
        buffer->emplace(buffer->source->asLambdaField(), 
            [f = std::forward<LAMBDA>(function)](Agent<Environment> &target) { 
                f(static_cast<T &>(target)); 
            });
        return true;
    }

    // get a remote reference to the target of this channel
    RemoteReference<T> target() {
        return {*this};
    }


    const SpaceTime &sourcePosition() const {
        assert(buffer != nullptr);
        return buffer->source->getCallbackField()->asPosition();
    }


    friend std::ostream &operator <<(std::ostream &out, const Channel<T> &chan) {
        out << chan.buffer;
        return out;
    }


protected:
    ChannelBuffer<typename T::Environment> *buffer;
};


// Use this class to capture references to remote agents in lambda functions.
// In this way, we can send references between agents.
// This is necessary because an agent may need to block on a reference while it is in transit. 
// This is implemented as a channel connected to a dummy source located at
// the point of creation of the reference.
template<class T>
class RemoteReference {
public:
    typedef T::Environment  Environment;

    // create a new channel with a given target and a stub as source
    RemoteReference(const Channel<T> &target) : 
        outChannel(*new CallbackField<Environment>(target.sourcePosition()), target) { }

    // create a new channel to a local target
    RemoteReference(T &target) : 
        outChannel(*new CallbackField<Environment>(target.position()), target) { } // If we have a raw reference to target it must be in same position

    RemoteReference(RemoteReference<T> &&other) : outChannel(std::move(other.outChannel)) { }

    // Can't delete copy constructor as needed for capture in a std::function until C++23
    RemoteReference(const RemoteReference<T> &dummy) {
        // TODO: Could make a copy the whole channel...?
        throw(std::runtime_error("Don't try to copy construct an RemoteReference. Use std::move instead"));
    }

    ~RemoteReference() {
        if(outChannel.buffer != nullptr) { // channel is still connected to a stub object
            delete(outChannel.buffer->source);
        }
    }

    Channel<T> attachSource(SourceAgent<Environment> &source) {
//        std::cout << this << " Attaching to source " << &source << std::endl;
        assert(outChannel.buffer != nullptr);
        assert(outChannel.buffer->source != nullptr);
        delete(outChannel.buffer->source); // delete stub object (this will cause callbacks to be submitted)
        outChannel.buffer->source = &source;
        return std::move(outChannel);
    }

protected:
    Channel<T> outChannel;
};

#endif
