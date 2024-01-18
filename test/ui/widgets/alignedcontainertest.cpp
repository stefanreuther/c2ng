/**
  *  \file test/ui/widgets/alignedcontainertest.cpp
  *  \brief Test for ui::widgets::AlignedContainer
  */

#include "ui/widgets/alignedcontainer.hpp"

#include "afl/test/testrunner.hpp"
#include "ui/spacer.hpp"

AFL_TEST("ui.widgets.AlignedContainer", a)
{
    // Content widget
    ui::Spacer content(ui::layout::Info(gfx::Point(300, 200), ui::layout::Info::GrowBoth));

    // Object under test
    ui::widgets::AlignedContainer testee(content, gfx::RightAlign, gfx::MiddleAlign);
    testee.setPadding(30, 7);

    // Verify layout
    ui::layout::Info layoutResult = testee.getLayoutInfo();
    a.checkEqual("01. getPreferredSize", layoutResult.getPreferredSize(), gfx::Point(360, 214));
    a.checkEqual("02. getGrowthBehaviour", layoutResult.getGrowthBehaviour(), ui::layout::Info::GrowBoth);

    // Give it more than it wants
    testee.setExtent(gfx::Rectangle(10, 5, 400, 500));
    a.checkEqual("11. getExtent", content.getExtent(), gfx::Rectangle(80, 155, 300, 200));

    // Give it its preferred width, and slightly more than preferred height
    testee.setExtent(gfx::Rectangle(5, 10, 360, 202));
    a.checkEqual("21. getExtent", content.getExtent(), gfx::Rectangle(35, 11, 300, 200));

    // Give it less than preferred
    testee.setExtent(gfx::Rectangle(20, 20, 70, 60));
    a.checkEqual("31. getExtent", content.getExtent(), gfx::Rectangle(20, 20, 70, 60));
}
