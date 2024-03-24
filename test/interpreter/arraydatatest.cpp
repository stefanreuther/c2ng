/**
  *  \file test/interpreter/arraydatatest.cpp
  *  \brief Test for interpreter::ArrayData
  */

#include "interpreter/arraydata.hpp"

#include "afl/data/segment.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/values.hpp"

namespace {
    size_t countValues(const afl::data::Segment& seg)
    {
        size_t result = 0;
        for (size_t i = 0, n = seg.size(); i < n; ++i) {
            if (seg[i] != 0) {
                ++result;
            }
        }
        return result;
    }
}

/** Simple tests. */
AFL_TEST("interpreter.ArrayData:vector", a)
{
    size_t n = 0;
    interpreter::ArrayData testee;
    a.checkEqual("01. getNumDimensions", testee.getNumDimensions(), 0U);
    a.checkEqual("02. getDimension",     testee.getDimension(1), 0U);
    a.checkEqual("03. getDimension",     testee.getDimension(10000), 0U);

    // Make it a vector of size 100
    a.check     ("11. addDimension",     testee.addDimension(100));
    a.checkEqual("12. getNumDimensions", testee.getNumDimensions(), 1U);
    a.checkEqual("13. getDimension",     testee.getDimension(0), 100U);
    a.checkEqual("14. getDimension",     testee.getDimension(10000), 0U);
    a.checkEqual("15. getDimension",     testee.getDimensions().size(), 1U);

    {
        // Index (30) -> ok
        afl::data::Segment seg;
        seg.pushBackInteger(30);
        interpreter::Arguments args(seg, 0, seg.size());
        a.check("21. computeIndex", testee.computeIndex(args, n));
        a.checkEqual("22. result", n, 30U);
    }
    {
        // Index (0) -> ok
        afl::data::Segment seg;
        seg.pushBackInteger(0);
        interpreter::Arguments args(seg, 0, seg.size());
        a.check("23. computeIndex", testee.computeIndex(args, n));
        a.checkEqual("24. result", n, 0U);
    }
    {
        // Index (99) -> ok
        afl::data::Segment seg;
        seg.pushBackInteger(99);
        interpreter::Arguments args(seg, 0, seg.size());
        a.check("25. computeIndex", testee.computeIndex(args, n));
        a.checkEqual("26. result", n, 99U);
    }
    {
        // Index (-1) -> nok
        afl::data::Segment seg;
        seg.pushBackInteger(-1);
        interpreter::Arguments args(seg, 0, seg.size());
        AFL_CHECK_THROWS(a("27. computeIndex"), testee.computeIndex(args, n), interpreter::Error);
    }
    {
        // Index (100) -> nok
        afl::data::Segment seg;
        seg.pushBackInteger(100);
        interpreter::Arguments args(seg, 0, seg.size());
        AFL_CHECK_THROWS(a("28. computeIndex"), testee.computeIndex(args, n), interpreter::Error);
    }
    {
        // Index ("7") -> nok
        afl::data::Segment seg;
        seg.pushBackString("7");
        interpreter::Arguments args(seg, 0, seg.size());
        AFL_CHECK_THROWS(a("29. computeIndex"), testee.computeIndex(args, n), interpreter::Error);
    }
    {
        // Index () -> nok
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, seg.size());
        AFL_CHECK_THROWS(a("30. computeIndex"), testee.computeIndex(args, n), interpreter::Error);
    }
    {
        // Index (1,1) -> nok
        afl::data::Segment seg;
        seg.pushBackInteger(1);
        seg.pushBackInteger(1);
        interpreter::Arguments args(seg, 0, seg.size());
        AFL_CHECK_THROWS(a("31. computeIndex"), testee.computeIndex(args, n), interpreter::Error);
    }
    {
        // Index (null) -> nok
        afl::data::Segment seg;
        seg.pushBack(0);
        interpreter::Arguments args(seg, 0, seg.size());
        a.check("32. computeIndex", !testee.computeIndex(args, n));
    }
}

/** Tests with a matrix. */
AFL_TEST("interpreter.ArrayData:matrix", a)
{
    size_t n = 0;
    interpreter::ArrayData testee;
    a.checkEqual("01. getNumDimensions", testee.getNumDimensions(), 0U);

    // Make it a matrix of size 100x200
    a.check     ("11. addDimension",     testee.addDimension(100));
    a.check     ("12. addDimension",     testee.addDimension(200));
    a.checkEqual("13. getNumDimensions", testee.getNumDimensions(), 2U);
    a.checkEqual("14. getDimensions",    testee.getDimensions().size(), 2U);

    {
        // Index (30,20) -> ok
        afl::data::Segment seg;
        seg.pushBackInteger(30);
        seg.pushBackInteger(20);
        interpreter::Arguments args(seg, 0, seg.size());
        a.check("21. computeIndex", testee.computeIndex(args, n));
        a.checkEqual("22. result", n, 6020U);
    }
    {
        // Index (0,0) -> ok
        afl::data::Segment seg;
        seg.pushBackInteger(0);
        seg.pushBackInteger(0);
        interpreter::Arguments args(seg, 0, seg.size());
        a.check("23. computeIndex", testee.computeIndex(args, n));
        a.checkEqual("24. result", n, 0U);
    }
    {
        // Index (99,199) -> ok
        afl::data::Segment seg;
        seg.pushBackInteger(99);
        seg.pushBackInteger(199);
        interpreter::Arguments args(seg, 0, seg.size());
        a.check("25. computeIndex", testee.computeIndex(args, n));
        a.checkEqual("26. result", n, 19999U);
    }
    {
        // Index (-1,1) -> nok
        afl::data::Segment seg;
        seg.pushBackInteger(-1);
        seg.pushBackInteger(1);
        interpreter::Arguments args(seg, 0, seg.size());
        AFL_CHECK_THROWS(a("27. computeIndex"), testee.computeIndex(args, n), interpreter::Error);
    }
    {
        // Index (100,1) -> nok
        afl::data::Segment seg;
        seg.pushBackInteger(100);
        seg.pushBackInteger(1);
        interpreter::Arguments args(seg, 0, seg.size());
        AFL_CHECK_THROWS(a("28. computeIndex"), testee.computeIndex(args, n), interpreter::Error);
    }
    {
        // Index () -> nok
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, seg.size());
        AFL_CHECK_THROWS(a("29. computeIndex"), testee.computeIndex(args, n), interpreter::Error);
    }
    {
        // Index (1) -> nok
        afl::data::Segment seg;
        seg.pushBackInteger(1);
        interpreter::Arguments args(seg, 0, seg.size());
        AFL_CHECK_THROWS(a("30. computeIndex"), testee.computeIndex(args, n), interpreter::Error);
    }
    {
        // Index (1) -> nok
        afl::data::Segment seg;
        seg.pushBackInteger(1);
        seg.pushBackString("7");
        interpreter::Arguments args(seg, 0, seg.size());
        AFL_CHECK_THROWS(a("31. computeIndex"), testee.computeIndex(args, n), interpreter::Error);
    }
    {
        // Index (null,1) -> nok
        afl::data::Segment seg;
        seg.pushBack(0);
        seg.pushBackInteger(1);
        interpreter::Arguments args(seg, 0, seg.size());
        a.check("32. computeIndex", !testee.computeIndex(args, n));
    }
    {
        // Index (1,null) -> nok
        afl::data::Segment seg;
        seg.pushBackInteger(1);
        seg.pushBack(0);
        interpreter::Arguments args(seg, 0, seg.size());
        a.check("33. computeIndex", !testee.computeIndex(args, n));
    }
}

/** Test resizing. */
AFL_TEST("interpreter.ArrayData:resize", a)
{
    size_t n = 0;
    int32_t iv = 0;

    // Make a 10x20x30 (=6000 element) array
    interpreter::ArrayData testee;
    a.check("01. addDimension", testee.addDimension(10));
    a.check("02. addDimension", testee.addDimension(20));
    a.check("03. addDimension", testee.addDimension(30));
    a.checkEqual("04. getNumDimensions", testee.getNumDimensions(), 3U);

    // Prepare a coordinate
    afl::data::Segment coord;
    coord.pushBackInteger(5);
    coord.pushBackInteger(6);
    coord.pushBackInteger(7);

    // Place a value
    {
        interpreter::Arguments args(coord, 0, coord.size());
        a.check("11. computeIndex", testee.computeIndex(args, n));
        testee.content().setNew(n, interpreter::makeIntegerValue(42));
        a.checkEqual("12. countValues", countValues(testee.content()), 1U);
    }

    // Resize to 15x20x30
    {
        interpreter::ArrayData newSize;
        a.check("21. addDimension", newSize.addDimension(15));
        a.check("22. addDimension", newSize.addDimension(20));
        a.check("23. addDimension", newSize.addDimension(30));
        testee.resize(newSize);
    }

    // Verify value
    {
        interpreter::Arguments args(coord, 0, coord.size());
        a.check("31. computeIndex", testee.computeIndex(args, n));
        a.check("32. checkIntegerArg", interpreter::checkIntegerArg(iv, testee.content()[n]));
        a.checkEqual("33. value", iv, 42);
        a.checkEqual("34. countValues", countValues(testee.content()), 1U);
    }

    // Resize to 16x26x36
    {
        interpreter::ArrayData newSize;
        a.check("41. addDimension", newSize.addDimension(16));
        a.check("42. addDimension", newSize.addDimension(26));
        a.check("43. addDimension", newSize.addDimension(36));
        testee.resize(newSize);
    }

    // Verify value
    {
        interpreter::Arguments args(coord, 0, coord.size());
        a.check("51. computeIndex", testee.computeIndex(args, n));
        a.check("52. checkIntegerArg", interpreter::checkIntegerArg(iv, testee.content()[n]));
        a.checkEqual("53. value", iv, 42);
        a.checkEqual("54. countValues", countValues(testee.content()), 1U);
    }

    // Resize to 6x6x6
    {
        interpreter::ArrayData newSize;
        a.check("61. addDimension", newSize.addDimension(6));
        a.check("62. addDimension", newSize.addDimension(6));
        a.check("63. addDimension", newSize.addDimension(6));
        testee.resize(newSize);
    }

    // Value gone now
    {
        interpreter::Arguments args(coord, 0, coord.size());
        AFL_CHECK_THROWS(a("71. computeIndex"), testee.computeIndex(args, n), interpreter::Error);
        a.checkEqual("72. countValues", countValues(testee.content()), 0U);
    }

    // Invalid resize request (wrong dimension)
    {
        interpreter::ArrayData newSize;
        a.check("81. addDimension", newSize.addDimension(16));
        a.check("82. addDimension", newSize.addDimension(26));
        AFL_CHECK_THROWS(a("83. result"), testee.resize(newSize), interpreter::Error);
    }
}

AFL_TEST("interpreter.ArrayData:resize:in-place", a)
{
    interpreter::ArrayData testee;
    a.check("01. addDimension", testee.addDimension(10));
    testee.content().setNew(8, interpreter::makeIntegerValue(10));
    testee.content().setNew(9, interpreter::makeIntegerValue(20));

    // Reduce size
    interpreter::ArrayData newSize;
    a.check("11. addDimension", newSize.addDimension(9));
    testee.resize(newSize);
    a.checkNonNull("12. get", testee.content()[8]);
    a.checkNull("13. get", testee.content()[9]);
}

/** Test dimensions. */

// Maximum size
AFL_TEST("interpreter.ArrayData:addDimension:limit:big-then-small", a)
{
    interpreter::ArrayData ad;
    a.check("01", ad.addDimension(10001));
    a.check("02", ad.addDimension(10001));
    a.check("03", ad.addDimension(1));
    a.check("04", ad.addDimension(1));
    a.check("05", ad.addDimension(1));
    a.check("06", ad.addDimension(1));
    a.check("07", !ad.addDimension(2));
}

// Maximum size (2)
AFL_TEST("interpreter.ArrayData:addDimension:limit:small-then-big", a)
{
    interpreter::ArrayData ad;
    a.check("01", ad.addDimension(1));
    a.check("02", ad.addDimension(1));
    a.check("03", ad.addDimension(1));
    a.check("04", ad.addDimension(1));
    a.check("05", ad.addDimension(10001));
    a.check("06", ad.addDimension(10001));
    a.check("07", !ad.addDimension(2));
}

// Maximum size (3)
AFL_TEST("interpreter.ArrayData:addDimension:limit:mixed", a)
{
    interpreter::ArrayData ad;
    a.check("01", ad.addDimension(73));
    a.check("02", ad.addDimension(137));
    a.check("03", ad.addDimension(73));
    a.check("04", ad.addDimension(137));
    a.check("05", !ad.addDimension(2));
}

// Maximum size (4)
AFL_TEST("interpreter.ArrayData:addDimension:limit:max", a)
{
    interpreter::ArrayData ad;
    a.check("01", ad.addDimension(100020001));
    a.check("02", !ad.addDimension(2));
}

// Maximum size (5)
AFL_TEST("interpreter.ArrayData:addDimension:limit:single-over-limit", a)
{
    interpreter::ArrayData ad;
    a.check("01", !ad.addDimension(100020002));
}

// Maximum size (6)
AFL_TEST("interpreter.ArrayData:addDimension:limit:half-then-double", a)
{
    interpreter::ArrayData ad;
    a.check("01", ad.addDimension(50010001));
    a.check("02", !ad.addDimension(2));
}

// Maximum size (7)
AFL_TEST("interpreter.ArrayData:addDimension:limit:half-squared", a)
{
    interpreter::ArrayData ad;
    a.check("01", ad.addDimension(50010001));
    a.check("02", !ad.addDimension(50010001));
}

// Negative
AFL_TEST("interpreter.ArrayData:addDimension:negative", a)
{
    interpreter::ArrayData ad;
    a.check("01", !ad.addDimension(-1));
}
