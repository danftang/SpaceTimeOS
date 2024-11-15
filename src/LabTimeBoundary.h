#ifndef LABTIMEBOUNDARY_H
#define LABTIMEBOUNDARY_H

#include "LinearField.h"

// Represents a boundary at a constant time in the laboratory frame
// LAMBDA should be a lambda function which takes an agent reference as argument.
template<class SPACETIME, typename SPACETIME::Time EndTime>
class LabTimeBoundary : public LinearField<SPACETIME, EndTime> {
public:
    typedef SPACETIME  SpaceTime;

    LabTimeBoundary() : LinearField<SpaceTime,EndTime>(Velocity<SpaceTime>()) { }

};


#endif
