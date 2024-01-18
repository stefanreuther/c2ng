/**
  *  \file test/util/mathtest.cpp
  *  \brief Test for util::Math
  */

#include "util/math.hpp"
#include "afl/test/testrunner.hpp"

/** Test divideAndRound. */
AFL_TEST("util.Math:divideAndRound", a)
{
    // Divide by 1
    a.checkEqual("01", util::divideAndRound(0, 1), 0);
    a.checkEqual("02", util::divideAndRound(1, 1), 1);
    a.checkEqual("03", util::divideAndRound(2, 1), 2);
    a.checkEqual("04", util::divideAndRound(3, 1), 3);

    // Divide by 2
    a.checkEqual("11", util::divideAndRound(0, 2), 0);
    a.checkEqual("12", util::divideAndRound(1, 2), 1);
    a.checkEqual("13", util::divideAndRound(2, 2), 1);
    a.checkEqual("14", util::divideAndRound(3, 2), 2);
    a.checkEqual("15", util::divideAndRound(4, 2), 2);

    // Divide by 3
    a.checkEqual("21", util::divideAndRound(0, 3), 0);
    a.checkEqual("22", util::divideAndRound(1, 3), 0);
    a.checkEqual("23", util::divideAndRound(2, 3), 1);
    a.checkEqual("24", util::divideAndRound(3, 3), 1);
    a.checkEqual("25", util::divideAndRound(4, 3), 1);
    a.checkEqual("26", util::divideAndRound(5, 3), 2);
    a.checkEqual("27", util::divideAndRound(6, 3), 2);
}

/** Test divideAndRoundUp. */
AFL_TEST("util.Math:divideAndRoundUp", a)
{
    // Divide by 1
    a.checkEqual("01", util::divideAndRoundUp(0, 1), 0);
    a.checkEqual("02", util::divideAndRoundUp(1, 1), 1);
    a.checkEqual("03", util::divideAndRoundUp(2, 1), 2);
    a.checkEqual("04", util::divideAndRoundUp(3, 1), 3);

    // Divide by 2
    a.checkEqual("11", util::divideAndRoundUp(0, 2), 0);
    a.checkEqual("12", util::divideAndRoundUp(1, 2), 1);
    a.checkEqual("13", util::divideAndRoundUp(2, 2), 1);
    a.checkEqual("14", util::divideAndRoundUp(3, 2), 2);
    a.checkEqual("15", util::divideAndRoundUp(4, 2), 2);

    // Divide by 3
    a.checkEqual("21", util::divideAndRoundUp(0, 3), 0);
    a.checkEqual("22", util::divideAndRoundUp(1, 3), 1);
    a.checkEqual("23", util::divideAndRoundUp(2, 3), 1);
    a.checkEqual("24", util::divideAndRoundUp(3, 3), 1);
    a.checkEqual("25", util::divideAndRoundUp(4, 3), 2);
    a.checkEqual("26", util::divideAndRoundUp(5, 3), 2);
    a.checkEqual("27", util::divideAndRoundUp(6, 3), 2);

    // Divide by 10
    a.checkEqual("31", util::divideAndRoundUp(0,  10), 0);
    a.checkEqual("32", util::divideAndRoundUp(1,  10), 1);
    a.checkEqual("33", util::divideAndRoundUp(2,  10), 1);
    a.checkEqual("34", util::divideAndRoundUp(3,  10), 1);
    a.checkEqual("35", util::divideAndRoundUp(9,  10), 1);
    a.checkEqual("36", util::divideAndRoundUp(10, 10), 1);
    a.checkEqual("37", util::divideAndRoundUp(11, 10), 2);
}

/** Test divideAndRoundToEven. */
AFL_TEST("util.Math:divideAndRoundToEven", a)
{
    // Trivial cases: divide by 1
    a.checkEqual("01", util::divideAndRoundToEven(0, 1, 0), 0);
    a.checkEqual("02", util::divideAndRoundToEven(1, 1, 0), 1);
    a.checkEqual("03", util::divideAndRoundToEven(2, 1, 0), 2);
    a.checkEqual("04", util::divideAndRoundToEven(3, 1, 0), 3);
    a.checkEqual("05", util::divideAndRoundToEven(0, 1, 1), 1);
    a.checkEqual("06", util::divideAndRoundToEven(0, 1, 2), 2);
    a.checkEqual("07", util::divideAndRoundToEven(0, 1, 3), 3);

    // Divide by 2
    a.checkEqual("11", util::divideAndRoundToEven(0, 2, 0), 0);
    a.checkEqual("12", util::divideAndRoundToEven(1, 2, 0), 0);
    a.checkEqual("13", util::divideAndRoundToEven(2, 2, 0), 1);
    a.checkEqual("14", util::divideAndRoundToEven(3, 2, 0), 2);
    a.checkEqual("15", util::divideAndRoundToEven(4, 2, 0), 2);

    a.checkEqual("21", util::divideAndRoundToEven(0, 2, 1), 1);
    a.checkEqual("22", util::divideAndRoundToEven(1, 2, 1), 2);
    a.checkEqual("23", util::divideAndRoundToEven(2, 2, 1), 2);
    a.checkEqual("24", util::divideAndRoundToEven(3, 2, 1), 2);
    a.checkEqual("25", util::divideAndRoundToEven(4, 2, 1), 3);

    a.checkEqual("31", util::divideAndRoundToEven(0, 2, -1), -1);
    a.checkEqual("32", util::divideAndRoundToEven(1, 2, -1), 0);
    a.checkEqual("33", util::divideAndRoundToEven(2, 2, -1), 0);
    a.checkEqual("34", util::divideAndRoundToEven(3, 2, -1), 0);
    a.checkEqual("35", util::divideAndRoundToEven(4, 2, -1), 1);
}

/** Test getHeadingRad/getHeadingDeg. */
AFL_TEST("util.Math:getHeadingDeg", a)
{
    // Degrees
    a.checkEqual("01", util::getHeadingDeg(1, 0), 90);
    a.checkEqual("02", util::getHeadingDeg(1, 1), 45);
    a.checkEqual("03", util::getHeadingDeg(0, 1), 0);
    a.checkEqual("04", util::getHeadingDeg(-1, 1), 315);
    a.checkEqual("05", util::getHeadingDeg(-1, 0), 270);
    a.checkEqual("06", util::getHeadingDeg(-1, -1), 225);
    a.checkEqual("07", util::getHeadingDeg(0, -1), 180);
    a.checkEqual("08", util::getHeadingDeg(1, -1), 135);
}

AFL_TEST("util.Math:getHeadingRad", a)
{
    // Radians
    a.checkEqual("01", util::getHeadingRad(1, 0), util::PI/2);
    a.checkEqual("02", util::getHeadingRad(1, 1), util::PI/4);
    a.checkEqual("03", util::getHeadingRad(0, 1), 0.0);
    a.checkEqual("04", util::getHeadingRad(-1, 1), util::PI*7/4);
    a.checkEqual("05", util::getHeadingRad(-1, 0), util::PI*3/2);
    a.checkEqual("06", util::getHeadingRad(-1, -1), util::PI*5/4);
    a.checkEqual("07", util::getHeadingRad(0, -1), util::PI);
    a.checkEqual("08", util::getHeadingRad(1, -1), util::PI*3/4);
}

AFL_TEST("util.Math:PI", a)
{
    a.check("01", util::PI >= 3.141592);
    a.check("02", util::PI <= 3.141593);
}

/** Test squareInteger. */
AFL_TEST("util.Math:squareInteger", a)
{
    // Possible signatures
    a.checkEqual("01", util::squareInteger(short(10)), 100);
    a.checkEqual("02", util::squareInteger(int(10)), 100);
    a.checkEqual("03", util::squareInteger(int16_t(10)), 100);
    a.checkEqual("04", util::squareInteger(int32_t(10)), 100);

    // Values
    a.checkEqual("11", util::squareInteger(20000), 400000000);
    a.checkEqual("12", util::squareInteger(42), 1764);
    a.checkEqual("13", util::squareInteger(1), 1);
    a.checkEqual("14", util::squareInteger(0), 0);
    a.checkEqual("15", util::squareInteger(-1), 1);
    a.checkEqual("16", util::squareInteger(-42), 1764);
    a.checkEqual("17", util::squareInteger(-20000), 400000000);
}

/** Test roundToInt. */
AFL_TEST("util.Math:roundToInt", a)
{
    a.checkEqual("01", util::roundToInt(0), 0);
    a.checkEqual("02", util::roundToInt(0.3), 0);
    a.checkEqual("03", util::roundToInt(0.5), 1);
    a.checkEqual("04", util::roundToInt(1.49), 1);
    a.checkEqual("05", util::roundToInt(1.5), 2);
    a.checkEqual("06", util::roundToInt(1.51), 2);

    a.checkEqual("11", util::roundToInt(-0.3), 0);
    a.checkEqual("12", util::roundToInt(-0.5), -1);
    a.checkEqual("13", util::roundToInt(-1.49), -1);
    a.checkEqual("14", util::roundToInt(-1.5), -2);
    a.checkEqual("15", util::roundToInt(-1.51), -2);
}

/** Test getDistanceFromDX, getDistance2FromDX. */
AFL_TEST("util.Math:getDistanceFromDX", a)
{
    a.checkEqual("01", util::getDistance2FromDX(0, 0), 0);
    a.checkEqual("02", util::getDistance2FromDX(0, 1), 1);
    a.checkEqual("03", util::getDistance2FromDX(0, 10), 100);
    a.checkEqual("04", util::getDistance2FromDX(0, 162), 26244);
    a.checkEqual("05", util::getDistance2FromDX(1, 0), 1);
    a.checkEqual("06", util::getDistance2FromDX(10, 0), 100);
    a.checkEqual("07", util::getDistance2FromDX(162, 0), 26244);
    a.checkEqual("08", util::getDistance2FromDX(3, 4), 25);
}

AFL_TEST("util.Math:getDistance2FromDX", a)
{
    a.checkEqual("01", util::getDistanceFromDX(0, 0), 0.0);
    a.checkEqual("02", util::getDistanceFromDX(0, 1), 1.0);
    a.checkEqual("03", util::getDistanceFromDX(0, 10), 10.0);
    a.checkEqual("04", util::getDistanceFromDX(0, 162), 162.0);
    a.checkEqual("05", util::getDistanceFromDX(1, 0), 1.0);
    a.checkEqual("06", util::getDistanceFromDX(10, 0), 10.0);
    a.checkEqual("07", util::getDistanceFromDX(162, 0), 162.0);
    a.checkEqual("08", util::getDistanceFromDX(3, 4), 5.0);
}

AFL_TEST("util.Math:squareFloat", a)
{
    a.checkEqual("01", util::squareFloat(20000.0), 400000000.0);
    a.checkEqual("02", util::squareFloat(42.0), 1764.0);
    a.checkEqual("03", util::squareFloat(1.5), 2.25);
    a.checkEqual("04", util::squareFloat(1.0), 1.0);
    a.checkEqual("05", util::squareFloat(0.5), 0.25);
    a.checkEqual("06", util::squareFloat(0.0), 0.0);
    a.checkEqual("07", util::squareFloat(-1.0), 1.0);
    a.checkEqual("08", util::squareFloat(-42.0), 1764.0);
}
