#ifndef SPACETIMEPTR_H
#define SPACETIMEPTR_H

#include <vector>
#include "SpaceTimeObject.h"

template<class T, ReferenceFrame FRAME> class ChannelRef;

// A smart pointer for use by a spatial funciton, to run on intersection with a spatial object.
template<class T, ReferenceFrame FRAME>
class SpaceTimePtr {
protected:
    SpaceTimeObject<T,FRAME> *ptr;
public:
    template<class T2, ReferenceFrame FRAME2> friend class ChannelRef;

    SpaceTimePtr(SpaceTimeObject<T,FRAME> *ptr): ptr(ptr) { }

    T *operator ->() {
        return &ptr->object;
    }

    // Spawn a new SpaceTimeObject in the same reference frame
    template<class NEWT>
    SpaceTimePtr<NEWT, FRAME> spawn(NEWT &&newObject) {
        return new SpaceTimeObject<NEWT,FRAME>(std::move(newObject), frame());
    }

    // Spawn in a new frame.
    // If the frame is in the non-future, it will be created at the intersection of the newFrame with 
    // this objects current light-cone.
    template<class NEWT>
    SpaceTimePtr<NEWT, FRAME> spawn(NEWT &&newObject, FRAME newFrame) {
        auto createPosition = newFrame.intersection(frame().origin);
        return SpaceTimePtr(new SpaceTimeObject(std::move(newObject), std::move(newFrame)));
    }

    // create a new (closed) channel to the target
    ChannelRef<T,FRAME> newChannel() {
        return ptr->newChannel();
    }


    // kill the referent
    void kill() {
        delete(ptr);
        ptr = nullptr;
    }
    
    FRAME &frame() { return ptr->frameOfReference; }

};

#endif
