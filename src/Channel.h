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
//    char                                    flags = 0;

    // channel needs to poll source for position and signal that it is blocking
    // and needs to be able to signal to target that it has been unblocked

    // enum State {
    //     OPEN,
    //     CLOSED,
    //     CONNECTING,  // waiting for other end to connect
    //     NOCHANNEL    // channel hasn't been created yet 
    // };

    // static constexpr is_blocking_flag = 1;
    // static constexpr has_connected_in_flag = 2;
    // static constexpr has_connected_out_flag = 4;

    void unblock() {
        if(target != nullptr) {
//            flags ^= is_blocking_flag;
            (*target)();
        }
//        if(souce == nullptr) delete(this);
    }

//    bool isBlocking() { return flags & is_blocking_flag; }

    Channel(SpaceTimeBase<SpaceTime> &source, SpaceTimeObject<T,FRAME> &target) : source(&source), target(&target) { }

    Channel(const Channel<T,FRAME> &other) = delete; // just don't copy channels

public:


    // friend class SpaceTimeObject<T,FRAME>;
    // friend class SpaceTimePtr<T,FRAME>;


//     class In { 
//     protected:
//         In(Channel<T, FRAME> *channel) : channel(channel) { }

//     public:
//         friend class Channel<T,FRAME>::Out;

//         In(const In &dummy) { // here to allow capture in std::function
//             throw(std::runtime_error("Channels can only be moved"));
//         }

//         In(In &&moveFrom) : channel(moveFrom.channel) {
//             moveFrom.channel = nullptr;
//         }

//         ~In() {
//             if(channel != nullptr) {
//                 channel->target = nullptr;
//                 if(channel->source == nullptr) {
//                     delete(channel);
//                 } else {
//                     channel->buffer.clear(); // delete any captured channels
//                 }
//             }
//         }

//         // void pop() { channel->buffer.pop_front(); }
//         // const SpatialFunction<T, FRAME> &front() { return channel->buffer.front(); }

//         void executeNext(SpaceTimePtr<T,FRAME> obj) const { // obj should always be the target (pointer to target can be a SpaceTimePtr)
//             channel->buffer.front()(obj);
//             channel->buffer.pop_front();
//         }

//         // position of the front of the queue, or source if empty
//         // nullptr if closed and empty
//         const SpaceTime &position() const {
//             return (empty() ?
//                 (channel->source != nullptr ? channel->source->position : SpaceTime::TOP) 
//                 : channel->buffer.front().position);
//         }

//         // tell source to callback target on move
//         void setBlockingCallback() const {
//             if(channel->source != nullptr) {
//                 channel->source->callbackOnMove([&channel = this->channel]() { channel.unblock(); });
// //                flags |= is_blocking_flag;
//             }
//         }

//         bool empty() const { return channel->buffer.empty(); }

//         // State state() {
//         //     if(channel.source == nullptr) {
//         //         return (channel.flags & has_connected_out_flag) ? CLOSED : CONNECTING;
//         //     }
//         //     return OPEN;
//         // }

//     protected:
//         Channel<T, FRAME> *channel;
//     };
    

//     class Unattached;

//     class Out
//     {
//     public:
//         Out() : channel(nullptr) {}

//         // open a channel on the source side
//         Out(SpaceTimeBase<SpaceTime> &source, const Out &target) {
//             channel = new Channel<T,FRAME>(source, *target.channel->target);
//             target.send([inChannel = In(channel)](SpaceTimePtr<T,FRAME> obj) mutable {
//                 obj.attach(std::move(inChannel));
//             });
//         }

//         Out(SpaceTimePtr<T,FRAME> source, const Out &target) : Out(source.ptr, target) { }

//         Out(SpaceTimeBase<SpaceTime> &source, SpaceTimeObject<T, FRAME> &target) {
//             channel = new Channel<T, FRAME>(source, target);
//             target.attach(In(channel));
//         }

//         // move a channel from another to this
//         Out(Out &&moveFrom) : channel(moveFrom.channel) {
//             moveFrom.channel = nullptr;
//         }

//         ~Out() {
//             if(channel != nullptr) {
//                 channel->source = nullptr;
//                 if(channel->target == nullptr) delete(channel);
//             }
//         }

//         Out &operator=(Out &&moveFrom) {
//             channel = moveFrom.channel;
//             moveFrom.channel = nullptr;
//             return *this;
//         }

//         template <class LAMBDA>
//         bool send(LAMBDA &&function) const {
//             if(channel == nullptr) return false;
//             channel->buffer.emplace_back(channel->source->position, std::forward<LAMBDA>(function));
//             return true;
//         }

//         // template<class SOURCET>
//         // void attachSource(SpaceTimePtr<SOURCET,FRAME> newSource) const {
//         //     channel->source = newSource.ptr;
//         // }

//         void attachSource(SpaceTimeBase<SpaceTime> &newSource) const {
//             channel->source = &newSource;
//         }

//         const SpaceTime &sourcePosition() const {
//             return channel->source->position;
//         }

//         // void close() {
//         //     State s = state();
//         //     if(s != NOCHANNEL) {
//         //         channel->source = nullptr;
//         //         if(s == CLOSED) {
//         //             if(!channel->isBlocking()) delete channel; // delay deletion of blocking so that channel can unblock
//         //         } else {
//         //             if(channel->buffer.empty()) {
//         //                 // place null task on buffer to ensure the target sees the channel close
//         //                 send([](SpaceTimePtr<T,FRAME>){});
//         //             }
//         //         }
//         //         channel = nullptr;
//         //     }
//         // }

//         // create a new channel whose target is the same as this, by sending a message down this channel.
//         Unattached unattachedCopy() const { return Unattached(*this); }

//         // bool isConnected() { return channel != nullptr; } // ... to a channel
//         // bool isOpen() { return isConnected() && channel->target != nullptr; } // ... to a target

// //        State state() {
// //            if(channel == nullptr) return NOCHANNEL;
// //            if(channel->source == nullptr) {
// //                return (channel->flags & has_connected_in_flag) ? CLOSED : CONNECTING;
// //            }
// //            return OPEN;
// //        }

//     protected:
//         Channel<T, FRAME> *channel;
//     };


//     // Represents the write-end of a channel that hasn't yet been connected to an object
//     // This provides a dummy object to connect to in the meantime.
//     class Unattached : public SpaceTimeBase<SpaceTime> {
//     public:
    
//         // create a new channel with a given target
//         Unattached(const Out &target) : SpaceTimeBase<SpaceTime>(target.sourcePosition()), outChannel(*this, target) { }

//         // Unattached(Unattached &&other) channel(std::move(other.channel)) {
//         // }

//         Unattached(const Unattached &dummy) : SpaceTimeBase<SpaceTime>(dummy.outChannel.sourcePosition()) {
//             throw(std::runtime_error("Don't try to copy construct a Channel. Use std::move instead"));
//         }

//         template<class SOURCET>
//         Out &&attachSource(SpaceTimePtr<SOURCET,FRAME> newSource) {
//             newSource.attach(outChannel);
//             return std::move(outChannel);
//         }

//     protected:
//         Out outChannel;
//     };





    // template<class SRCTYPE>
    // Out open(SpaceTimePtr<SRCTYPE,FRAME> src) {
    //     if(source != nullptr) throw(std::runtime_error("Channel already connected to a source"));
    //     source = src.ptr;
    //     return Out(this);
    // }

    // In open(SpaceTimeObject<T,FRAME> *target) {
    //     if(this->target != nullptr) throw(std::runtime_error("Channel already connected to a target"));
    //     this->target = target;
    //     return In(this);
    // }

    // OutChannel<T,FRAME> openOut(SpaceTimeBase<T,FRAME> *);


//    Channel(SpaceTimeBase<FRAME> *sourcePtr = nullptr, SpaceTimeObject<T,FRAME> *targetPtr = nullptr) : source(sourcePtr), target(targetPtr) {}

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
            if(channel->source == nullptr) {
                delete(channel);
            } else {
                channel->buffer.clear(); // delete any captured channels
            }
        }
    }

    // void pop() { channel->buffer.pop_front(); }
    // const SpatialFunction<T, FRAME> &front() { return channel->buffer.front(); }

    void executeNext(SpaceTimePtr<T,FRAME> obj) const { // obj should always be the target (pointer to target can be a SpaceTimePtr)
        channel->buffer.front()(obj);
        channel->buffer.pop_front();
    }

    // position of the front of the queue, or source if empty
    // nullptr if closed and empty
    const SpaceTime &position() const {
        return (empty() ?
            (channel->source != nullptr ? channel->source->position : SpaceTime::TOP) 
            : channel->buffer.front().position);
    }

    // tell source to callback target on move
    void setBlockingCallback() const {
        if(channel->source != nullptr) {
            channel->source->callbackOnMove([&channel = this->channel]() { channel.unblock(); });
//                flags |= is_blocking_flag;
        }
    }

    bool empty() const { return channel->buffer.empty(); }

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
            if(channel->target == nullptr) delete(channel);
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

    // template<class SOURCET>
    // void attachSource(SpaceTimePtr<SOURCET,FRAME> newSource) const {
    //     channel->source = newSource.ptr;
    // }

    void attachSource(SpaceTimeBase<SpaceTime> &newSource) const {
        channel->source = &newSource;
    }

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

    template<class SOURCET>
    ChannelWriter<T,FRAME> &&attachSource(SpaceTimePtr<SOURCET,FRAME> newSource) {
        newSource.attach(outChannel);
        return std::move(outChannel);
    }

protected:
    ChannelWriter<T,FRAME> outChannel;
};


#endif
