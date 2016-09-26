/**
  *  \file util/math.hpp
  */
#ifndef C2NG_UTIL_MATH_HPP
#define C2NG_UTIL_MATH_HPP

#include "afl/base/types.hpp"

#undef PI

namespace util {

    extern const double PI;

    /** Divide a/b, and round result arithmetically.
        Uses integer arithmetic only.
        \pre a >= 0, b > 0 */
    inline int32_t divideAndRound(int32_t a, int32_t b)
    {
        // ex game/formula.h:radiv
        return (a + (b/2)) / b;
    }


    /** Compute a/b+plus using IEEE rounding.
        The result is rounded to the nearest integer.
        If the fractional part is exactly 0.5, rounds to the nearest even integer.
        Uses only integer math.
        This is hard to implement efficiently in C++ with floating point math, anyway.
        \pre a >= 0, b > 0

        \change PCC2 had an overload on the first parameter being int/long (meaning 16/32 bit).
        We force it to be int32_t. */
    inline int32_t divideAndRoundToEven(int32_t a, int b, int plus)
    {
        // ex game/formula.h:rdivaddl,rdivaddw
        int32_t x = a / b + plus;
        int r = a % b;
        if (r*2 + (x&1) > b) {
            ++x;
        }
        return x;
    }

    /** Given coordinate offsets, compute heading.
        \pre dx!=0 || dy!=0
        \returns heading in radians, [0, 2*PI] */
    double getHeadingRad(double dx, double dy);

    /** Get heading in degrees. */
    double getHeadingDeg(double dx, double dy);
    
    inline int32_t squareInteger(int32_t x)
    {
        return x*x;
    }

}

#endif
