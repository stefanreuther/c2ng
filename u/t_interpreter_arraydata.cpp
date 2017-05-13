/**
  *  \file u/t_interpreter_arraydata.cpp
  *  \brief Test for interpreter::ArrayData
  */

#include "interpreter/arraydata.hpp"

#include "t_interpreter.hpp"
#include "afl/data/segment.hpp"
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
void
TestInterpreterArrayData::testSimple()
{
    size_t n = 0;
    interpreter::ArrayData testee;
    TS_ASSERT_EQUALS(testee.getNumDimensions(), 0U);
    TS_ASSERT_EQUALS(testee.getDimension(1), 0U);
    TS_ASSERT_EQUALS(testee.getDimension(10000), 0U);

    // Make it a vector of size 100
    TS_ASSERT(testee.addDimension(100));
    TS_ASSERT_EQUALS(testee.getNumDimensions(), 1U);
    TS_ASSERT_EQUALS(testee.getDimension(0), 100U);
    TS_ASSERT_EQUALS(testee.getDimension(10000), 0U);
    TS_ASSERT_EQUALS(testee.getDimensions().size(), 1U);

    {
        // Index (30) -> ok
        afl::data::Segment seg;
        seg.pushBackInteger(30);
        interpreter::Arguments args(seg, 0, seg.size());
        TS_ASSERT(testee.computeIndex(args, n));
        TS_ASSERT_EQUALS(n, 30U);
    }
    {
        // Index (0) -> ok
        afl::data::Segment seg;
        seg.pushBackInteger(0);
        interpreter::Arguments args(seg, 0, seg.size());
        TS_ASSERT(testee.computeIndex(args, n));
        TS_ASSERT_EQUALS(n, 0U);
    }
    {
        // Index (99) -> ok
        afl::data::Segment seg;
        seg.pushBackInteger(99);
        interpreter::Arguments args(seg, 0, seg.size());
        TS_ASSERT(testee.computeIndex(args, n));
        TS_ASSERT_EQUALS(n, 99U);
    }
    {
        // Index (-1) -> nok
        afl::data::Segment seg;
        seg.pushBackInteger(-1);
        interpreter::Arguments args(seg, 0, seg.size());
        TS_ASSERT_THROWS(testee.computeIndex(args, n), interpreter::Error);
    }
    {
        // Index (100) -> nok
        afl::data::Segment seg;
        seg.pushBackInteger(100);
        interpreter::Arguments args(seg, 0, seg.size());
        TS_ASSERT_THROWS(testee.computeIndex(args, n), interpreter::Error);
    }
    {
        // Index ("7") -> nok
        afl::data::Segment seg;
        seg.pushBackString("7");
        interpreter::Arguments args(seg, 0, seg.size());
        TS_ASSERT_THROWS(testee.computeIndex(args, n), interpreter::Error);
    }
    {
        // Index () -> nok
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, seg.size());
        TS_ASSERT_THROWS(testee.computeIndex(args, n), interpreter::Error);
    }
    {
        // Index (1,1) -> nok
        afl::data::Segment seg;
        seg.pushBackInteger(1);
        seg.pushBackInteger(1);
        interpreter::Arguments args(seg, 0, seg.size());
        TS_ASSERT_THROWS(testee.computeIndex(args, n), interpreter::Error);
    }
    {
        // Index (null) -> nok
        afl::data::Segment seg;
        seg.pushBack(0);
        interpreter::Arguments args(seg, 0, seg.size());
        TS_ASSERT(!testee.computeIndex(args, n));
    }
}

/** Tests with a matrix. */
void
TestInterpreterArrayData::testMatrix()
{
    size_t n = 0;
    interpreter::ArrayData testee;
    TS_ASSERT_EQUALS(testee.getNumDimensions(), 0U);

    // Make it a matrix of size 100x200
    TS_ASSERT(testee.addDimension(100));
    TS_ASSERT(testee.addDimension(200));
    TS_ASSERT_EQUALS(testee.getNumDimensions(), 2U);
    TS_ASSERT_EQUALS(testee.getDimensions().size(), 2U);

    {
        // Index (30,20) -> ok
        afl::data::Segment seg;
        seg.pushBackInteger(30);
        seg.pushBackInteger(20);
        interpreter::Arguments args(seg, 0, seg.size());
        TS_ASSERT(testee.computeIndex(args, n));
        TS_ASSERT_EQUALS(n, 6020U);
    }
    {
        // Index (0,0) -> ok
        afl::data::Segment seg;
        seg.pushBackInteger(0);
        seg.pushBackInteger(0);
        interpreter::Arguments args(seg, 0, seg.size());
        TS_ASSERT(testee.computeIndex(args, n));
        TS_ASSERT_EQUALS(n, 0U);
    }
    {
        // Index (99,199) -> ok
        afl::data::Segment seg;
        seg.pushBackInteger(99);
        seg.pushBackInteger(199);
        interpreter::Arguments args(seg, 0, seg.size());
        TS_ASSERT(testee.computeIndex(args, n));
        TS_ASSERT_EQUALS(n, 19999U);
    }
    {
        // Index (-1,1) -> nok
        afl::data::Segment seg;
        seg.pushBackInteger(-1);
        seg.pushBackInteger(1);
        interpreter::Arguments args(seg, 0, seg.size());
        TS_ASSERT_THROWS(testee.computeIndex(args, n), interpreter::Error);
    }
    {
        // Index (100,1) -> nok
        afl::data::Segment seg;
        seg.pushBackInteger(100);
        seg.pushBackInteger(1);
        interpreter::Arguments args(seg, 0, seg.size());
        TS_ASSERT_THROWS(testee.computeIndex(args, n), interpreter::Error);
    }
    {
        // Index () -> nok
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, seg.size());
        TS_ASSERT_THROWS(testee.computeIndex(args, n), interpreter::Error);
    }
    {
        // Index (1) -> nok
        afl::data::Segment seg;
        seg.pushBackInteger(1);
        interpreter::Arguments args(seg, 0, seg.size());
        TS_ASSERT_THROWS(testee.computeIndex(args, n), interpreter::Error);
    }
    {
        // Index (1) -> nok
        afl::data::Segment seg;
        seg.pushBackInteger(1);
        seg.pushBackString("7");
        interpreter::Arguments args(seg, 0, seg.size());
        TS_ASSERT_THROWS(testee.computeIndex(args, n), interpreter::Error);
    }
    {
        // Index (null,1) -> nok
        afl::data::Segment seg;
        seg.pushBack(0);
        seg.pushBackInteger(1);
        interpreter::Arguments args(seg, 0, seg.size());
        TS_ASSERT(!testee.computeIndex(args, n));
    }
    {
        // Index (1,null) -> nok
        afl::data::Segment seg;
        seg.pushBackInteger(1);
        seg.pushBack(0);
        interpreter::Arguments args(seg, 0, seg.size());
        TS_ASSERT(!testee.computeIndex(args, n));
    }
}

/** Test resizing. */
void
TestInterpreterArrayData::testResize()
{
    size_t n = 0;
    int32_t iv = 0;

    // Make a 10x20x30 (=6000 element) array
    interpreter::ArrayData testee;
    TS_ASSERT(testee.addDimension(10));
    TS_ASSERT(testee.addDimension(20));
    TS_ASSERT(testee.addDimension(30));
    TS_ASSERT_EQUALS(testee.getNumDimensions(), 3U);

    // Prepare a coordinate
    afl::data::Segment coord;
    coord.pushBackInteger(5);
    coord.pushBackInteger(6);
    coord.pushBackInteger(7);

    // Place a value
    {
        interpreter::Arguments args(coord, 0, coord.size());
        TS_ASSERT(testee.computeIndex(args, n));
        testee.content.setNew(n, interpreter::makeIntegerValue(42));
        TS_ASSERT_EQUALS(countValues(testee.content), 1U);
    }

    // Resize to 15x20x30
    {
        interpreter::ArrayData newSize;
        TS_ASSERT(newSize.addDimension(15));
        TS_ASSERT(newSize.addDimension(20));
        TS_ASSERT(newSize.addDimension(30));
        testee.resize(newSize);
    }

    // Verify value
    {
        interpreter::Arguments args(coord, 0, coord.size());
        TS_ASSERT(testee.computeIndex(args, n));
        TS_ASSERT(interpreter::checkIntegerArg(iv, testee.content[n]));
        TS_ASSERT_EQUALS(iv, 42);
        TS_ASSERT_EQUALS(countValues(testee.content), 1U);
    }

    // Resize to 16x26x36
    {
        interpreter::ArrayData newSize;
        TS_ASSERT(newSize.addDimension(16));
        TS_ASSERT(newSize.addDimension(26));
        TS_ASSERT(newSize.addDimension(36));
        testee.resize(newSize);
    }

    // Verify value
    {
        interpreter::Arguments args(coord, 0, coord.size());
        TS_ASSERT(testee.computeIndex(args, n));
        TS_ASSERT(interpreter::checkIntegerArg(iv, testee.content[n]));
        TS_ASSERT_EQUALS(iv, 42);
        TS_ASSERT_EQUALS(countValues(testee.content), 1U);
    }

    // Resize to 6x6x6
    {
        interpreter::ArrayData newSize;
        TS_ASSERT(newSize.addDimension(6));
        TS_ASSERT(newSize.addDimension(6));
        TS_ASSERT(newSize.addDimension(6));
        testee.resize(newSize);
    }

    // Value gone now
    {
        interpreter::Arguments args(coord, 0, coord.size());
        TS_ASSERT_THROWS(testee.computeIndex(args, n), interpreter::Error);
        TS_ASSERT_EQUALS(countValues(testee.content), 0U);
    }

    // Invalid resize request (wrong dimension)
    {
        interpreter::ArrayData newSize;
        TS_ASSERT(newSize.addDimension(16));
        TS_ASSERT(newSize.addDimension(26));
        TS_ASSERT_THROWS(testee.resize(newSize), interpreter::Error);
    }
}

/** Test dimensions. */
void
TestInterpreterArrayData::testDimension()
{
    // Maximum size
    {
        interpreter::ArrayData a;
        TS_ASSERT(a.addDimension(10001));
        TS_ASSERT(a.addDimension(10001));
        TS_ASSERT(a.addDimension(1));
        TS_ASSERT(a.addDimension(1));
        TS_ASSERT(a.addDimension(1));
        TS_ASSERT(a.addDimension(1));
        TS_ASSERT(!a.addDimension(2));
    }
    // Maximum size (2)
    {
        interpreter::ArrayData a;
        TS_ASSERT(a.addDimension(1));
        TS_ASSERT(a.addDimension(1));
        TS_ASSERT(a.addDimension(1));
        TS_ASSERT(a.addDimension(1));
        TS_ASSERT(a.addDimension(10001));
        TS_ASSERT(a.addDimension(10001));
        TS_ASSERT(!a.addDimension(2));
    }
    // Maximum size (3)
    {
        interpreter::ArrayData a;
        TS_ASSERT(a.addDimension(73));
        TS_ASSERT(a.addDimension(137));
        TS_ASSERT(a.addDimension(73));
        TS_ASSERT(a.addDimension(137));
        TS_ASSERT(!a.addDimension(2));
    }
    // Maximum size (4)
    {
        interpreter::ArrayData a;
        TS_ASSERT(a.addDimension(100020001));
        TS_ASSERT(!a.addDimension(2));
    }
    // Maximum size (5)
    {
        interpreter::ArrayData a;
        TS_ASSERT(!a.addDimension(100020002));
    }
    // Maximum size (6)
    {
        interpreter::ArrayData a;
        TS_ASSERT(a.addDimension(50010001));
        TS_ASSERT(!a.addDimension(2));
    }
    // Maximum size (7)
    {
        interpreter::ArrayData a;
        TS_ASSERT(a.addDimension(50010001));
        TS_ASSERT(!a.addDimension(50010001));
    }
}

