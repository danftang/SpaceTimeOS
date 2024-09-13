#ifndef MINKOWSKI_H
#define MINKOWSKI_H

#include <valarray>
#include <array>
#include <limits>
#include <ostream>

template<uint DIMENSIONS, class SCALAR = double>
class Minkowski : public std::array<SCALAR,DIMENSIONS> {
public:
    typedef SCALAR Scalar;

    // Default gives the reference origin 
    Minkowski() {
        for(int i=0; i<DIMENSIONS; ++i) (*this)[i] = 0;
    }

    // Constructing with a time gives the laboratory origin at a given time
    // Constructing with 1 gives the laboratory 4-velocity
    Minkowski(SCALAR laboratoryTime) {
        (*this)[0] = laboratoryTime;
        for(int i=1; i<DIMENSIONS; ++i) (*this)[i] = 0;
    }

    Minkowski(const std::initializer_list<SCALAR> &init) {
        int i=0;
        for(const SCALAR &item : init) (*this)[i++] = item;
    }


    // bool operator <(const Minkowski<DIMENSIONS,SCALAR> &other) {
    //     SCALAR sum = 0;
    //     for(int d=1; d<DIMENSIONS; ++d) {
    //         SCALAR delta =  other[d] - (*this)[d];
    //         sum += delta*delta;
    //     }
    //     SCALAR deltat = other[0] - (*this)[0];
    //     return deltat >=0 && sum < deltat*deltat; 
    // }

    Minkowski<DIMENSIONS,SCALAR> operator -(const Minkowski<DIMENSIONS,SCALAR> &other) const {
        Minkowski<DIMENSIONS,SCALAR> result;
        for(int i=0; i<DIMENSIONS; ++i) result[i] = (*this)[i] - other[i];
        return result;
    }


    Minkowski<DIMENSIONS,SCALAR> operator +(const Minkowski<DIMENSIONS,SCALAR> &other) const {
        Minkowski<DIMENSIONS,SCALAR> result;
        for(int i=0; i<DIMENSIONS; ++i) result[i] = (*this)[i] + other[i];
        return result;
    }


    // If this is a 4-velocity, then this returns the displacement of a clock moving at this velocity
    // after it experiences properTime
    Minkowski<DIMENSIONS,SCALAR> operator *(const SCALAR &properTime) const {
        Minkowski<DIMENSIONS,SCALAR> result;
        for(int i=0; i<DIMENSIONS; ++i) result[i] = (*this)[i]*properTime;
        return result;
    }

    // inner product A*B = A0B0 - A1B1 - A2B2 ...
    SCALAR operator *(const Minkowski<DIMENSIONS,SCALAR> &other) const {
        SCALAR innerProd = (*this)[0]*other[0];
        for(int i=1; i<DIMENSIONS; ++i) innerProd -= (*this)[i]*other[i];
        return innerProd;
    }


    Minkowski<DIMENSIONS,SCALAR> &operator +=(const Minkowski<DIMENSIONS,SCALAR> &other) {
        for(int i=0; i<DIMENSIONS; ++i) (*this)[i] += other[i];
        return *this;
    }


    static const Minkowski<DIMENSIONS,SCALAR> top() {
        Minkowski<DIMENSIONS,SCALAR> t;
        t[0] = std::numeric_limits<SCALAR>::infinity();
        return t;
    } 

    static inline const Minkowski<DIMENSIONS,SCALAR> TOP = top();

    friend std::ostream &operator <<(std::ostream &out, const Minkowski<DIMENSIONS,SCALAR> &pos) {
        out << "(";
        for(int i=0; i<DIMENSIONS; ++i) out << pos[i] << " ";
        out << ")";
        return out;
    }
};


template<int DIM, class SCALAR>
Minkowski<DIM,SCALAR> operator *(const SCALAR &properTime, const Minkowski<DIM,SCALAR> &velocity) {
    return velocity*properTime;
}


// t = B/A iff |tA - B| = 0
// where t is a scalar.
// Equivalently
// (tA - B).(tA - B) = 0
// So, given that A.B = B.A 
// A.At^2 - 2A.Bt + B.B = 0
template<int DIM, class SCALAR>
SCALAR operator /(const Minkowski<DIM,SCALAR> &displacement, const Minkowski<DIM,SCALAR> &velocity) {
    SCALAR a = velocity * velocity;
    SCALAR mb = 2 * velocity * displacement; // minus b
    SCALAR c = displacement * displacement;

    return (mb + sqrt(mb*mb - 4*a*c))/(2*a);
}

// In the 2 dimensional case we have 
// t = B/A iff |tA - B| = 0
// which is true if
// tA0 - B0 = tA1 - B1
// so t = (B0 - B1)/(A0 - A1);
template<class SCALAR>
SCALAR operator /(const Minkowski<2,SCALAR> &displacement, const Minkowski<2,SCALAR> &velocity) {
    return (displacement[0] - displacement[1])/(velocity[0] - velocity[1]);
}



#endif
