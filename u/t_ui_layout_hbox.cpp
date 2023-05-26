/**
  *  \file u/t_ui_layout_hbox.cpp
  *  \brief Test for ui::layout::HBox
  */

#include "ui/layout/hbox.hpp"

#include "t_ui_layout.hpp"
#include "ui/group.hpp"
#include "ui/spacer.hpp"

using ui::layout::Info;
using gfx::Point;
using gfx::Rectangle;

/** Generic test. */
void
TestUiLayoutHBox::testIt()
{
    // Widgets: simulating [Button] [Spacer] [Button] [Button]
    ui::Spacer s1(Point(10, 20));
    ui::Spacer s2(Info(Point(50, 0), Info::GrowBoth));
    ui::Spacer s3(Point(12, 25));
    ui::Spacer s4(Point(9, 30));

    ui::layout::HBox testee(7, 3);
    ui::Group g(testee);
    g.add(s1);
    g.add(s2);
    g.add(s3);
    g.add(s4);

    // Verify layout
    Info li = g.getLayoutInfo();
    TS_ASSERT_EQUALS(li.getPreferredSize(), Point(108 /*3 + 10 + 7 + 50 + 7 + 12 + 7 + 9 + 3*/, 30));
    TS_ASSERT_EQUALS(li.getGrowthBehaviour(), Info::GrowHorizontal);
    TS_ASSERT_EQUALS(li.isGrowHorizontal(), true);
    TS_ASSERT_EQUALS(li.isGrowVertical(), false);
    TS_ASSERT_EQUALS(li.isIgnored(), false);

    // Give it the desired space; verify
    g.setExtent(Rectangle(100, 200, 108, 30));
    TS_ASSERT_EQUALS(s1.getExtent(), Rectangle(103, 200, 10, 30));
    TS_ASSERT_EQUALS(s2.getExtent(), Rectangle(120, 200, 50, 30));
    TS_ASSERT_EQUALS(s3.getExtent(), Rectangle(177, 200, 12, 30));
    TS_ASSERT_EQUALS(s4.getExtent(), Rectangle(196, 200,  9, 30));

    // Give it too much space
    g.setExtent(Rectangle(100, 200, 300, 40));
    TS_ASSERT_EQUALS(s1.getExtent(), Rectangle(103, 200,  10, 40));
    TS_ASSERT_EQUALS(s2.getExtent(), Rectangle(120, 200, 242, 40));
    TS_ASSERT_EQUALS(s3.getExtent(), Rectangle(369, 200,  12, 40));
    TS_ASSERT_EQUALS(s4.getExtent(), Rectangle(388, 200,   9, 40));

    // Give it too little space; starts by reducing margins
    g.setExtent(Rectangle(100, 200, 100, 20));
    TS_ASSERT_EQUALS(s1.getExtent(), Rectangle(103, 200, 10, 20));
    TS_ASSERT_EQUALS(s2.getExtent(), Rectangle(118, 200, 50, 20));
    TS_ASSERT_EQUALS(s3.getExtent(), Rectangle(172, 200, 12, 20));
    TS_ASSERT_EQUALS(s4.getExtent(), Rectangle(188, 200,  9, 20));

    // Give it even less space; reduces margins to zero and reduce the flexible component
    g.setExtent(Rectangle(100, 200, 70, 20));
    TS_ASSERT_EQUALS(s1.getExtent(), Rectangle(100, 200, 10, 20));
    TS_ASSERT_EQUALS(s2.getExtent(), Rectangle(110, 200, 39, 20));
    TS_ASSERT_EQUALS(s3.getExtent(), Rectangle(149, 200, 12, 20));
    TS_ASSERT_EQUALS(s4.getExtent(), Rectangle(161, 200,  9, 20));

    // Give it even less space so it now needs to reduce even the fixed components
    g.setExtent(Rectangle(100, 200, 20, 20));
    TS_ASSERT_EQUALS(s1.getExtent(), Rectangle(100, 200, 6, 20));
    TS_ASSERT_EQUALS(s2.getExtent(), Rectangle(106, 200, 0, 20));
    TS_ASSERT_EQUALS(s3.getExtent(), Rectangle(106, 200, 9, 20));
    TS_ASSERT_EQUALS(s4.getExtent(), Rectangle(115, 200, 5, 20));
}

/** Verify behaviour on empty group. */
void
TestUiLayoutHBox::testEmpty()
{
    ui::layout::HBox testee(7, 3);
    ui::Group g(testee);

    Info li = g.getLayoutInfo();
    TS_ASSERT_EQUALS(li.getPreferredSize(), Point(6, 0));
    TS_ASSERT_EQUALS(li.getGrowthBehaviour(), Info::NoLayout);
    TS_ASSERT_EQUALS(li.isGrowHorizontal(), false);
    TS_ASSERT_EQUALS(li.isGrowVertical(), false);
    TS_ASSERT_EQUALS(li.isIgnored(), true);
}

/** Verify behaviour with single fixed content widget. */
void
TestUiLayoutHBox::testSingle()
{
    ui::Spacer s1(Point(30, 20));

    ui::layout::HBox testee(2, 5);
    ui::Group g(testee);
    g.add(s1);

    Info li = g.getLayoutInfo();
    TS_ASSERT_EQUALS(li.getPreferredSize(), Point(40, 20));
    TS_ASSERT_EQUALS(li.getGrowthBehaviour(), Info::Fixed);
    TS_ASSERT_EQUALS(li.isGrowHorizontal(), false);
    TS_ASSERT_EQUALS(li.isGrowVertical(), false);
    TS_ASSERT_EQUALS(li.isIgnored(), false);

    // Give it the desired space; verify
    g.setExtent(Rectangle(100, 200, 40, 20));
    TS_ASSERT_EQUALS(s1.getExtent(), Rectangle(105, 200, 30, 20));

    // Give it too much space
    g.setExtent(Rectangle(100, 200, 300, 40));
    TS_ASSERT_EQUALS(s1.getExtent(), Rectangle(105, 200, 290, 40));

    // Give it too little space
    g.setExtent(Rectangle(100, 200, 34, 40));
    TS_ASSERT_EQUALS(s1.getExtent(), Rectangle(102, 200, 30, 40));

    // Give it way too little space
    g.setExtent(Rectangle(100, 200, 10, 40));
    TS_ASSERT_EQUALS(s1.getExtent(), Rectangle(100, 200, 10, 40));
}

