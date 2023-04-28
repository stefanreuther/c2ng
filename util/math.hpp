/**
  *  \file util/math.hpp
  *  \brief Mathematical Functions
  */
#ifndef C2NG_UTIL_MATH_HPP
#define C2NG_UTIL_MATH_HPP

#include "afl/base/types.hpp"

#undef PI

namespace util {

    /** Pi (3.141592...). */
    extern const double PI;

    /** Arithmetic rounding.
        Rounds towards nearest integer, except when number ends in .5 exactly
        where it rounds to next larger integer (where -10 is larger than -9).

        This function used to be called /round/ but it seems SUSv3 defines such a function, too. */
    int32_t roundToInt(double d);

    /** Square an integer.
        \param x Value
        \return x*x */
    int32_t squareInteger(int32_t x);

    /** Square a floating-point value.
        \param x Value
        \return x*x */
    double squareFloat(double x);

    /** Divide a/b, and round result arithmetically.
        Uses integer arithmetic only.
        \pre a >= 0, b > 0 */
    int32_t divideAndRound(int32_t a, int32_t b);

    /** Divide a/b, rounding up.
        Uses integer arithmetic only.
        \pre a >= 0, b > 0 */
    int32_t divideAndRoundUp(int32_t a, int32_t b);

    /** Compute a/b+plus using IEEE rounding.
        The result is rounded to the nearest integer.
        If the fractional part is exactly 0.5, rounds to the nearest even integer.
        Uses only integer math.
        This is hard to implement efficiently in C++ with floating point math, anyway.
        \pre a >= 0, b > 0

        \change PCC2 had an overload on the first parameter being int/long (meaning 16/32 bit).
        We force it to be int32_t. */
    int32_t divideAndRoundToEven(int32_t a, int b, int plus);

    /** Given coordinate offsets, compute heading in radians.
        \param dx X (east-west) offset, positive = east
        \param dy Y (north-south) offset, positive = north
        \pre dx!=0 || dy!=0
        \returns heading in radians, [0, 2*PI] */
    double getHeadingRad(double dx, double dy);

    /** Given coordinate offsets, compute heading in degrees.
        \param dx X (east-west) offset, positive = east
        \param dy Y (north-south) offset, positive = north
        \pre dx!=0 || dy!=0
        \returns heading in degrees, [0, 360] */
    double getHeadingDeg(double dx, double dy);

    /** Given coordinate offsets, compute squared distance.
        \param dx X (east-west) offset, positive = east
        \param dy Y (north-south) offset, positive = north
        \return distance */
    int32_t getDistance2FromDX(int dx, int dy);

    /** Given coordinate offsets, compute distance.
        \param dx X (east-west) offset, positive = east
        \param dy Y (north-south) offset, positive = north
        \return distance */
    double getDistanceFromDX(int dx, int dy);

    /** Three-way compare.
        \tparam T type to compare
        \param a First value
        \param b Second value
        \return -1 if a<b, +1 if a>b, 0 if a==b */
    template<typename T>
    int compare3(const T& a, const T& b);

}

inline int32_t
util::squareInteger(int32_t x)
{
    return x*x;
}

inline double
util::squareFloat(double x)
{
    return x*x;
}

inline int32_t
util::divideAndRound(int32_t a, int32_t b)
{
    // ex game/formula.h:radiv, ccvcr.pas:RDiv
    return (a + (b/2)) / b;
}

inline int32_t
util::divideAndRoundUp(int32_t a, int32_t b)
{
    return (a + (b-1)) / b;
}

inline int32_t
util::divideAndRoundToEven(int32_t a, int b, int plus)
{
    // ex game/formula.h:rdivaddl,rdivaddw, lowlevel.pas:RDiv, ccvcr.pas:RDivAdd, ccvcr.pas:RDivAddW
    int32_t x = a / b + plus;
    int r = a % b;
    if (r*2 + (x&1) > b) {
        ++x;
    }
    return x;
}

template<typename T>
inline int
util::compare3(const T& a, const T& b)
{
    return (a == b ? 0 : a < b ? -1 : +1);
}

#endif
