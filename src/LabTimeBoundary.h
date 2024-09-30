#ifndef LABTIMEBOUNDARY_H
#define LABTIMEBOUNDARY_H

// Represents a boundary at a constant time in the laboratory frame
// LAMBDA should be a lambda function which takes an agent reference as argument.
template<class SPACETIME, class LAMBDA>
class LabTimeBoundary {
public:
    typedef SPACETIME           SpaceTime;
    typedef SPACETIME::Scalar   Scalar;

protected:
    Scalar          time = 0;   // laboratory time of this boundary
    LAMBDA          lambda;
public:

    LabTimeBoundary() = default;

    LabTimeBoundary(LAMBDA lambda) : lambda(lambda) { }
    
    Scalar timeToIntersection(const SpaceTime &position, const SpaceTime &velocity) {
        return (time - position[0])/velocity[0];
    }

    void setTime(Scalar newTime) {
        time = newTime;
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
