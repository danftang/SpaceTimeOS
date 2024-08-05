#include <functional>

template<class T, class SPACETIME>
class SpatialFunction {
    public:
    SPACETIME                       position;   // position on emission
    std::function<SPACETIME(T &)>   event;       // returns new velocity of agent

    SpatialFunction(SPACETIME position, std::function<SPACETIME(T &)> event) : position(std::move(position)), event(std::move(event)) {}
    SpatialFunction(SPACETIME position) : position(std::move(position)) {} // create a blocking entry

    bool isBlocking() { return !((bool)event); }
};
