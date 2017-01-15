/**
  *  \file util/math.cpp
  */

#include <cmath>
#include "util/math.hpp"

/** Pi. This is what "echo 'a(1)*4' | bc -l" says. */
const double util::PI = 3.14159265358979323844;


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

double
util::getHeadingDeg(double dx, double dy)
{
    return getHeadingRad(dx, dy) * (180 / PI);
}


double
util::getDistanceFromDX(int dx, int dy)
{
    // ex game/formula.h:distFromDX
    return std::sqrt(double(dx*dx + dy*dy));
}
