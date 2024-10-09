#ifndef LABTIMEBOUNDARY_H
#define LABTIMEBOUNDARY_H

// Represents a boundary at a constant time in the laboratory frame
// LAMBDA should be a lambda function which takes an agent reference as argument.
template<class SPACETIME, class LAMBDA>
class LabTimeBoundary {
public:
    typedef SPACETIME         SpaceTime;
    typedef SPACETIME::Time   Time;

protected:
    Time          boundaryTime = 0;   // laboratory time of this boundary
    LAMBDA        lambda;
public:

    LabTimeBoundary() = default;

    LabTimeBoundary(LAMBDA lambda) : lambda(lambda) { }
    
    // Define a first order field that is zero along labTime = boundaryTime
    Time operator ()(const SPACETIME &pos) const { return pos.labTime() - boundaryTime; }
    Time d_dt(const SPACETIME &velocity) const { return velocity.labTime(); }

    const Time &getTime() const { return boundaryTime; }

    void setTime(Time newTime) {
        boundaryTime = newTime;
//        this->callbacks.execAll();
    }

    // Agent will call this when it reaches the boundary
    template<class T> //requires std::invocable<LAMBDA, T>
    void execute(T &agent) {
        lambda(agent);
    }
};

template<class SPACETIME, class LAMBDA> LabTimeBoundary(LAMBDA lambda) -> LabTimeBoundary<SPACETIME,LAMBDA>;

#endif
