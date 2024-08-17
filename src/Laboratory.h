
#include "SpaceTimePtr.h"
#include "ThreadPool.h"
#include "spacetime/ReferenceFrame.h"

class Lab {};

template<ReferenceFrame FRAME>
class Laboratory : public SpaceTimePtr<Lab,FRAME> {
public:
    Laboratory(FRAME frame = FRAME()) : SpaceTimePtr<Lab,FRAME>(new SpaceTimeObject<Lab,FRAME>(typename FRAME::SpaceTime(), frame)) {}
    ~Laboratory() {
        this->kill();
    }

    void simulateFor(double seconds) {
        this->ptr->position = this->frame().positionAfter(this->position(), seconds);
        this->ptr->execCallbacks();
    }


};
