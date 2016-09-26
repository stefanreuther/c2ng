/**
  *  \file u/t_util_math.cpp
  *  \brief Test for util::Math
  */

#include "util/math.hpp"

#include "t_util.hpp"

/** Test divideAndRound. */
void
TestUtilMath::testDivideAndRound()
{
    // Divide by 1
    TS_ASSERT_EQUALS(util::divideAndRound(0, 1), 0);
    TS_ASSERT_EQUALS(util::divideAndRound(1, 1), 1);
    TS_ASSERT_EQUALS(util::divideAndRound(2, 1), 2);
    TS_ASSERT_EQUALS(util::divideAndRound(3, 1), 3);

    // Divide by 2
    TS_ASSERT_EQUALS(util::divideAndRound(0, 2), 0);
    TS_ASSERT_EQUALS(util::divideAndRound(1, 2), 1);
    TS_ASSERT_EQUALS(util::divideAndRound(2, 2), 1);
    TS_ASSERT_EQUALS(util::divideAndRound(3, 2), 2);
    TS_ASSERT_EQUALS(util::divideAndRound(4, 2), 2);

    // Divide by 3
    TS_ASSERT_EQUALS(util::divideAndRound(0, 3), 0);
    TS_ASSERT_EQUALS(util::divideAndRound(1, 3), 0);
    TS_ASSERT_EQUALS(util::divideAndRound(2, 3), 1);
    TS_ASSERT_EQUALS(util::divideAndRound(3, 3), 1);
    TS_ASSERT_EQUALS(util::divideAndRound(4, 3), 1);
    TS_ASSERT_EQUALS(util::divideAndRound(5, 3), 2);
    TS_ASSERT_EQUALS(util::divideAndRound(6, 3), 2);
}

/** Test divideAndRoundToEven. */
void
TestUtilMath::testDivideAndRoundToEven()
{
    // Trivial cases: divide by 1
    TS_ASSERT_EQUALS(util::divideAndRoundToEven(0, 1, 0), 0);
    TS_ASSERT_EQUALS(util::divideAndRoundToEven(1, 1, 0), 1);
    TS_ASSERT_EQUALS(util::divideAndRoundToEven(2, 1, 0), 2);
    TS_ASSERT_EQUALS(util::divideAndRoundToEven(3, 1, 0), 3);
    TS_ASSERT_EQUALS(util::divideAndRoundToEven(0, 1, 1), 1);
    TS_ASSERT_EQUALS(util::divideAndRoundToEven(0, 1, 2), 2);
    TS_ASSERT_EQUALS(util::divideAndRoundToEven(0, 1, 3), 3);

    // Divide by 2
    TS_ASSERT_EQUALS(util::divideAndRoundToEven(0, 2, 0), 0);
    TS_ASSERT_EQUALS(util::divideAndRoundToEven(1, 2, 0), 0);
    TS_ASSERT_EQUALS(util::divideAndRoundToEven(2, 2, 0), 1);
    TS_ASSERT_EQUALS(util::divideAndRoundToEven(3, 2, 0), 2);
    TS_ASSERT_EQUALS(util::divideAndRoundToEven(4, 2, 0), 2);

    TS_ASSERT_EQUALS(util::divideAndRoundToEven(0, 2, 1), 1);
    TS_ASSERT_EQUALS(util::divideAndRoundToEven(1, 2, 1), 2);
    TS_ASSERT_EQUALS(util::divideAndRoundToEven(2, 2, 1), 2);
    TS_ASSERT_EQUALS(util::divideAndRoundToEven(3, 2, 1), 2);
    TS_ASSERT_EQUALS(util::divideAndRoundToEven(4, 2, 1), 3);

    TS_ASSERT_EQUALS(util::divideAndRoundToEven(0, 2, -1), -1);
    TS_ASSERT_EQUALS(util::divideAndRoundToEven(1, 2, -1), 0);
    TS_ASSERT_EQUALS(util::divideAndRoundToEven(2, 2, -1), 0);
    TS_ASSERT_EQUALS(util::divideAndRoundToEven(3, 2, -1), 0);
    TS_ASSERT_EQUALS(util::divideAndRoundToEven(4, 2, -1), 1);
}

/** Test getHeadingRad/getHeadingDeg. */
void
TestUtilMath::testGetHeading()
{
    // Degrees
    TS_ASSERT_EQUALS(util::getHeadingDeg(1, 0), 90);
    TS_ASSERT_EQUALS(util::getHeadingDeg(1, 1), 45);
    TS_ASSERT_EQUALS(util::getHeadingDeg(0, 1), 0);
    TS_ASSERT_EQUALS(util::getHeadingDeg(-1, 1), 315);
    TS_ASSERT_EQUALS(util::getHeadingDeg(-1, 0), 270);
    TS_ASSERT_EQUALS(util::getHeadingDeg(-1, -1), 225);
    TS_ASSERT_EQUALS(util::getHeadingDeg(0, -1), 180);
    TS_ASSERT_EQUALS(util::getHeadingDeg(1, -1), 135);

    // Radians
    TS_ASSERT_EQUALS(util::getHeadingRad(1, 0), util::PI/2);
    TS_ASSERT_EQUALS(util::getHeadingRad(1, 1), util::PI/4);
    TS_ASSERT_EQUALS(util::getHeadingRad(0, 1), 0.0);
    TS_ASSERT_EQUALS(util::getHeadingRad(-1, 1), util::PI*7/4);
    TS_ASSERT_EQUALS(util::getHeadingRad(-1, 0), util::PI*3/2);
    TS_ASSERT_EQUALS(util::getHeadingRad(-1, -1), util::PI*5/4);
    TS_ASSERT_EQUALS(util::getHeadingRad(0, -1), util::PI);
    TS_ASSERT_EQUALS(util::getHeadingRad(1, -1), util::PI*3/4);

    // pi
    TS_ASSERT(util::PI >= 3.141592);
    TS_ASSERT(util::PI <= 3.141593);
}

/** Test squareInteger. */
void
TestUtilMath::testSquareInteger()
{
    // Possible signatures
    TS_ASSERT_EQUALS(util::squareInteger(short(10)), 100);
    TS_ASSERT_EQUALS(util::squareInteger(int(10)), 100);
    TS_ASSERT_EQUALS(util::squareInteger(int16_t(10)), 100);
    TS_ASSERT_EQUALS(util::squareInteger(int32_t(10)), 100);

    // Values
    TS_ASSERT_EQUALS(util::squareInteger(20000), 400000000);
    TS_ASSERT_EQUALS(util::squareInteger(42), 1764);
    TS_ASSERT_EQUALS(util::squareInteger(1), 1);
    TS_ASSERT_EQUALS(util::squareInteger(0), 0);
    TS_ASSERT_EQUALS(util::squareInteger(-1), 1);
    TS_ASSERT_EQUALS(util::squareInteger(-42), 1764);
    TS_ASSERT_EQUALS(util::squareInteger(-20000), 400000000);
}
