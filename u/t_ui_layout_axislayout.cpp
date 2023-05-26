/**
  *  \file u/t_ui_layout_axislayout.cpp
  *  \brief Test for ui::layout::AxisLayout
  */

#include "ui/layout/axislayout.hpp"

#include "t_ui_layout.hpp"

using ui::layout::AxisLayout;

/** Test data management. */
void
TestUiLayoutAxisLayout::testData()
{
    // Initial state
    AxisLayout testee;
    TS_ASSERT_EQUALS(testee.empty(), true);
    TS_ASSERT_EQUALS(testee.size(), 0U);

    // Set up content:
    //    [100 flex] [50 ignore] [200 fixed] [150 fixed]
    testee.add(100, true, false);
    testee.add(50,  true, true);
    testee.add(200, true, false);
    testee.add(150, false, false);
    testee.update(2, 180, false);

    // New size
    TS_ASSERT_EQUALS(testee.empty(), false);
    TS_ASSERT_EQUALS(testee.size(), 4U);

    // Total size does not include ignored
    TS_ASSERT_EQUALS(testee.getTotalSize(), 450);

    // Flexible because we have one flexible component
    TS_ASSERT_EQUALS(testee.isFlexible(), true);

    // Ignored slots
    TS_ASSERT_EQUALS(testee.isIgnored(0), false);
    TS_ASSERT_EQUALS(testee.isIgnored(1), true);
    TS_ASSERT_EQUALS(testee.isIgnored(2), false);
    TS_ASSERT_EQUALS(testee.isIgnored(3), false);
    TS_ASSERT_EQUALS(testee.isIgnored(4), false);  // out-of-range
}

/** Test layout computation. */
void
TestUiLayoutAxisLayout::testLayout()
{
    // Empty
    {
        AxisLayout testee;
        std::vector<AxisLayout::Position> result = testee.computeLayout(0, 0, 100);
        TS_ASSERT_EQUALS(result.size(), 0U);
    }

    // All fixed
    {
        AxisLayout testee;
        testee.add(100, false, false);
        testee.add(100, false, false);

        // No margin
        {
            std::vector<AxisLayout::Position> result = testee.computeLayout(0, 0, 200);
            TS_ASSERT_EQUALS(result.size(), 2U);
            TS_ASSERT_EQUALS(result[0].position, 0);
            TS_ASSERT_EQUALS(result[0].size, 100);
            TS_ASSERT_EQUALS(result[1].position, 100);
            TS_ASSERT_EQUALS(result[1].size, 100);
        }

        // Margins given, but removed due to lacking space
        {
            std::vector<AxisLayout::Position> result = testee.computeLayout(10, 40, 200);
            TS_ASSERT_EQUALS(result.size(), 2U);
            TS_ASSERT_EQUALS(result[0].position, 0);
            TS_ASSERT_EQUALS(result[0].size, 100);
            TS_ASSERT_EQUALS(result[1].position, 100);
            TS_ASSERT_EQUALS(result[1].size, 100);
        }

        // Correct margins given
        {
            std::vector<AxisLayout::Position> result = testee.computeLayout(10, 40, 290);
            TS_ASSERT_EQUALS(result.size(), 2U);
            TS_ASSERT_EQUALS(result[0].position, 40);
            TS_ASSERT_EQUALS(result[0].size, 100);
            TS_ASSERT_EQUALS(result[1].position, 150);
            TS_ASSERT_EQUALS(result[1].size, 100);
        }

        // Size too large
        {
            std::vector<AxisLayout::Position> result = testee.computeLayout(10, 40, 390);
            TS_ASSERT_EQUALS(result.size(), 2U);
            TS_ASSERT_EQUALS(result[0].position, 40);
            TS_ASSERT_EQUALS(result[0].size, 150);
            TS_ASSERT_EQUALS(result[1].position, 200);
            TS_ASSERT_EQUALS(result[1].size, 150);
        }

        // Size too small
        {
            std::vector<AxisLayout::Position> result = testee.computeLayout(10, 40, 50);
            TS_ASSERT_EQUALS(result.size(), 2U);
            TS_ASSERT_EQUALS(result[0].position, 0);
            TS_ASSERT_EQUALS(result[0].size, 25);
            TS_ASSERT_EQUALS(result[1].position, 25);
            TS_ASSERT_EQUALS(result[1].size, 25);
        }
    }

    // One flexible
    {
        AxisLayout testee;
        testee.add(100, false, false);
        testee.add(100, true, false);

        // No margin
        {
            std::vector<AxisLayout::Position> result = testee.computeLayout(0, 0, 200);
            TS_ASSERT_EQUALS(result.size(), 2U);
            TS_ASSERT_EQUALS(result[0].position, 0);
            TS_ASSERT_EQUALS(result[0].size, 100);
            TS_ASSERT_EQUALS(result[1].position, 100);
            TS_ASSERT_EQUALS(result[1].size, 100);
        }

        // Margins given, but removed due to lacking space
        {
            std::vector<AxisLayout::Position> result = testee.computeLayout(10, 40, 200);
            TS_ASSERT_EQUALS(result.size(), 2U);
            TS_ASSERT_EQUALS(result[0].position, 0);
            TS_ASSERT_EQUALS(result[0].size, 100);
            TS_ASSERT_EQUALS(result[1].position, 100);
            TS_ASSERT_EQUALS(result[1].size, 100);
        }

        // Correct margins given
        {
            std::vector<AxisLayout::Position> result = testee.computeLayout(10, 40, 290);
            TS_ASSERT_EQUALS(result.size(), 2U);
            TS_ASSERT_EQUALS(result[0].position, 40);
            TS_ASSERT_EQUALS(result[0].size, 100);
            TS_ASSERT_EQUALS(result[1].position, 150);
            TS_ASSERT_EQUALS(result[1].size, 100);
        }

        // Size too large
        {
            std::vector<AxisLayout::Position> result = testee.computeLayout(10, 40, 390);
            TS_ASSERT_EQUALS(result.size(), 2U);
            TS_ASSERT_EQUALS(result[0].position, 40);
            TS_ASSERT_EQUALS(result[0].size, 100);
            TS_ASSERT_EQUALS(result[1].position, 150);
            TS_ASSERT_EQUALS(result[1].size, 200);
        }

        // Size too small
        {
            std::vector<AxisLayout::Position> result = testee.computeLayout(10, 40, 50);
            TS_ASSERT_EQUALS(result.size(), 2U);
            TS_ASSERT_EQUALS(result[0].position, 0);
            TS_ASSERT_EQUALS(result[0].size, 50);
            TS_ASSERT_EQUALS(result[1].position, 50);
            TS_ASSERT_EQUALS(result[1].size, 0);
        }
    }
}

