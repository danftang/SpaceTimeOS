
template<class SPACETIME>
class SpatialFunction {
    public:
    SPACETIME                       position;   // position on emission
    std::function<SPACETIME(T &)>   event;       // returns new velocity of agent
};
