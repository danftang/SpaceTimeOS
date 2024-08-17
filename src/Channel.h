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
template<class T, ReferenceFrame FRAME> class SpaceTimePtr;

template<class T, ReferenceFrame FRAME>
class Channel {
protected:
    SpaceTimeBase<FRAME> *                  source = nullptr; // null if closed (target will block after processing the last item in the buffer)
    SpaceTimeObject<T,FRAME> *              target = nullptr;
    std::deque<SpatialFunction<T,FRAME>>    buffer;


public:
    Channel() = default;
    Channel(const Channel<T,FRAME> &other) = delete;

    typedef typename FRAME::SpaceTime SpaceTime;

    // friend class SpaceTimeObject<T,FRAME>;
    // friend class SpaceTimePtr<T,FRAME>;

    class In{
    public:
        In(Channel<T, FRAME> &channel, SpaceTimeObject<T,FRAME> &target) {
            if(channel.target == nullptr) {
                channel.target = &target;
                this->channel = &channel;
            } else {
                this->channel = nullptr;
            }
        }

        In(In &&moveFrom) : channel(moveFrom.channel) {
            moveFrom.channel = nullptr;
        }

        ~In() {
            if (isConnected()) {
                channel->target = nullptr;
                if (!isOpen()) delete (channel);
            }
        }

        // void pop() { channel->buffer.pop_front(); }
        // const SpatialFunction<T, FRAME> &front() { return channel->buffer.front(); }

        void executeNext(SpaceTimePtr<T,FRAME> obj) {
            channel->buffer.front()(obj);
            channel->buffer.pop_front();
        }

        // position of the front of the queue, or source if empty
        // nullptr if closed and empty
        const FRAME::SpaceTime *positionPtr() const {
            typename FRAME::SpaceTime *ppos = nullptr;
            if(empty()) {
                if(isOpen()) ppos = &channel->source->position;
            } else {
                ppos = &channel->buffer.front().position;
            }
            return ppos;
        }

        // tell source to callback target on move
        void setBlockingCallback() {
            if(isOpen()) channel->source->callbackOnMove(*channel->target);
        }

        bool empty() { return channel->buffer.empty(); }

        bool isConnected() { return channel != nullptr; }
        bool isOpen() { return isConnected() && channel->source != nullptr; }

    protected:
        Channel<T, FRAME> *channel;
    };

    class Out
    {
    protected:
    public:
        Out() : channel(nullptr) {}

        Out(SpaceTimeBase<FRAME> &source, Channel<T,FRAME> &channel) {
            if(channel.source == nullptr) {
                channel.source = &source;
                this->channel = &channel;
            } else {
                this->channel = nullptr; // channel already open
            }
        }


        // Out(SpaceTimeBase<FRAME> *source, SpaceTimeObject<T, FRAME> *target) {
        //     channel = new Channel<T, FRAME>();
        //     target->connect(channel);
        //     channel->open(source);
        // }

        // move a channel from another to this
        Out(Out &&moveFrom) : channel(moveFrom.channel) {
            moveFrom.channel = nullptr;
        }

        ~Out() {
            close();
        }

        Out &operator=(Out &&moveFrom) {
            channel = moveFrom.channel;
            moveFrom.channel = nullptr;
            return *this;
        }

        template <class LAMBDA>
        void send(LAMBDA &&function) {
            channel->buffer.emplace_back(channel->source->position, std::forward<LAMBDA>(function));
        }

        void close() {
            // TODO: Danger if we close this end before the other end has connected, then the other end tried to connect.
            // need to distinguish between never-connected and closed for each end of the channel.
            if (isConnected()) {
                if (channel->target == nullptr) {
                    delete channel;
                } else {
                    if(channel->buffer.empty()) {
                        // place null task on buffer to ensure the target sees the channel close
                        send([](SpaceTimePtr<T,FRAME>){});
                    }
                    channel->source = nullptr;
                    channel = nullptr;
                }
            }
        }

        // create a new channel and send a message down this channel connecting to the target of this
        Channel<T,FRAME> &newChannel() {
            Channel *newChannel = new Channel<T, FRAME>();
            // send connection code to target
            send([chan = newChannel](SpaceTimePtr<T, FRAME> targetObj) {
                targetObj.connectIn(*chan);
            });
            return *newChannel;
        }

        bool isConnected() { return channel != nullptr; } // ... to a channel
        bool isOpen() { return isConnected() && channel->target != nullptr; } // ... to a target

    protected:
        Channel<T, FRAME> *channel;
    };



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

#endif

