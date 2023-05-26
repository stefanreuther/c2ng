/**
  *  \file u/t_ui_layout_vbox.cpp
  *  \brief Test for ui::layout::VBox
  */

#include "ui/layout/vbox.hpp"

#include "t_ui_layout.hpp"
#include "ui/group.hpp"
#include "ui/spacer.hpp"

using ui::layout::Info;
using gfx::Point;
using gfx::Rectangle;

/** Generic test. */
void
TestUiLayoutVBox::testIt()
{
    // Widgets: simulating [Button] [Spacer] [Button] [Button]
    // (Same as TestUiLayoutHBox::testIt, with X/Y swapped)
    ui::Spacer s1(Point(20, 10));
    ui::Spacer s2(Info(Point(0, 50), Info::GrowBoth));
    ui::Spacer s3(Point(25, 12));
    ui::Spacer s4(Point(30, 9));

    ui::layout::VBox testee(7, 3);
    ui::Group g(testee);
    g.add(s1);
    g.add(s2);
    g.add(s3);
    g.add(s4);

    // Verify layout
    Info li = g.getLayoutInfo();
    TS_ASSERT_EQUALS(li.getPreferredSize(), Point(30, 108 /*3 + 10 + 7 + 50 + 7 + 12 + 7 + 9 + 3*/));
    TS_ASSERT_EQUALS(li.getGrowthBehaviour(), Info::GrowVertical);
    TS_ASSERT_EQUALS(li.isGrowHorizontal(), false);
    TS_ASSERT_EQUALS(li.isGrowVertical(), true);
    TS_ASSERT_EQUALS(li.isIgnored(), false);

    // Give it the desired space; verify
    g.setExtent(Rectangle(100, 200, 30, 108));
    TS_ASSERT_EQUALS(s1.getExtent(), Rectangle(100, 203, 30, 10));
    TS_ASSERT_EQUALS(s2.getExtent(), Rectangle(100, 220, 30, 50));
    TS_ASSERT_EQUALS(s3.getExtent(), Rectangle(100, 277, 30, 12));
    TS_ASSERT_EQUALS(s4.getExtent(), Rectangle(100, 296, 30,  9));

    // Give it too much space
    g.setExtent(Rectangle(100, 200, 40, 300));
    TS_ASSERT_EQUALS(s1.getExtent(), Rectangle(100, 203, 40,  10));
    TS_ASSERT_EQUALS(s2.getExtent(), Rectangle(100, 220, 40, 242));
    TS_ASSERT_EQUALS(s3.getExtent(), Rectangle(100, 469, 40,  12));
    TS_ASSERT_EQUALS(s4.getExtent(), Rectangle(100, 488, 40,   9));

    // Give it too little space; starts by reducing margins
    g.setExtent(Rectangle(100, 200, 20, 100));
    TS_ASSERT_EQUALS(s1.getExtent(), Rectangle(100, 203, 20, 10));
    TS_ASSERT_EQUALS(s2.getExtent(), Rectangle(100, 218, 20, 50));
    TS_ASSERT_EQUALS(s3.getExtent(), Rectangle(100, 272, 20, 12));
    TS_ASSERT_EQUALS(s4.getExtent(), Rectangle(100, 288, 20,  9));

    // Give it even less space; reduces margins to zero and reduce the flexible component
    g.setExtent(Rectangle(100, 200, 20, 70));
    TS_ASSERT_EQUALS(s1.getExtent(), Rectangle(100, 200, 20, 10));
    TS_ASSERT_EQUALS(s2.getExtent(), Rectangle(100, 210, 20, 39));
    TS_ASSERT_EQUALS(s3.getExtent(), Rectangle(100, 249, 20, 12));
    TS_ASSERT_EQUALS(s4.getExtent(), Rectangle(100, 261, 20,  9));

    // Give it even less space so it now needs to reduce even the fixed components
    g.setExtent(Rectangle(100, 200, 20, 20));
    TS_ASSERT_EQUALS(s1.getExtent(), Rectangle(100, 200, 20, 6));
    TS_ASSERT_EQUALS(s2.getExtent(), Rectangle(100, 206, 20, 0));
    TS_ASSERT_EQUALS(s3.getExtent(), Rectangle(100, 206, 20, 9));
    TS_ASSERT_EQUALS(s4.getExtent(), Rectangle(100, 215, 20, 5));
}

/** Verify behaviour on empty group. */
void
TestUiLayoutVBox::testEmpty()
{
    ui::layout::VBox testee(7, 3);
    ui::Group g(testee);

    Info li = g.getLayoutInfo();
    TS_ASSERT_EQUALS(li.getPreferredSize(), Point(0, 6));
    TS_ASSERT_EQUALS(li.getGrowthBehaviour(), Info::NoLayout);
    TS_ASSERT_EQUALS(li.isGrowHorizontal(), false);
    TS_ASSERT_EQUALS(li.isGrowVertical(), false);
    TS_ASSERT_EQUALS(li.isIgnored(), true);
}

/** Verify behaviour with single fixed content widget. */
void
TestUiLayoutVBox::testSingle()
{
    ui::Spacer s1(Point(35, 20));

    ui::layout::VBox testee(2, 5);
    ui::Group g(testee);
    g.add(s1);

    Info li = g.getLayoutInfo();
    TS_ASSERT_EQUALS(li.getPreferredSize(), Point(35, 30));
    TS_ASSERT_EQUALS(li.getGrowthBehaviour(), Info::Fixed);
    TS_ASSERT_EQUALS(li.isGrowHorizontal(), false);
    TS_ASSERT_EQUALS(li.isGrowVertical(), false);
    TS_ASSERT_EQUALS(li.isIgnored(), false);

    // Give it the desired space; verify
    g.setExtent(Rectangle(100, 200, 35, 30));
    TS_ASSERT_EQUALS(s1.getExtent(), Rectangle(100, 205, 35, 20));

    // Give it too much space
    g.setExtent(Rectangle(100, 200, 300, 40));
    TS_ASSERT_EQUALS(s1.getExtent(), Rectangle(100, 205, 300, 30));

    // Give it too little space
    g.setExtent(Rectangle(100, 200, 35, 24));
    TS_ASSERT_EQUALS(s1.getExtent(), Rectangle(100, 202, 35, 20));

    // Give it way too little space
    g.setExtent(Rectangle(100, 200, 40, 10));
    TS_ASSERT_EQUALS(s1.getExtent(), Rectangle(100, 200, 40, 10));
}

