/**
  *  \file u/t_ui_widgets_icongrid.cpp
  *  \brief Test for ui::widgets::IconGrid
  */

#include "ui/widgets/icongrid.hpp"

#include "t_ui_widgets.hpp"
#include "gfx/nullengine.hpp"

/** Test initialisation behaviour. */
void
TestUiWidgetsIconGrid::testInit()
{
    // IconGrid containing 5x3 icons of 20x10 each
    gfx::NullEngine engine;
    ui::widgets::IconGrid testee(engine, gfx::Point(20, 10), 5, 3);
    testee.setPadding(3);

    // Each icon now is (3+20+3)+1=27 pixels wide and (3+10+3)+1=17 pixels tall,
    // giving a width of 27*5+1 = 136 pixels, and a height of 17*3+1 = 51 pixels.
    ui::layout::Info info = testee.getLayoutInfo();
    TS_ASSERT_EQUALS(info.getMinSize(), gfx::Point(136, 52));
    TS_ASSERT_EQUALS(info.getPreferredSize(), gfx::Point(136, 52));
    TS_ASSERT_EQUALS(info.isGrowHorizontal(), false);
    TS_ASSERT_EQUALS(info.isGrowVertical(), true);

    // We don't have any icons yet, so total size is 0
    TS_ASSERT_EQUALS(testee.getTotalSize(), 0);
    TS_ASSERT_EQUALS(testee.getPageTop(), 0);
    TS_ASSERT_EQUALS(testee.getCursorTop(), 0);
    TS_ASSERT_EQUALS(testee.getCursorSize(), 1);

    // Setting dimensions will make getPageSize() report the preferred height
    testee.setExtent(gfx::Rectangle(10, 10, 136, 52));
    TS_ASSERT_EQUALS(testee.getPageSize(), 3);

    // Add some icons
    // - one at the end of the first line
    testee.setIcon(4, 0, 0);
    TS_ASSERT_EQUALS(testee.getTotalSize(), 1);

    // - two more
    testee.addIcon(0);
    testee.addIcon(0);
    TS_ASSERT_EQUALS(testee.getTotalSize(), 2);
}

/** Test scrolling behaviour. */
void
TestUiWidgetsIconGrid::testScroll()
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
    TS_ASSERT_EQUALS(testee.getTotalSize(), 3);
    TS_ASSERT_EQUALS(testee.getCurrentItem(), 0U);
    TS_ASSERT_EQUALS(testee.getCurrentLine(), 0);
    TS_ASSERT_EQUALS(testee.getCurrentColumn(), 0);

    // Go down
    testee.scroll(ui::ScrollableWidget::LineDown);
    TS_ASSERT_EQUALS(testee.getCurrentItem(), 5U);
    TS_ASSERT_EQUALS(testee.getCurrentLine(), 1);
    TS_ASSERT_EQUALS(testee.getCurrentColumn(), 0);

    // Place cursor
    testee.setCurrentItem(4, 1);
    TS_ASSERT_EQUALS(testee.getCurrentItem(), 9U);
    TS_ASSERT_EQUALS(testee.getCurrentLine(), 1);
    TS_ASSERT_EQUALS(testee.getCurrentColumn(), 4);

    // Go down again: this is too far so it is limited
    testee.scroll(ui::ScrollableWidget::LineDown);
    TS_ASSERT_EQUALS(testee.getCurrentItem(), 9U);
    TS_ASSERT_EQUALS(testee.getCurrentLine(), 1);
    TS_ASSERT_EQUALS(testee.getCurrentColumn(), 4);

    // Place cursor
    testee.setCurrentItem(11);
    TS_ASSERT_EQUALS(testee.getCurrentItem(), 11U);
    TS_ASSERT_EQUALS(testee.getCurrentLine(), 2);
    TS_ASSERT_EQUALS(testee.getCurrentColumn(), 1);

    // Go up
    testee.scroll(ui::ScrollableWidget::LineUp);
    TS_ASSERT_EQUALS(testee.getCurrentItem(), 6U);
    TS_ASSERT_EQUALS(testee.getCurrentLine(), 1);
    TS_ASSERT_EQUALS(testee.getCurrentColumn(), 1);

    // Cannot set page top
    testee.setPageTop(1);
    TS_ASSERT_EQUALS(testee.getPageTop(), 0);
}

/** Test key behaviour on widget containing just a single line.
    In this case, vertical movement keys are not accepted. */
void
TestUiWidgetsIconGrid::testKeySingle()
{
    // IconGrid containing 5x3 icons of 20x10 each
    gfx::NullEngine engine;
    ui::widgets::IconGrid testee(engine, gfx::Point(20, 10), 5, 3);
    testee.setPadding(3);
    testee.setExtent(gfx::Rectangle(10, 10, 136, 52));
    testee.setState(ui::Widget::FocusedState, true);

    // Add an icon at position (4,0), producing a single line
    testee.setIcon(4, 0, 0);
    TS_ASSERT_EQUALS(testee.getTotalSize(), 1);
    TS_ASSERT_EQUALS(testee.getCurrentItem(), 0U);
    TS_ASSERT_EQUALS(testee.getCurrentLine(), 0);
    TS_ASSERT_EQUALS(testee.getCurrentColumn(), 0);

    // Up and down keys are not accepted
    TS_ASSERT(!testee.handleKey(util::Key_Up, 0));
    TS_ASSERT(!testee.handleKey(util::Key_Down, 0));
    TS_ASSERT(!testee.handleKey(util::Key_PgUp, 0));
    TS_ASSERT(!testee.handleKey(util::Key_PgDn, 0));

    // Right
    TS_ASSERT(testee.handleKey(util::Key_Right, 0));
    TS_ASSERT_EQUALS(testee.getCurrentItem(), 1U);

    // Left
    TS_ASSERT(testee.handleKey(util::Key_Left, 0));
    TS_ASSERT_EQUALS(testee.getCurrentItem(), 0U);
}

/** Test key behaviour on widget containing multiple lines.
    All movement keys are not accepted. */
void
TestUiWidgetsIconGrid::testKeyMulti()
{
    // IconGrid containing 5x3 icons of 20x10 each
    gfx::NullEngine engine;
    ui::widgets::IconGrid testee(engine, gfx::Point(20, 10), 5, 3);
    testee.setPadding(3);
    testee.setExtent(gfx::Rectangle(10, 10, 136, 52));
    testee.setState(ui::Widget::FocusedState, true);

    // Add an icon at position (4,2), producing three lines
    testee.setIcon(4, 2, 0);
    TS_ASSERT_EQUALS(testee.getTotalSize(), 3);
    TS_ASSERT_EQUALS(testee.getCurrentItem(), 0U);
    TS_ASSERT_EQUALS(testee.getCurrentLine(), 0);
    TS_ASSERT_EQUALS(testee.getCurrentColumn(), 0);

    // Down
    TS_ASSERT(testee.handleKey(util::Key_Down, 0));
    TS_ASSERT_EQUALS(testee.getCurrentItem(), 5U);

    // Right
    TS_ASSERT(testee.handleKey(util::Key_Right, 0));
    TS_ASSERT_EQUALS(testee.getCurrentItem(), 6U);

    // Up
    TS_ASSERT(testee.handleKey(util::Key_Up, 0));
    TS_ASSERT_EQUALS(testee.getCurrentItem(), 1U);

    // Left
    TS_ASSERT(testee.handleKey(util::Key_Left, 0));
    TS_ASSERT_EQUALS(testee.getCurrentItem(), 0U);

    // End
    TS_ASSERT(testee.handleKey(util::Key_End, 0));
    TS_ASSERT_EQUALS(testee.getCurrentItem(), 14U);

    // Left
    TS_ASSERT(testee.handleKey(util::Key_Home, 0));
    TS_ASSERT_EQUALS(testee.getCurrentItem(), 0U);
}

/** Test setPageTop() and related methods. */
void
TestUiWidgetsIconGrid::testScrollPageTop()
{
    // IconGrid containing 5x3 icons of 20x10 each
    gfx::NullEngine engine;
    ui::widgets::IconGrid testee(engine, gfx::Point(20, 10), 5, 3);
    testee.setPadding(3);
    testee.setExtent(gfx::Rectangle(10, 10, 136, 52));

    // Make it contain 23 icons
    testee.setIcon(2, 4, 0);
    TS_ASSERT_EQUALS(testee.getTotalSize(), 5);
    TS_ASSERT_EQUALS(testee.getCurrentItem(), 0U);
    TS_ASSERT_EQUALS(testee.getCurrentLine(), 0);
    TS_ASSERT_EQUALS(testee.getCurrentColumn(), 0);

    // Maximum page top is 2
    testee.setPageTop(100);
    TS_ASSERT_EQUALS(testee.getPageTop(), 2);
    testee.setPageTop(1);
    TS_ASSERT_EQUALS(testee.getPageTop(), 1);

    // Place cursor to set page top
    testee.setCurrentItem(2, 4);
    TS_ASSERT_EQUALS(testee.getPageTop(), 2);
    TS_ASSERT_EQUALS(testee.getCurrentItem(), 22U);

    testee.setCurrentItem(1);
    TS_ASSERT_EQUALS(testee.getPageTop(), 0);
    TS_ASSERT_EQUALS(testee.getCurrentItem(), 1U);
}

/** Test handling of inaccessible items. */
void
TestUiWidgetsIconGrid::testInaccessible()
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
    TS_ASSERT_EQUALS(testee.getTotalSize(), 3)

    // Down; skips an item
    TS_ASSERT(testee.handleKey(util::Key_Down, 0));
    TS_ASSERT_EQUALS(testee.getCurrentItem(), 7U);
    TS_ASSERT_EQUALS(testee.getCurrentLine(), 2);
    TS_ASSERT_EQUALS(testee.getCurrentColumn(), 1);

    // Cannot go further down
    TS_ASSERT(!testee.handleKey(util::Key_Down, 0));
    TS_ASSERT_EQUALS(testee.getCurrentItem(), 7U);
    TS_ASSERT_EQUALS(testee.getCurrentLine(), 2);
    TS_ASSERT_EQUALS(testee.getCurrentColumn(), 1);

    // Cannot go right
    TS_ASSERT(!testee.handleKey(util::Key_Right, 0));
    TS_ASSERT_EQUALS(testee.getCurrentItem(), 7U);
    TS_ASSERT_EQUALS(testee.getCurrentLine(), 2);
    TS_ASSERT_EQUALS(testee.getCurrentColumn(), 1);

    // Go left thrice
    TS_ASSERT(testee.handleKey(util::Key_Left, 0));
    TS_ASSERT(testee.handleKey(util::Key_Left, 0));
    TS_ASSERT(testee.handleKey(util::Key_Left, 0));
    TS_ASSERT_EQUALS(testee.getCurrentItem(), 3U);
    TS_ASSERT_EQUALS(testee.getCurrentLine(), 1);
    TS_ASSERT_EQUALS(testee.getCurrentColumn(), 0);
}

