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


public:
    friend class SpatialFunction<T,FRAME>; // allow access to constructor to send to SpatialFunction
    
    T *operator ->() { return &ptr->object; }

    // Spawn a new SpaceTimeObject in the same position and reference frame as this object
    // and create a channel to it, connected to this. []
    template<class NEWTYPE, class... ARGS>
    ChannelWriter<NEWTYPE,FRAME> spawn(ARGS &&...args) {
        auto *pTarget = new SpaceTimeObject<NEWTYPE,FRAME>(position(), frame(), std::forward<ARGS>(args)...);
        ChannelWriter<NEWTYPE,FRAME> writer{*ptr, *pTarget};
        pTarget->step(); // make sure new object is blocking on this
        return std::move(writer);
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


    // kill the referent
    void kill() {
        delete(ptr);
        ptr = nullptr;
    }

    void attach(ChannelReader<T,FRAME> &&inChannel) {
//        std::cout << "Attaching ChannelReader to " << ptr << std::endl;
        ptr->attach(std::move(inChannel));
    }

    template<class TARGETT>
    ChannelWriter<TARGETT,FRAME> attach(UnattachedChannelWriter<TARGETT,FRAME> &&unattachedChannel) {
//        std::cout << "Attaching ChannelWriter to " << ptr << std::endl;
        return std::move(unattachedChannel).attachSource(*ptr);
    }
    
    FRAME &frame() { return ptr->frameOfReference; }

    const FRAME::SpaceTime &position() const { return ptr->position; }

    friend std::ostream &operator <<(std::ostream &out, const SpaceTimePtr<T,FRAME> &obj) {
        out << obj.ptr;
        return out;
    }
};

#endif
