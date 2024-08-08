
namespace spacetime {

    template<class SPACETIME>
    class IntertialFrame {
    public:
        typedef SPACETIME SpaceTime;

        SPACETIME   velocity;

        // default to the laboratory reference frame
        IntertialFrame() {}

        IntertialFrame(SPACETIME velocity) : velocity(std::move(velocity)) {}

        SpaceTime intersection(SpaceTime &callPosition, SpaceTime &origin) {
            double time = (origin - callPosition) / velocity;
            return origin + time*velocity;
        }

        // void setPosition(const SpaceTime &pos) {
        //     origin = pos;
        // }

        

    };

    
    
};
