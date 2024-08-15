#include <valarray>
#include <array>

namespace spacetime {


    template<int DIMENSIONS, double MAX_TIME>
    class Minkowski {
    public:
        std::array<double,DIMENSIONS> v;

        // default to the origin of the laboratory
        Minkowski() {
            for(int i=0; i<DIMENSIONS; ++i) v[i] = 0.0;
        }


        Minkowski(std::initializer_list<double> initList) {
            // TODO:
        }

        bool operator <(const Minkowski<DIMENSIONS,MAX_TIME> &other) {
            double sum = 0.0;
            for(int d=1; d<DIMENSIONS; ++d) {
                double delta =  other.v[d] - v[d];
                sum += delta*delta;
            }
            double deltat = other.v[0] - v[0];
            return deltat >=0 && sum < deltat*deltat; 
        }

        Minkowski<DIMENSIONS,MAX_TIME> operator -(const Minkowski<DIMENSIONS,MAX_TIME> &other) const {
            Minkowski<DIMENSIONS,MAX_TIME> result;
            for(int i=0; i<DIMENSIONS; ++i) result.v[i] = v[i] - other.v[i];
            return result;
        }


        Minkowski<DIMENSIONS,MAX_TIME> operator +(const Minkowski<DIMENSIONS,MAX_TIME> &other) const {
            Minkowski<DIMENSIONS,MAX_TIME> result;
            for(int i=0; i<DIMENSIONS; ++i) result.v[i] = v[i] + other.v[i];
            return result;
        }


        Minkowski<DIMENSIONS,MAX_TIME> operator *(double time) const {
            Minkowski<DIMENSIONS,MAX_TIME> result;
            for(int i=0; i<DIMENSIONS; ++i) result.v[i] = v[i]*time;
            return result;
        }

        // t = B/A iff |tA - B| = 0
        // for which there are always two real solutions if the frame is moving slower than light-speed
        // the earlier solution is the past light cone so we ignore this and return the solution on the future light cone.
        // If this is in the agent's past, it must already be in the future light-cone.
        double operator /(const Minkowski<DIMENSIONS,MAX_TIME> &velocity) const {
            return *this / velocity;
        }

        bool isWithinBounds() {
            return v[0] < MAX_TIME;
        }

    };

    // t = B/A iff |tA - B| = 0
    // which is true if
    // (tA0 - B0)^2 - \sum_i=1^D (tAi - Bi)^2 = 0
    // so we have the quadratic
    // t^2(A0^2 - A1^2 - A2^2...) + 2t(A1B1 + A2B2 ... - A0B0) + (B0^2 - B1^2...) = 0
    template<int DIM, double MAX_TIME>
    Minkowski<DIM,MAX_TIME> operator /(const Minkowski<DIM,MAX_TIME> &position, const Minkowski<DIM,MAX_TIME> &velocity) {
        return std::numeric_limits<double>::signaling_NaN; // TODO: implement quadratic solve
    }

    // In the 2 dimensional case we have 
    // t = B/A iff |tA - B| = 0
    // which is true if
    // tA0 - B0 = tA1 - B1
    // so t = (B0 - B1)/(A0 - A1);
    template<double MAX_TIME>
    Minkowski<2,MAX_TIME> operator /(const Minkowski<2,MAX_TIME> &position, const Minkowski<2,MAX_TIME> &velocity) {
        return (position.v[0] - position.v[1])/(velocity.v[0] - velocity.v[1]);
    }


}