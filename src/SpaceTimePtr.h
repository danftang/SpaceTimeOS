#ifndef SPACETIMEPTR_H
#define SPACETIMEPTR_H

#include <vector>
#include "SpaceTimeObject.h"

template<class T, ReferenceFrame FRAME> class OutChannel;

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
    template<class NEWTYPE, class... ARGS>
    Channel<NEWTYPE,FRAME> &doSpawn(ARGS &&...args) {
        // SpaceTimeObject<NEWTYPE,FRAME> *newObjPtr = ;
        // OutChannel<NEWTYPE,FRAME> chan(*newObjPtr);
        auto pChannel = new Channel<NEWTYPE,FRAME>();
        auto pObj = new SpaceTimeObject<NEWTYPE,FRAME>(std::forward<ARGS>(args)...);
        pObj->connect(pChannel);
        return *pChannel;
    }    

public:
//    template<class T2, ReferenceFrame FRAME2> friend class OutChannel; // allow access to pointer to open channel
    friend class SpaceTimeObject<T,FRAME>; // allow access to constructor to send to SpatialFunction

    T *operator ->() {
        return &ptr->object;
    }

    // Spawn a new SpaceTimeObject in the same positiopn and reference frame as this object
    // and create a channel to it, connected to this. []
    template<class NEWTYPE, class... ARGS>
    Channel<NEWTYPE,FRAME> &spawn(ARGS &&...args) {
        return doSpawn<NEWTYPE>(position(), frame(), std::forward<ARGS>(args)...);
    }

    // Spawn in a new position and frame.
    // If the position is in the non-future, it will be created at the intersection of the new objet's trajectory with 
    // this object's current light-cone.
    template<class NEWTYPE, class POS, class FRM, class... ARGS>
    Channel<NEWTYPE, FRAME> &spawnAt(POS &&initPosition, FRM &&initFrame, ARGS &&...args) {
        return position() <= initPosition ? 
            doSpawn<NEWTYPE>(std::forward<POS>(initPosition), std::forward<FRM>(initFrame), std::forward<ARGS>(args)...) : 
            doSpawn<NEWTYPE>(initFrame.intersection(position(), initPosition), std::forward<FRM>(initFrame), std::forward<ARGS>(args)...);
    }



    // kill the referent
    void kill() {
        delete(ptr);
        ptr = nullptr;
    }

    // void connect(InChannel<T,FRAME> &&inChan) {
    //     ptr->connect(std::move(inChan));
    // }

    void openIn(Channel<T,FRAME> &chan) {
        // check not already open
        ptr->connect(chan.openIn(ptr));
    }
    
    OutChannel<T,FRAME> openOut(Channel<T,FRAME> &chan) {

    }

    // Disconnect the corresponding InChannel
    void closeIn(Channel<T,FRAME> &channel) { // this could be in OutChannel
        // check that channel is currently open on this object
    }

    
    FRAME &frame() { return ptr->frameOfReference; }

    const FRAME::SpaceTime &position() const { return ptr->position; }

};

#endif
