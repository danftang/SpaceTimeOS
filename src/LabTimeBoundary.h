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
    Time operator ()(const SPACETIME &pos) const { return static_cast<Time>(pos) - boundaryTime; }
    Time d_dt(const SPACETIME &velocity) const { return static_cast<Time>(velocity); }

    const Time &getTime() const { return boundaryTime; }

    void setTime(Time newTime) {
        boundaryTime = newTime;
    }
};


#endif
