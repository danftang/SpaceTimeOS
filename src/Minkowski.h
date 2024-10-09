#ifndef MINKOWSKI_H
#define MINKOWSKI_H

#include <cassert>
#include <valarray>
#include <array>
#include <limits>
#include <ostream>

// A D-dimensional Minkowski spacetime is one whose inner product is defined as
// x.y = x_0.y_0 - sum_{i=1}^D x_dy_d
//
// In the absence of gravity, we live in a 4D Minkowski spacetime._
template<uint DIMENSIONS, class SCALAR = double>
class Minkowski : public std::array<SCALAR,DIMENSIONS> {
public:
    typedef SCALAR                          Time;
    typedef Minkowski<DIMENSIONS,SCALAR>    Velocity;
    static constexpr uint Dimensions = DIMENSIONS;

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
        while(i < DIMENSIONS) (*this)[i++] = 0;
    }

    static constexpr uint size() { return DIMENSIONS; }

    // The ordering of a Minkowski space is given by x < y if 
    // y is inside the future light cone of x.
    bool operator <(const Minkowski<DIMENSIONS,SCALAR> &other) {
        Minkowski<DIMENSIONS,SCALAR> displacement = other - *this;
        return displacement*displacement >= 0 && displacement[0] > 0;
    }

    const Time &labTime() const { return (*this)[0]; }

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
    Minkowski<DIMENSIONS,SCALAR> operator *(SCALAR properTime) const {
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

    Minkowski<DIMENSIONS,SCALAR> &operator *=(SCALAR scale) {
        for(int i=0; i<DIMENSIONS; ++i) (*this)[i] *= scale;
        return *this;
    }

    Minkowski<DIMENSIONS,SCALAR> &operator /=(SCALAR scale) {
        for(int i=0; i<DIMENSIONS; ++i) (*this)[i] /= scale;
        return *this;
    }

    static inline const Minkowski<DIMENSIONS,SCALAR> TOP = Minkowski<DIMENSIONS,SCALAR>({ std::numeric_limits<SCALAR>::infinity() }); // NB: If we dont have infinity we have to make sure there's no overflow somehow
    static inline const Minkowski<DIMENSIONS,SCALAR> BOTTOM = Minkowski<DIMENSIONS,SCALAR>({ -std::numeric_limits<SCALAR>::infinity() });

    friend std::ostream &operator <<(std::ostream &out, const Minkowski<DIMENSIONS,SCALAR> &pos) {
        out << "(";
        for(int i=0; i<DIMENSIONS; ++i) out << pos[i] << " ";
        out << ")";
        return out;
    }

    friend Minkowski<DIMENSIONS,SCALAR> operator *(SCALAR properTime, const Minkowski<DIMENSIONS,SCALAR> &velocity) {
        return velocity*properTime;
    }


};

typedef Minkowski<1> GlobalTime; // 1-D Minkowski is the same as global time




// t = S/V iff |tV - S| = 0
// where t is a scalar.
// i.e. if V is a velocity and S is a displacement, then S/V is the
// time it would take in a reference framd moving with V to reach zero
// distance to S, starting at the origin. 
// For a suitably defined dot product, this is equivalent to
// (tV - S).(tV - S) = 0
// So, given that V.S = S.V 
// V.Vt^2 - 2V.St + S.S = 0
// N.B. if we assume |V| = 1 then V.V = 1
// TODO: we can generalise this even further by solving |tV - S| = c. This corresponds to a channel that has a constant distance between emission and absorbtion
// template<uint DIM, class SCALAR>
// SCALAR operator /(const Minkowski<DIM,SCALAR> &displacement, const Minkowski<DIM,SCALAR> &velocity) {
// //    SCALAR a = velocity * velocity;
//     assert(fabs(velocity*velocity - 1) < 1e-6); // |v| should be 1
//     SCALAR mb = velocity * displacement; // -b/2
//     SCALAR c = displacement * displacement;

//     return mb + sqrt(mb*mb - c); // quadratic formula with a=1 second^2
// }

// template<class SCALAR>
// SCALAR operator /(const Minkowski<1,SCALAR> &displacement, const Minkowski<1,SCALAR> &velocity) {
//     assert(fabs(velocity*velocity - 1) < 1e-6); // |v| should be 1
//     return displacement[0]; // in 1D case, the square root of the quadratic identically goes to zero, so we don't need to calculate it 
// }


#endif
