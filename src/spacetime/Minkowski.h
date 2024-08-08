#include <valarray>
#include <array>

namespace spacetime {


    template<int DIMENSIONS>
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

        bool operator <(const Minkowski<DIMENSIONS> &other) {
            double sum = 0.0;
            for(int d=1; d<DIMENSIONS; ++d) {
                double delta =  other.v[d] - v[d];
                sum += delta*delta;
            }
            double deltat = other.v[0] - v[0];
            return deltat >=0 && sum < deltat*deltat; 
        }


    };

    
}