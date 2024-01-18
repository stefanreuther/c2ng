/**
  *  \file test/ui/layout/gridtest.cpp
  *  \brief Test for ui::layout::Grid
  */

#include "ui/layout/grid.hpp"

#include "afl/test/testrunner.hpp"
#include "ui/group.hpp"
#include "ui/spacer.hpp"

using gfx::Point;
using gfx::Rectangle;
using ui::layout::Info;

/** Test layout with some "fixed size" widgets. */
AFL_TEST("ui.layout.Grid:fixed", a)
{
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
    a.checkEqual("01. getPreferredSize", i.getPreferredSize(), Point(34, 16));

    // Perform layout
    g.setExtent(Rectangle(100, 100, 34, 16));
    a.checkEqual("11. getExtent", w11.getExtent(), Rectangle(100, 100, 10, 3));
    a.checkEqual("12. getExtent", w12.getExtent(), Rectangle(115, 100,  9, 3));
    a.checkEqual("13. getExtent", w13.getExtent(), Rectangle(129, 100,  5, 3));
    a.checkEqual("14. getExtent", w21.getExtent(), Rectangle(100, 108, 10, 8));
    a.checkEqual("15. getExtent", w22.getExtent(), Rectangle(115, 108,  9, 8));

    // Fix one size
    testee.setForcedCellSize(100, afl::base::Nothing);
    i = g.getLayoutInfo();
    a.checkEqual("21. getPreferredSize", i.getPreferredSize(), Point(310, 16));

    g.setExtent(Rectangle(100, 100, 310, 16));
    a.checkEqual("31. getExtent", w11.getExtent(), Rectangle(100, 100, 100, 3));
    a.checkEqual("32. getExtent", w12.getExtent(), Rectangle(205, 100, 100, 3));
    a.checkEqual("33. getExtent", w13.getExtent(), Rectangle(310, 100, 100, 3));
    a.checkEqual("34. getExtent", w21.getExtent(), Rectangle(100, 108, 100, 8));
    a.checkEqual("35. getExtent", w22.getExtent(), Rectangle(205, 108, 100, 8));

    // Fix both sizes
    testee.setForcedCellSize(100, 50);
    i = g.getLayoutInfo();
    a.checkEqual("41. getPreferredSize", i.getPreferredSize(), Point(310, 105));

    g.setExtent(Rectangle(100, 100, 310, 105));
    a.checkEqual("51. getExtent", w11.getExtent(), Rectangle(100, 100, 100, 50));
    a.checkEqual("52. getExtent", w12.getExtent(), Rectangle(205, 100, 100, 50));
    a.checkEqual("53. getExtent", w13.getExtent(), Rectangle(310, 100, 100, 50));
    a.checkEqual("54. getExtent", w21.getExtent(), Rectangle(100, 155, 100, 50));
    a.checkEqual("55. getExtent", w22.getExtent(), Rectangle(205, 155, 100, 50));
}

/*
 *  Test empty container.
 */

// Default, with default padding
AFL_TEST("ui.layout.Grid:empty:default", a)
{
    ui::layout::Grid testee(5);
    ui::Group g(testee);
    Info info = g.getLayoutInfo();
    a.checkEqual("", info.getPreferredSize(), Point(0, 0));
}

// Configuring an outer
AFL_TEST("ui.layout.Grid:empty:outer", a)
{
    ui::layout::Grid testee(5, 5, 23);
    ui::Group g(testee);
    Info info = g.getLayoutInfo();
    a.checkEqual("", info.getPreferredSize(), Point(46, 46));
}

// Same thing, but with a widget which is ignored
AFL_TEST("ui.layout.Grid:empty:ignored-widget", a)
{
    ui::layout::Grid testee(5, 5, 23);
    ui::Group g(testee);
    ui::Spacer ignore((Info()));
    g.add(ignore);
    Info info = g.getLayoutInfo();
    a.checkEqual("", info.getPreferredSize(), Point(46, 46));
}
