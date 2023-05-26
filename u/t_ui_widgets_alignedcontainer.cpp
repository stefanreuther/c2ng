/**
  *  \file u/t_ui_widgets_alignedcontainer.cpp
  *  \brief Test for ui::widgets::AlignedContainer
  */

#include "ui/widgets/alignedcontainer.hpp"

#include "t_ui_widgets.hpp"
#include "ui/spacer.hpp"

void
TestUiWidgetsAlignedContainer::testIt()
{
    // Content widget
    ui::Spacer content(ui::layout::Info(gfx::Point(300, 200), ui::layout::Info::GrowBoth));

    // Object under test
    ui::widgets::AlignedContainer testee(content, gfx::RightAlign, gfx::MiddleAlign);
    testee.setPadding(30, 7);

    // Verify layout
    ui::layout::Info layoutResult = testee.getLayoutInfo();
    TS_ASSERT_EQUALS(layoutResult.getPreferredSize(), gfx::Point(360, 214));
    TS_ASSERT_EQUALS(layoutResult.getGrowthBehaviour(), ui::layout::Info::GrowBoth);

    // Give it more than it wants
    testee.setExtent(gfx::Rectangle(10, 5, 400, 500));
    TS_ASSERT_EQUALS(content.getExtent(), gfx::Rectangle(80, 155, 300, 200));

    // Give it its preferred width, and slightly more than preferred height
    testee.setExtent(gfx::Rectangle(5, 10, 360, 202));
    TS_ASSERT_EQUALS(content.getExtent(), gfx::Rectangle(35, 11, 300, 200));

    // Give it less than preferred
    testee.setExtent(gfx::Rectangle(20, 20, 70, 60));
    TS_ASSERT_EQUALS(content.getExtent(), gfx::Rectangle(20, 20, 70, 60));
}

