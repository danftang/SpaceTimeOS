#ifndef LABORATORY_H
#define LABORATORY_H


#include "SpaceTimePtr.h"
#include "ThreadPool.h"
#include "Simulation.h"


// Dummy object for a Laboratory
class Lab {};

// A Laboratory is a spatial object in the default reference frame, at the default position
// However, a laboratory provides initiating methods that do not respect limitations on the
// maximum velocity of the flow of information.
template<SpaceTime SPACETIME, class EXECUTOR = ThreadPool<0>>
class Laboratory {
public:
    typedef SPACETIME                   SpaceTime;
    typedef Laboratory<SPACETIME,EXECUTOR>  Simulation;


    static inline SpaceTimeBase<SpaceTime>  callbacks = SpaceTimeBase<SpaceTime>(SpaceTime());
    static inline EXECUTOR                  executor;
    static inline std::function<bool(const SpaceTime &)>   isInBounds;

//    Laboratory(SIM frame = SIM()) : SpaceTimePtr<Lab,SIM>(new SpaceTimeObject<Lab,SIM>(typename SIM::SpaceTime(), frame)) {}

    // ~Laboratory() {
    //     this->kill();
    // }
protected:
    template<class T>
    static void submit(T &&runnable) { executor.submit(std::forward<T>(runnable)); }
public:

    template<class T>
    static void step(SpaceTimeObject<T,Simulation> &obj) { 
        if(isInBounds(obj.position)) {
            executor.submit([&obj]() {
                obj.step();
            });
        }
    }

    static void simulateUntil(SpaceTime::ScalarType endTime) {
        isInBounds = [endTime](const SpaceTime &position) {
            return position[0] < endTime;
        };
        callbacks.execCallbacks();
        executor.join();
    }

    template<class NEWTYPE, class... ARGS>
    static SpaceTimePtr<NEWTYPE,Simulation> spawnAt(const SpaceTime &initPosition, ARGS &&...args) {
        auto *pTarget = new SpaceTimeObject<NEWTYPE,Simulation>(initPosition, SpaceTime(1), std::forward<ARGS>(args)...);
        callbacks.callbackOnMove([pTarget]() {
            Simulation::step(*pTarget);
        });
        return SpaceTimePtr<NEWTYPE,Simulation>{ pTarget };
    }

    template<class NEWTYPE, class... ARGS>
    static SpaceTimePtr<NEWTYPE,Simulation> spawnAt(SpaceTime &&initPosition, ARGS &&...args) {
        auto *pTarget = new SpaceTimeObject<NEWTYPE,Simulation>(std::move(initPosition), SpaceTime(1), std::forward<ARGS>(args)...);
        callbacks.callbackOnMove([pTarget]() { 
            Simulation::step(*pTarget); 
        });
        return SpaceTimePtr<NEWTYPE,Simulation>{ pTarget };
    }


    // void simulateFor(FRAME::SpaceTime::Scalar seconds) {
    //     this->ptr->position = this->frame().positionAfter(this->position(), seconds);
    //     this->ptr->execCallbacks();
    // }


};

#endif
