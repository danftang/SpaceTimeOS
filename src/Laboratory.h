
#include "SpaceTimePtr.h"

class Lab {};

template<class FRAME>
class Laboratory : public SpaceTimePtr<Lab,FRAME> {
public:
    Laboratory(FRAME frame) : SpaceTimePtr<Lab,FRAME>(new SpaceTimeObject<Lab,FRAME>(frame)) {}
    ~Laboratory() {
        this->kill();
    }
};
