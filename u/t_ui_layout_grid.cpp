/**
  *  \file u/t_ui_layout_grid.cpp
  *  \brief Test for ui::layout::Grid
  */

#include "ui/layout/grid.hpp"

#include "t_ui_layout.hpp"
#include "ui/group.hpp"
#include "ui/spacer.hpp"

/** Test layout with some "fixed size" widgets. */
void
TestUiLayoutGrid::testFixed()
{
    using ui::layout::Info;
    using gfx::Point;
    using gfx::Rectangle;

    // Layout manager under test
    ui::layout::Grid testee(3);

    // Widget structure. Use spacers to define the Layout Info
    //     [10x3] [3x3] [5x2]      - height 3
    //     [2x2]  [9x8]            - height 8
    // width 10     9     5
    // -> total width = 34 (including gaps of 5)
    // -> total height = 16 (including gap of 5)
    ui::Group g(testee);
    ui::Spacer w11(Info(Point(10,3))), w12(Info(Point(3,3))), w13(Info(Point(5,2)));
    ui::Spacer w21(Info(Point(2,2))),  w22(Info(Point(9,8)));
    ui::Spacer ignore1((Info())), ignore2((Info()));

    g.add(w11);
    g.add(w12);
    g.add(ignore1);
    g.add(w13);
    g.add(ignore2);
    g.add(w21);
    g.add(w22);

    // Check layout info
    Info i = g.getLayoutInfo();
    TS_ASSERT_EQUALS(i.getPreferredSize(), Point(34, 16));

    // Perform layout
    g.setExtent(Rectangle(100, 100, 34, 16));
    TS_ASSERT_EQUALS(w11.getExtent(), Rectangle(100, 100, 10, 3));
    TS_ASSERT_EQUALS(w12.getExtent(), Rectangle(115, 100,  9, 3));
    TS_ASSERT_EQUALS(w13.getExtent(), Rectangle(129, 100,  5, 3));
    TS_ASSERT_EQUALS(w21.getExtent(), Rectangle(100, 108, 10, 8));
    TS_ASSERT_EQUALS(w22.getExtent(), Rectangle(115, 108,  9, 8));

    // Fix one size
    testee.setForcedCellSize(100, afl::base::Nothing);
    i = g.getLayoutInfo();
    TS_ASSERT_EQUALS(i.getPreferredSize(), Point(310, 16));

    g.setExtent(Rectangle(100, 100, 310, 16));
    TS_ASSERT_EQUALS(w11.getExtent(), Rectangle(100, 100, 100, 3));
    TS_ASSERT_EQUALS(w12.getExtent(), Rectangle(205, 100, 100, 3));
    TS_ASSERT_EQUALS(w13.getExtent(), Rectangle(310, 100, 100, 3));
    TS_ASSERT_EQUALS(w21.getExtent(), Rectangle(100, 108, 100, 8));
    TS_ASSERT_EQUALS(w22.getExtent(), Rectangle(205, 108, 100, 8));

    // Fix both sizes
    testee.setForcedCellSize(100, 50);
    i = g.getLayoutInfo();
    TS_ASSERT_EQUALS(i.getPreferredSize(), Point(310, 105));

    g.setExtent(Rectangle(100, 100, 310, 105));
    TS_ASSERT_EQUALS(w11.getExtent(), Rectangle(100, 100, 100, 50));
    TS_ASSERT_EQUALS(w12.getExtent(), Rectangle(205, 100, 100, 50));
    TS_ASSERT_EQUALS(w13.getExtent(), Rectangle(310, 100, 100, 50));
    TS_ASSERT_EQUALS(w21.getExtent(), Rectangle(100, 155, 100, 50));
    TS_ASSERT_EQUALS(w22.getExtent(), Rectangle(205, 155, 100, 50));
}

/** Test empty container. */
void
TestUiLayoutGrid::testEmpty()
{
    using ui::layout::Info;
    using gfx::Point;

    // Default, with default padding
    {
        ui::layout::Grid testee(5);
        ui::Group g(testee);
        Info info = g.getLayoutInfo();
        TS_ASSERT_EQUALS(info.getPreferredSize(), Point(0, 0));
    }

    // Configuring an outer
    {
        ui::layout::Grid testee(5, 5, 23);
        ui::Group g(testee);
        Info info = g.getLayoutInfo();
        TS_ASSERT_EQUALS(info.getPreferredSize(), Point(46, 46));
    }

    // Same thing, but with a widget which is ignored
    {
        ui::layout::Grid testee(5, 5, 23);
        ui::Group g(testee);
        ui::Spacer ignore((Info()));
        g.add(ignore);
        Info info = g.getLayoutInfo();
        TS_ASSERT_EQUALS(info.getPreferredSize(), Point(46, 46));
    }
}
