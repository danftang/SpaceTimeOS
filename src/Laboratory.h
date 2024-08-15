
#include "SpaceTimePtr.h"
#include "ThreadPool.h"

class Lab {};

template<class FRAME>
class Laboratory : public SpaceTimePtr<Lab,FRAME> {
public:
    Laboratory(FRAME frame = FRAME()) : SpaceTimePtr<Lab,FRAME>(new SpaceTimeObject<Lab,FRAME>(typename FRAME::SpaceTime(), frame)) {}
    ~Laboratory() {
        this->kill();
    }

    // void simulateFor(double seconds) {
    //     this->ptr->position = 
    // }
};
