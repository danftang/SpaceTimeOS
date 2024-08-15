#ifndef CHANNEL_H
#define CHANNEL_H


#include <deque>
#include <functional>
#include <exception>

#include "SpatialFunction.h"
//#include "SpaceTimeObject.h"
#include "ThreadPool.h"
#include "spacetime/ReferenceFrame.h"

template<ReferenceFrame FRAME> class SpaceTimeBase;
template<class T, ReferenceFrame FRAME> class SpaceTimeObject;

template<class T, ReferenceFrame FRAME>
class Channel {
protected:
    SpaceTimeBase<FRAME> *                  source = nullptr; // null if closed (target will block after processing the last item in the buffer)
    SpaceTimeObject<T,FRAME> *              target = nullptr;
    std::deque<SpatialFunction<T,FRAME>>    buffer;

    friend class InChannel<T,FRAME>;
    friend class OutChannel<T,FRAME>;

    Channel() = default; // user creates new channels through existing ones and through spawning

public:
    typedef typename FRAME::SpaceTime SpaceTime;

    class In{
    protected:
        In(Channel<T, FRAME> *channel) : channel(channel) { }
    public:
        In(InChannel<T, FRAME> &&moveFrom) : channel(moveFrom.channel) {
            moveFrom.channel = nullptr;
        }

        ~In() {
            if (channel != nullptr) {
                channel->target = nullptr;
                if (channel->source == nullptr) delete (channel);
            }
        }

        void pop() { channel->buffer.pop_front(); }
        const SpatialFunction<T, FRAME> &front() { return channel->buffer.front(); }

        // position of the front of the queue, or source if empty
        // error if closed and empty
        FRAME::SpaceTime position() const {
            return channel->buffer.empty() ? channel->source->position : channel->buffer.front().position;
        }

        // tell source to callback target on move
        void setBlockingCallback() {
            channel->source->callbackOnMove(*channel->target);
        }

    protected:
        Channel<T, FRAME> *channel;
    };

    class Out
    {
    protected:
    public:
        Out() : channel(nullptr) {}

        // create a new channel to a given target
        // Out(Out<T, FRAME> &targetChannel)
        // {
        //     channel = new Channel<T, FRAME>();
        //     // send connection code to target
        //     targetChannel.send([chan = channel](SpaceTimePtr<T, FRAME> targetObj) {
        //         targetObj.connect(chan); // TODO: make this a protected method ?
        //         });
        // }

        Out(SpaceTimeBase<FRAME> *source, SpaceTimeObject<T, FRAME> *target) {
            channel = new Channel<T, FRAME>();
            target->connect(channel);
            channel->open(source);
        }

        // move a channel from another to this
        Out(OutChannel<T, FRAME> &&moveFrom) : channel(moveFrom.channel) {
            moveFrom.channel = nullptr;
        }

        ~Out() {
            close();
        }

        Out<T, FRAME> &operator=(Out<T, FRAME> &&moveFrom) {
            channel = moveFrom.channel;
            moveFrom.channel = nullptr;
            return *this;
        }

        template <class LAMBDA>
        void send(LAMBDA &&function) {
            channel->buffer.emplace_back(channel->source->position, std::forward<LAMBDA>(function));
        }

        void close() {
            if (channel != nullptr) {
                if (channel->target == nullptr) {
                    delete channel;
                } else if(buffer->empty()) {
                    // place null task on buffer to ensure the target sees the channel close
                    send([](SpaceTimePtr<T,FRAME>){});
                }
                    
                }
                channel->source = nullptr;
                channel = nullptr;
            }
        }

        // create a new channel connected to the target of this
        Channel<T,FRAME> *newChannel() {
            Channel *newChannel = new Channel<T, FRAME>();
            // send connection code to target
            send([chan = newChannel](SpaceTimePtr<T, FRAME> targetObj) {
                targetObj.connect(chan); // TODO: make this a protected method ?
                });
            return newChannel;
        }

    protected:
        Channel<T, FRAME> *channel;
    };



    template<class SRCTYPE>
    Out open(SpaceTimePtr<SRCTYPE,FRAME> src) {
        if(source != nullptr) throw(std::runtime_error("Channel already connected to a source"));
        source = src.ptr;
        return Out(this);
    }

    InChannel<T,FRAME> open(SpaceTimeObject<T,FRAME> *target) {
        if(this->target != nullptr) throw(std::runtime_error("Channel already connected to a target"));
        this->target = target;
        return InChannel<T,FRAME>(this);
    }

    // OutChannel<T,FRAME> openOut(SpaceTimeBase<T,FRAME> *);


//    Channel(SpaceTimeBase<FRAME> *sourcePtr = nullptr, SpaceTimeObject<T,FRAME> *targetPtr = nullptr) : source(sourcePtr), target(targetPtr) {}

};

#endif

