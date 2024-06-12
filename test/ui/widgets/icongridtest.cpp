/**
  *  \file test/ui/widgets/icongridtest.cpp
  *  \brief Test for ui::widgets::IconGrid
  */

#include "ui/widgets/icongrid.hpp"

#include "afl/test/testrunner.hpp"
#include "gfx/nullengine.hpp"

/** Test initialisation behaviour. */
AFL_TEST("ui.widgets.IconGrid:init", a)
{
    // IconGrid containing 5x3 icons of 20x10 each
    gfx::NullEngine engine;
    ui::widgets::IconGrid testee(engine, gfx::Point(20, 10), 5, 3);
    testee.setPadding(3);

    // Each icon now is (3+20+3)+1=27 pixels wide and (3+10+3)+1=17 pixels tall,
    // giving a width of 27*5+1 = 136 pixels, and a height of 17*3+1 = 51 pixels.
    ui::layout::Info info = testee.getLayoutInfo();
    a.checkEqual("01. getPreferredSize", info.getPreferredSize(), gfx::Point(136, 52));
    a.checkEqual("02. isGrowHorizontal", info.isGrowHorizontal(), false);
    a.checkEqual("03. isGrowVertical",   info.isGrowVertical(), true);

    // We don't have any icons yet, so total size is 0
    a.checkEqual("11. getTotalSize",  testee.getTotalSize(), 0);
    a.checkEqual("12. getPageTop",    testee.getPageTop(), 0);

    // Setting dimensions will make getPageSize() report the preferred height
    testee.setExtent(gfx::Rectangle(10, 10, 136, 52));
    a.checkEqual("21. getPageSize", testee.getPageSize(), 3);

    // Add some icons
    // - one at the end of the first line
    testee.setIcon(4, 0, 0);
    a.checkEqual("31. getTotalSize", testee.getTotalSize(), 1);

    // - two more
    testee.addIcon(0);
    testee.addIcon(0);
    a.checkEqual("41. getTotalSize", testee.getTotalSize(), 2);
}

/** Test scrolling behaviour. */
AFL_TEST("ui.widgets.IconGrid:scroll", a)
{
    // IconGrid containing 5x3 icons of 20x10 each
    gfx::NullEngine engine;
    ui::widgets::IconGrid testee(engine, gfx::Point(20, 10), 5, 3);
    testee.setPadding(3);
    testee.setExtent(gfx::Rectangle(10, 10, 136, 52));

    // Add an icon at position (1,2), making the layout look like this:
    //  x x x x x
    //  x x x x x
    //  x x
    testee.setIcon(1, 2, 0);
    a.checkEqual("01. getTotalSize",     testee.getTotalSize(), 3);
    a.checkEqual("02. getCurrentItem",   testee.getCurrentItem(), 0U);
    a.checkEqual("03. getCurrentLine",   testee.getCurrentLine(), 0);
    a.checkEqual("04. getCurrentColumn", testee.getCurrentColumn(), 0);

    // Go down
    testee.scroll(ui::ScrollableWidget::LineDown);
    a.checkEqual("11. getCurrentItem",   testee.getCurrentItem(), 5U);
    a.checkEqual("12. getCurrentLine",   testee.getCurrentLine(), 1);
    a.checkEqual("13. getCurrentColumn", testee.getCurrentColumn(), 0);

    // Place cursor
    testee.setCurrentItem(4, 1);
    a.checkEqual("21. getCurrentItem",   testee.getCurrentItem(), 9U);
    a.checkEqual("22. getCurrentLine",   testee.getCurrentLine(), 1);
    a.checkEqual("23. getCurrentColumn", testee.getCurrentColumn(), 4);

    // Go down again: this is too far so it is limited
    testee.scroll(ui::ScrollableWidget::LineDown);
    a.checkEqual("31. getCurrentItem",   testee.getCurrentItem(), 9U);
    a.checkEqual("32. getCurrentLine",   testee.getCurrentLine(), 1);
    a.checkEqual("33. getCurrentColumn", testee.getCurrentColumn(), 4);

    // Place cursor
    testee.setCurrentItem(11);
    a.checkEqual("41. getCurrentItem",   testee.getCurrentItem(), 11U);
    a.checkEqual("42. getCurrentLine",   testee.getCurrentLine(), 2);
    a.checkEqual("43. getCurrentColumn", testee.getCurrentColumn(), 1);

    // Go up
    testee.scroll(ui::ScrollableWidget::LineUp);
    a.checkEqual("51. getCurrentItem",   testee.getCurrentItem(), 6U);
    a.checkEqual("52. getCurrentLine",   testee.getCurrentLine(), 1);
    a.checkEqual("53. getCurrentColumn", testee.getCurrentColumn(), 1);

    // Cannot set page top
    testee.setPageTop(1);
    a.checkEqual("61. getPageTop", testee.getPageTop(), 0);
}

/** Test key behaviour on widget containing just a single line.
    In this case, vertical movement keys are not accepted. */
AFL_TEST("ui.widgets.IconGrid:handleKey:single-line", a)
{
    // IconGrid containing 5x3 icons of 20x10 each
    gfx::NullEngine engine;
    ui::widgets::IconGrid testee(engine, gfx::Point(20, 10), 5, 3);
    testee.setPadding(3);
    testee.setExtent(gfx::Rectangle(10, 10, 136, 52));
    testee.setState(ui::Widget::FocusedState, true);

    // Add an icon at position (4,0), producing a single line
    testee.setIcon(4, 0, 0);
    a.checkEqual("01. getTotalSize",     testee.getTotalSize(), 1);
    a.checkEqual("02. getCurrentItem",   testee.getCurrentItem(), 0U);
    a.checkEqual("03. getCurrentLine",   testee.getCurrentLine(), 0);
    a.checkEqual("04. getCurrentColumn", testee.getCurrentColumn(), 0);

    // Up and down keys are not accepted
    a.check("11. Up",   !testee.handleKey(util::Key_Up, 0));
    a.check("12. Down", !testee.handleKey(util::Key_Down, 0));
    a.check("13. PgUp", !testee.handleKey(util::Key_PgUp, 0));
    a.check("14. PgDn", !testee.handleKey(util::Key_PgDn, 0));

    // Right
    a.check("21. Right", testee.handleKey(util::Key_Right, 0));
    a.checkEqual("22. getCurrentItem", testee.getCurrentItem(), 1U);

    // Left
    a.check("31. Left", testee.handleKey(util::Key_Left, 0));
    a.checkEqual("32. getCurrentItem", testee.getCurrentItem(), 0U);
}

/** Test key behaviour on widget containing multiple lines.
    All movement keys are not accepted. */
AFL_TEST("ui.widgets.IconGrid:handleKey:multiple-lines", a)
{
    // IconGrid containing 5x3 icons of 20x10 each
    gfx::NullEngine engine;
    ui::widgets::IconGrid testee(engine, gfx::Point(20, 10), 5, 3);
    testee.setPadding(3);
    testee.setExtent(gfx::Rectangle(10, 10, 136, 52));
    testee.setState(ui::Widget::FocusedState, true);

    // Add an icon at position (4,2), producing three lines
    testee.setIcon(4, 2, 0);
    a.checkEqual("01. getTotalSize",     testee.getTotalSize(), 3);
    a.checkEqual("02. getCurrentItem",   testee.getCurrentItem(), 0U);
    a.checkEqual("03. getCurrentLine",   testee.getCurrentLine(), 0);
    a.checkEqual("04. getCurrentColumn", testee.getCurrentColumn(), 0);

    // Down
    a.check("11. Down", testee.handleKey(util::Key_Down, 0));
    a.checkEqual("12. getCurrentItem", testee.getCurrentItem(), 5U);

    // Right
    a.check("21. Right", testee.handleKey(util::Key_Right, 0));
    a.checkEqual("22. getCurrentItem", testee.getCurrentItem(), 6U);

    // Up
    a.check("31. Up", testee.handleKey(util::Key_Up, 0));
    a.checkEqual("32. getCurrentItem", testee.getCurrentItem(), 1U);

    // Left
    a.check("41. Left", testee.handleKey(util::Key_Left, 0));
    a.checkEqual("42. getCurrentItem", testee.getCurrentItem(), 0U);

    // End
    a.check("51. End", testee.handleKey(util::Key_End, 0));
    a.checkEqual("52. getCurrentItem", testee.getCurrentItem(), 14U);

    // Home
    a.check("61. Home", testee.handleKey(util::Key_Home, 0));
    a.checkEqual("62. getCurrentItem", testee.getCurrentItem(), 0U);
}

/** Test setPageTop() and related methods. */
AFL_TEST("ui.widgets.IconGrid:setPageTop", a)
{
    // IconGrid containing 5x3 icons of 20x10 each
    gfx::NullEngine engine;
    ui::widgets::IconGrid testee(engine, gfx::Point(20, 10), 5, 3);
    testee.setPadding(3);
    testee.setExtent(gfx::Rectangle(10, 10, 136, 52));

    // Make it contain 23 icons
    testee.setIcon(2, 4, 0);
    a.checkEqual("01. getTotalSize",     testee.getTotalSize(), 5);
    a.checkEqual("02. getCurrentItem",   testee.getCurrentItem(), 0U);
    a.checkEqual("03. getCurrentLine",   testee.getCurrentLine(), 0);
    a.checkEqual("04. getCurrentColumn", testee.getCurrentColumn(), 0);

    // Maximum page top is 2
    testee.setPageTop(100);
    a.checkEqual("11. getPageTop", testee.getPageTop(), 2);
    testee.setPageTop(1);
    a.checkEqual("12. getPageTop", testee.getPageTop(), 1);

    // Place cursor to set page top
    testee.setCurrentItem(2, 4);
    a.checkEqual("21. getPageTop", testee.getPageTop(), 2);
    a.checkEqual("22. getCurrentItem", testee.getCurrentItem(), 22U);

    testee.setCurrentItem(1);
    a.checkEqual("31. getPageTop", testee.getPageTop(), 0);
    a.checkEqual("32. getCurrentItem", testee.getCurrentItem(), 1U);
}

/** Test handling of inaccessible items. */
AFL_TEST("ui.widgets.IconGrid:inaccessible", a)
{
    // IconGrid containing 3x3 icons of 10x10 each:
    //   . x x
    //   x . x
    //   x x .
    // (similar to Alliance Grid)
    gfx::NullEngine engine;
    ui::widgets::IconGrid testee(engine, gfx::Point(10, 10), 3, 3);
    testee.setState(ui::Widget::FocusedState, true);
    testee.setIcon(2, 2, 0);
    testee.setItemAccessible(0, 0, false);
    testee.setItemAccessible(1, 1, false);
    testee.setItemAccessible(2, 2, false);
    testee.setCurrentItem(1);
    a.checkEqual("01. getTotalSize", testee.getTotalSize(), 3);

    // Down; skips an item
    a.check("11. Down", testee.handleKey(util::Key_Down, 0));
    a.checkEqual("12. getCurrentItem",   testee.getCurrentItem(), 7U);
    a.checkEqual("13. getCurrentLine",   testee.getCurrentLine(), 2);
    a.checkEqual("14. getCurrentColumn", testee.getCurrentColumn(), 1);

    // Cannot go further down
    a.check("21. Down", !testee.handleKey(util::Key_Down, 0));
    a.checkEqual("22. getCurrentItem",   testee.getCurrentItem(), 7U);
    a.checkEqual("23. getCurrentLine",   testee.getCurrentLine(), 2);
    a.checkEqual("24. getCurrentColumn", testee.getCurrentColumn(), 1);

    // Cannot go right
    a.check("31. Right", !testee.handleKey(util::Key_Right, 0));
    a.checkEqual("32. getCurrentItem",   testee.getCurrentItem(), 7U);
    a.checkEqual("33. getCurrentLine",   testee.getCurrentLine(), 2);
    a.checkEqual("34. getCurrentColumn", testee.getCurrentColumn(), 1);

    // Go left thrice
    a.check("41. Left", testee.handleKey(util::Key_Left, 0));
    a.check("42. Left", testee.handleKey(util::Key_Left, 0));
    a.check("43. Left", testee.handleKey(util::Key_Left, 0));
    a.checkEqual("44. getCurrentItem",   testee.getCurrentItem(), 3U);
    a.checkEqual("45. getCurrentLine",   testee.getCurrentLine(), 1);
    a.checkEqual("46. getCurrentColumn", testee.getCurrentColumn(), 0);
}
