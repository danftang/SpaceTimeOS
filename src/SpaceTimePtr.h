#ifndef SPACETIMEPTR_H
#define SPACETIMEPTR_H

#include <vector>
#include "SpaceTimeObject.h"


// A smart pointer for use by a spatial funciton, to run on intersection with a spatial object.
template<class T, ReferenceFrame FRAME>
class SpaceTimePtr {
protected:
    SpaceTimeObject<T,FRAME> *ptr;

    // protected constructor as we don't want the user to be passing these around
    // these pointers are just for use by SpatialFunctions which can only execute on
    // data at zero distance.
    explicit SpaceTimePtr(SpaceTimeObject<T,FRAME> *ptr): ptr(ptr) { }

    // Spawn a new SpaceTimeObject with the given arguments,
    // create a (closed) channel to the new object
    // The new object will block until the channel is opened.
    // template<class NEWTYPE, class... ARGS>
    // Channel<NEWTYPE,FRAME>::Out doSpawn(ARGS &&...args) {
    //     auto pTarget = new SpaceTimeObject<NEWTYPE,FRAME>(std::forward<ARGS>(args)...);
    //     auto pChannel = new Channel<NEWTYPE,FRAME>();
    //     pTarget->connect(typename Channel<NEWTYPE,FRAME>::In(*pChannel, *pTarget));
    //     return {*ptr, pChannel};
    // }    

public:
    friend class SpaceTimeObject<T,FRAME>; // allow access to constructor to send to SpatialFunction
    
    T *operator ->() {
        return &ptr->object;
    }

    // Spawn a new SpaceTimeObject in the same positiopn and reference frame as this object
    // and create a channel to it, connected to this. []
    template<class NEWTYPE, class... ARGS>
    ChannelWriter<NEWTYPE,FRAME> spawn(ARGS &&...args) {
         auto *pTarget = new SpaceTimeObject<NEWTYPE,FRAME>(position(), frame(), std::forward<ARGS>(args)...);
        return ChannelWriter<NEWTYPE,FRAME>{*ptr, *pTarget};
    }

    // Spawn in a new position and frame.
    // If the position is in the non-future, it will be created at the intersection of the new objet's trajectory with 
    // this object's current light-cone.
    // template<class NEWTYPE, class POS, class FRM, class... ARGS>
    // Channel<NEWTYPE, FRAME>::Out spawnAt(POS &&initPosition, FRM &&initFrame, ARGS &&...args) {
    //     return position() <= initPosition ? 
    //         doSpawn<NEWTYPE>(std::forward<POS>(initPosition), std::forward<FRM>(initFrame), std::forward<ARGS>(args)...) : 
    //         doSpawn<NEWTYPE>(initFrame.intersection(position(), initPosition), std::forward<FRM>(initFrame), std::forward<ARGS>(args)...);
    // }

    // create a new channel connected to two ChannelCouriers
    // Channel<T,FRAME> &spawnChannel() {
    //     Channel<T,FRAME> *pChannel = new Channel<T,FRAME>();
    //     ptr->connect(Channel<T,FRAME>::In(*pChannel, *ptr));
    //     return *pChannel;
    // }



    // kill the referent
    void kill() {
        delete(ptr);
        ptr = nullptr;
    }

    void attach(ChannelReader<T,FRAME> &&inChannel) {
        ptr->attach(std::move(inChannel));
    }

    template<class TARGETT>
    void attach(const ChannelWriter<TARGETT,FRAME> &outChannel) {
        outChannel.attachSource(*ptr);
    }


    // void connectIn(Channel<T,FRAME> &chan) {
    //     ptr->connect(typename Channel<T,FRAME>::In(chan, *ptr));
    // }

    // template<class TARGETTYPE>
    // Channel<TARGETTYPE,FRAME>::Out connectOut(Channel<TARGETTYPE,FRAME> *chan) {
    //     return {*ptr, chan};
    // }

    // void openIn(Channel<T,FRAME> &chan) {
    //     // check not already open
    //     ptr->connect(chan.openIn(ptr));
    // }
    

    // // Disconnect the corresponding InChannel
    // void closeIn(Channel<T,FRAME> &channel) { // this could be in OutChannel
    //     // check that channel is currently open on this object
    // }

    
    FRAME &frame() { return ptr->frameOfReference; }

    const FRAME::SpaceTime &position() const { return ptr->position; }

};

#endif
