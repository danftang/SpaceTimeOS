#ifndef INNERPRODFIELD_H
#define INNERPRODFIELD_H

#include "Concepts.h"

// A spacetime with an inner product defines a second order scalar field over the spacetime
// given by F(X) = X.X
// This in turn defines a family of fields of the form:
// F(X) = (X-X0).(X-X0) - c;
// for some vector X0 and constant c.
// This covers all second order fields, whose second order differential is the same as X.X
// F(X) = X.X - 2X0.X + X0.X0 - c [assuming X.Y = Y.X]
// Topologiaclly (up to affine transformations), it covers all second order fields with the same
// signature as the inner product (i.e. we can always reduce the second order term to a
// diagonal in [-1,1] by choosing an appropriate basis).
// 
// If X0.X0 > 0 and c > 0 then the field is contained within X.X.
//
// What's more, we can calculate the exact set-convolution of fields F and G by
// X0_H = X0_F + X0_G and c_H = c_F + c_G
//
// We define set convolution as H(X)>0 iff there exists a Y such that F(Y)>0 and G(X-Y)>0
// A widening would be just one way: if there exists a Y such that F(Y)>0 and G(X-Y)>0 then H(X) > 0
//
// Surely if (Y-Y0_F).(Y-Y0_F) - c_F > 0 and (X-Y-Y0_G).(X-Y-Y0_G) - c_G > 0
// then
// (Y-Y0_F).(Y-Y0_F) - c_F + (X-Y-Y0_G).(X-Y-Y0_G) - c_G > 0
// by linearity of inner prod
// (X - (Y0_F + Y0_G)).(X-(Y0_F + Y0_G)) - (c_F + c_G) > 0
// so it is at least a widining.
// Conversely, suppose H(X) = d for some d>0 then set Y to any point on the manifold F(Y) = d/2 so that
// (Y-Y0_F).(Y-Y0_F) - c_F = d/2
// and
// (X - (Y0_F + Y0_G)).(X-(Y0_F + Y0_G)) - (c_F + c_G) = d
// so
// (X - (Y0_F + Y0_G)).(X-(Y0_F + Y0_G)) - (Y-Y0_F).(Y-Y0_F) -  c_G = d/2
// so
// ((X-Y) - Y0_G).((X-Y) - Y0_G) - c_G = d/2
// so G(X-Y) > 0 and we have an exact set convolution.
//
// If we also allow the origin of the field to be at some parametric position o, then we
// have x0 <- x-o 
template<SpaceTime SPACETIME, SPACETIME::Time Offset = 0>
class InnerProdField {
public:
    SPACETIME       origin;

    auto operator()(const SPACETIME &x) {
        auto displacement = x-origin;
        return displacement * displacement - Offset;
    }

    auto d_dt(const SPACETIME &x, const SPACETIME &velocity) {
        return 2*((x-origin)*velocity);
    }

    auto d2_dt2(const SPACETIME &velocity) {
        return velocity * velocity;
    }
};

#endif
