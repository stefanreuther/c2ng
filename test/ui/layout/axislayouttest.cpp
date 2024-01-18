/**
  *  \file test/ui/layout/axislayouttest.cpp
  *  \brief Test for ui::layout::AxisLayout
  */

#include "ui/layout/axislayout.hpp"
#include "afl/test/testrunner.hpp"

using ui::layout::AxisLayout;

/** Test data management. */
AFL_TEST("ui.layout.AxisLayout:data", a)
{
    // Initial state
    AxisLayout testee;
    a.checkEqual("01. empty", testee.empty(), true);
    a.checkEqual("02. size", testee.size(), 0U);

    // Set up content:
    //    [100 flex] [50 ignore] [200 fixed] [150 fixed]
    testee.add(100, true, false);
    testee.add(50,  true, true);
    testee.add(200, true, false);
    testee.add(150, false, false);
    testee.update(2, 180, false);

    // New size
    a.checkEqual("11. empty", testee.empty(), false);
    a.checkEqual("12. size", testee.size(), 4U);

    // Total size does not include ignored
    a.checkEqual("21. getTotalSize", testee.getTotalSize(), 450);

    // Flexible because we have one flexible component
    a.checkEqual("31. isFlexible", testee.isFlexible(), true);

    // Ignored slots
    a.checkEqual("41. isIgnored", testee.isIgnored(0), false);
    a.checkEqual("42. isIgnored", testee.isIgnored(1), true);
    a.checkEqual("43. isIgnored", testee.isIgnored(2), false);
    a.checkEqual("44. isIgnored", testee.isIgnored(3), false);
    a.checkEqual("45. isIgnored", testee.isIgnored(4), false);  // out-of-range
}

/*
 *  Test layout computation.
 */

// Empty
AFL_TEST("ui.layout.AxisLayout:computeLayout:empty", a)
{
    AxisLayout testee;
    std::vector<AxisLayout::Position> result = testee.computeLayout(0, 0, 100);
    a.checkEqual("size", result.size(), 0U);
}

// -- All fixed --
// No margin
AFL_TEST("ui.layout.AxisLayout:computeLayout:all-fixed:no-margin", a)
{
    AxisLayout testee;
    testee.add(100, false, false);
    testee.add(100, false, false);
    std::vector<AxisLayout::Position> result = testee.computeLayout(0, 0, 200);
    a.checkEqual("size", result.size(), 2U);
    a.checkEqual("pos 0",  result[0].position, 0);
    a.checkEqual("size 0", result[0].size, 100);
    a.checkEqual("pos 1",  result[1].position, 100);
    a.checkEqual("size 1", result[1].size, 100);
}

// Margins given, but removed due to lacking space
AFL_TEST("ui.layout.AxisLayout:computeLayout:all-fixed:small-margin", a)
{
    AxisLayout testee;
    testee.add(100, false, false);
    testee.add(100, false, false);
    std::vector<AxisLayout::Position> result = testee.computeLayout(10, 40, 200);
    a.checkEqual("size", result.size(), 2U);
    a.checkEqual("pos 0",  result[0].position, 0);
    a.checkEqual("size 0", result[0].size, 100);
    a.checkEqual("pos 1",  result[1].position, 100);
    a.checkEqual("size 1", result[1].size, 100);
}

// Correct margins given
AFL_TEST("ui.layout.AxisLayout:computeLayout:all-fixed:matching-margin", a)
{
    AxisLayout testee;
    testee.add(100, false, false);
    testee.add(100, false, false);
    std::vector<AxisLayout::Position> result = testee.computeLayout(10, 40, 290);
    a.checkEqual("size", result.size(), 2U);
    a.checkEqual("pos 0",  result[0].position, 40);
    a.checkEqual("size 0", result[0].size, 100);
    a.checkEqual("pos 1",  result[1].position, 150);
    a.checkEqual("size 1", result[1].size, 100);
}

// Size too large
AFL_TEST("ui.layout.AxisLayout:computeLayout:all-fixed:too-large", a)
{
    AxisLayout testee;
    testee.add(100, false, false);
    testee.add(100, false, false);
    std::vector<AxisLayout::Position> result = testee.computeLayout(10, 40, 390);
    a.checkEqual("size", result.size(), 2U);
    a.checkEqual("pos 0",  result[0].position, 40);
    a.checkEqual("size 0", result[0].size, 150);
    a.checkEqual("pos 1",  result[1].position, 200);
    a.checkEqual("size 1", result[1].size, 150);
}

// Size too small
AFL_TEST("ui.layout.AxisLayout:computeLayout:all-fixed:too-small", a)
{
    AxisLayout testee;
    testee.add(100, false, false);
    testee.add(100, false, false);
    std::vector<AxisLayout::Position> result = testee.computeLayout(10, 40, 50);
    a.checkEqual("size", result.size(), 2U);
    a.checkEqual("pos 0",  result[0].position, 0);
    a.checkEqual("size 0", result[0].size, 25);
    a.checkEqual("pos 1",  result[1].position, 25);
    a.checkEqual("size 1", result[1].size, 25);
}

// -- One flexible --
// No margin
AFL_TEST("ui.layout.AxisLayout:computeLayout:one-flexible:no-margin", a)
{
    AxisLayout testee;
    testee.add(100, false, false);
    testee.add(100, true, false);
    std::vector<AxisLayout::Position> result = testee.computeLayout(0, 0, 200);
    a.checkEqual("size", result.size(), 2U);
    a.checkEqual("pos 0",  result[0].position, 0);
    a.checkEqual("size 0", result[0].size, 100);
    a.checkEqual("pos 1",  result[1].position, 100);
    a.checkEqual("size 1", result[1].size, 100);
}

// Margins given, but removed due to lacking space
AFL_TEST("ui.layout.AxisLayout:computeLayout:one-flexible:small-margin", a)
{
    AxisLayout testee;
    testee.add(100, false, false);
    testee.add(100, true, false);
    std::vector<AxisLayout::Position> result = testee.computeLayout(10, 40, 200);
    a.checkEqual("size", result.size(), 2U);
    a.checkEqual("pos 0",  result[0].position, 0);
    a.checkEqual("size 0", result[0].size, 100);
    a.checkEqual("pos 1",  result[1].position, 100);
    a.checkEqual("size 1", result[1].size, 100);
}

// Correct margins given
AFL_TEST("ui.layout.AxisLayout:computeLayout:one-flexible:matching-margin", a)
{
    AxisLayout testee;
    testee.add(100, false, false);
    testee.add(100, true, false);
    std::vector<AxisLayout::Position> result = testee.computeLayout(10, 40, 290);
    a.checkEqual("size", result.size(), 2U);
    a.checkEqual("pos 0",  result[0].position, 40);
    a.checkEqual("size 0", result[0].size, 100);
    a.checkEqual("pos 1",  result[1].position, 150);
    a.checkEqual("size 1", result[1].size, 100);
}

// Size too large
AFL_TEST("ui.layout.AxisLayout:computeLayout:one-flexible:too-large", a)
{
    AxisLayout testee;
    testee.add(100, false, false);
    testee.add(100, true, false);
    std::vector<AxisLayout::Position> result = testee.computeLayout(10, 40, 390);
    a.checkEqual("size", result.size(), 2U);
    a.checkEqual("pos 0",  result[0].position, 40);
    a.checkEqual("size 0", result[0].size, 100);
    a.checkEqual("pos 1",  result[1].position, 150);
    a.checkEqual("size 1", result[1].size, 200);
}

// Size too small
AFL_TEST("ui.layout.AxisLayout:computeLayout:one-flexible:too-small", a)
{
    AxisLayout testee;
    testee.add(100, false, false);
    testee.add(100, true, false);
    std::vector<AxisLayout::Position> result = testee.computeLayout(10, 40, 50);
    a.checkEqual("size", result.size(), 2U);
    a.checkEqual("pos 0",  result[0].position, 0);
    a.checkEqual("size 0", result[0].size, 50);
    a.checkEqual("pos 1",  result[1].position, 50);
    a.checkEqual("size 1", result[1].size, 0);
}
