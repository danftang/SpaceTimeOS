#ifndef SPACETIMEPTR_H
#define SPACETIMEPTR_H

#include <vector>
#include "SpaceTimeObject.h"
#include "Simulation.h"


// A smart pointer for use by a spatial funciton, to run on intersection with a spatial object.
template<class T, Simulation SIM>
class SpaceTimePtr {
protected:
    SpaceTimeObject<T,SIM> *ptr;

    // Protected constructor as we don't want the user passing these through Channels
    // A piece of code should only have access to SpaceTimePtrs which point to objects
    // that are currently (computation-time) at the same space-time position.
    SpaceTimePtr(const SpaceTimePtr<T,SIM> &other) = default;
    explicit SpaceTimePtr(SpaceTimeObject<T,SIM> *ptr): ptr(ptr) { }

public:
    friend class SpatialFunction<T,SIM>; // allow access to constructor to send to SpatialFunction
    template<class OTHER, Simulation OTHERSIM> friend class SpaceTimePtr;
    friend SIM;


    SpaceTimePtr(SpaceTimePtr<T,SIM> &&other) : ptr(other.ptr) {
        other.ptr = nullptr;
    }

    T *operator ->() { return &ptr->object; }

    // Spawn a new SpaceTimeObject in the same position and reference frame as this object
    // and create a channel to it, connected to this. []
    template<class NEWTYPE, class... ARGS>
    ChannelWriter<NEWTYPE,SIM> spawn(ARGS &&...args) {
        auto *pTarget = new SpaceTimeObject<NEWTYPE,SIM>(position(), velocity(), std::forward<ARGS>(args)...);
        ChannelWriter<NEWTYPE,SIM> writer{*ptr, *pTarget};
        pTarget->step(); // make sure new object is blocking on this
        return std::move(writer);
    }

    template<class TARGETT>
    ChannelWriter<TARGETT,SIM> openChannelTo(const ChannelWriter<TARGETT,SIM> &target) {
        return ChannelWriter(*ptr, target);
    }

    template<class TARGETT>
    ChannelWriter<TARGETT,SIM> openChannelTo(SpaceTimePtr<TARGETT,SIM> &target) {
        return ChannelWriter(*ptr, *target.ptr);
    }


    // kill the referent
    void kill() {
        delete(ptr);
        ptr = nullptr;
    }

    void attach(ChannelReader<T,SIM> &&inChannel) {
//        std::cout << "Attaching ChannelReader to " << ptr << std::endl;
        ptr->attach(std::move(inChannel));
    }

    template<class TARGETT>
    ChannelWriter<TARGETT,SIM> attach(UnattachedChannelWriter<TARGETT,SIM> &&unattachedChannel) {
//        std::cout << "Attaching ChannelWriter to " << ptr << std::endl;
        return std::move(unattachedChannel).attachSource(*ptr);
    }

    
    SIM::SpaceTime &velocity() { return ptr->velocity; }

    const SIM::SpaceTime &position() const { return ptr->position; }

    friend std::ostream &operator <<(std::ostream &out, const SpaceTimePtr<T,SIM> &obj) {
        out << obj.ptr;
        return out;
    }
};

#endif
