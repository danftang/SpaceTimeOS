
template<class SPACETIME>
class InertialReferenceFrame {
public:
    SPACETIME   origin;
    SPACETIME   velocity; // 4-velocity unit vector

    SPACETIME intersection(SPACETIME &callPosition) {
        // |origin + time*velocity - callPosition| = 0
        double time = (origin - callPosition) / velocity;
        return origin + time*velocity;
    }
};