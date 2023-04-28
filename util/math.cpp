/**
  *  \file util/math.cpp
  *  \brief Mathematical Functions
  */

#include <cmath>
#include "util/math.hpp"

// Pi. This is what "echo 'a(1)*4' | bc -l" says.
const double util::PI = 3.14159265358979323844;

// Arithmetic rounding.
int32_t
util::roundToInt(double d)
{
    // ex game/formula.h:roundToInt
    if (d < 0) {
        return -int32_t(-d + 0.5);
    } else {
        return int32_t(d + 0.5);
    }
}

// Given coordinate offsets, compute heading.
double
util::getHeadingRad(double dx, double dy)
{
    // atan2 returns [-PI, PI], says C99. We want it to return [0, 2*PI].
    // Yes, this ought to be dx,dy, to get the angles in the way VGAP uses them.
    double value = std::atan2(dx, dy);
    if (value < 0) {
        value += 2*PI;
    }
    return value;
}

// Given coordinate offsets, compute heading in degrees.
double
util::getHeadingDeg(double dx, double dy)
{
    return getHeadingRad(dx, dy) * (180 / PI);
}

// Given coordinate offsets, compute squared distance.
int32_t
util::getDistance2FromDX(int dx, int dy)
{
    // ex game/formula.h:dist2FromDX
    return dx*dx + dy*dy;
}

// Given coordinate offsets, compute distance.
double
util::getDistanceFromDX(int dx, int dy)
{
    // ex game/formula.h:distFromDX
    return std::sqrt(double(getDistance2FromDX(dx, dy)));
}
