#ifndef LABTIMEBOUNDARY_H
#define LABTIMEBOUNDARY_H

// Represents a boundary at a constant time in the laboratory frame
// LAMBDA should be a lambda function which takes an agent reference as argument.
template<class SPACETIME>
class LabTimeBoundary {
public:
    typedef SPACETIME         SpaceTime;
    typedef SPACETIME::Time   Time;

protected:
    Time          boundaryTime = 0;   // laboratory time of this boundary
public:


    // Define a first order field that is zero along labTime = boundaryTime
    Time operator ()(const SPACETIME &pos) const { return pos.labTime() - boundaryTime; }
    Time d_dt(const SPACETIME &velocity) const { return velocity.labTime(); }

    const Time &getTime() const { return boundaryTime; }

    void setTime(Time newTime) {
        boundaryTime = newTime;
    }
};


#endif
