/**
  *  \file test/ui/layout/vboxtest.cpp
  *  \brief Test for ui::layout::VBox
  */

#include "ui/layout/vbox.hpp"

#include "afl/test/testrunner.hpp"
#include "ui/group.hpp"
#include "ui/spacer.hpp"

using ui::layout::Info;
using gfx::Point;
using gfx::Rectangle;

/** Generic test. */
AFL_TEST("ui.layout.VBox:basics", a)
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
    a.checkEqual("01. getPreferredSize",   li.getPreferredSize(), Point(30, 108 /*3 + 10 + 7 + 50 + 7 + 12 + 7 + 9 + 3*/));
    a.checkEqual("02. getGrowthBehaviour", li.getGrowthBehaviour(), Info::GrowVertical);
    a.checkEqual("03. isGrowHorizontal",   li.isGrowHorizontal(), false);
    a.checkEqual("04. isGrowVertical",     li.isGrowVertical(), true);
    a.checkEqual("05. isIgnored",          li.isIgnored(), false);

    // Give it the desired space; verify
    g.setExtent(Rectangle(100, 200, 30, 108));
    a.checkEqual("11. getExtent", s1.getExtent(), Rectangle(100, 203, 30, 10));
    a.checkEqual("12. getExtent", s2.getExtent(), Rectangle(100, 220, 30, 50));
    a.checkEqual("13. getExtent", s3.getExtent(), Rectangle(100, 277, 30, 12));
    a.checkEqual("14. getExtent", s4.getExtent(), Rectangle(100, 296, 30,  9));

    // Give it too much space
    g.setExtent(Rectangle(100, 200, 40, 300));
    a.checkEqual("21. getExtent", s1.getExtent(), Rectangle(100, 203, 40,  10));
    a.checkEqual("22. getExtent", s2.getExtent(), Rectangle(100, 220, 40, 242));
    a.checkEqual("23. getExtent", s3.getExtent(), Rectangle(100, 469, 40,  12));
    a.checkEqual("24. getExtent", s4.getExtent(), Rectangle(100, 488, 40,   9));

    // Give it too little space; starts by reducing margins
    g.setExtent(Rectangle(100, 200, 20, 100));
    a.checkEqual("31. getExtent", s1.getExtent(), Rectangle(100, 203, 20, 10));
    a.checkEqual("32. getExtent", s2.getExtent(), Rectangle(100, 218, 20, 50));
    a.checkEqual("33. getExtent", s3.getExtent(), Rectangle(100, 272, 20, 12));
    a.checkEqual("34. getExtent", s4.getExtent(), Rectangle(100, 288, 20,  9));

    // Give it even less space; reduces margins to zero and reduce the flexible component
    g.setExtent(Rectangle(100, 200, 20, 70));
    a.checkEqual("41. getExtent", s1.getExtent(), Rectangle(100, 200, 20, 10));
    a.checkEqual("42. getExtent", s2.getExtent(), Rectangle(100, 210, 20, 39));
    a.checkEqual("43. getExtent", s3.getExtent(), Rectangle(100, 249, 20, 12));
    a.checkEqual("44. getExtent", s4.getExtent(), Rectangle(100, 261, 20,  9));

    // Give it even less space so it now needs to reduce even the fixed components
    g.setExtent(Rectangle(100, 200, 20, 20));
    a.checkEqual("51. getExtent", s1.getExtent(), Rectangle(100, 200, 20, 6));
    a.checkEqual("52. getExtent", s2.getExtent(), Rectangle(100, 206, 20, 0));
    a.checkEqual("53. getExtent", s3.getExtent(), Rectangle(100, 206, 20, 9));
    a.checkEqual("54. getExtent", s4.getExtent(), Rectangle(100, 215, 20, 5));
}

/** Verify behaviour on empty group. */
AFL_TEST("ui.layout.VBox:empty", a)
{
    ui::layout::VBox testee(7, 3);
    ui::Group g(testee);

    Info li = g.getLayoutInfo();
    a.checkEqual("01. getPreferredSize",   li.getPreferredSize(), Point(0, 6));
    a.checkEqual("02. getGrowthBehaviour", li.getGrowthBehaviour(), Info::NoLayout);
    a.checkEqual("03. isGrowHorizontal",   li.isGrowHorizontal(), false);
    a.checkEqual("04. isGrowVertical",     li.isGrowVertical(), false);
    a.checkEqual("05. isIgnored",          li.isIgnored(), true);
}

/** Verify behaviour with single fixed content widget. */
AFL_TEST("ui.layout.VBox:single", a)
{
    ui::Spacer s1(Point(35, 20));

    ui::layout::VBox testee(2, 5);
    ui::Group g(testee);
    g.add(s1);

    Info li = g.getLayoutInfo();
    a.checkEqual("01. getPreferredSize",   li.getPreferredSize(), Point(35, 30));
    a.checkEqual("02. getGrowthBehaviour", li.getGrowthBehaviour(), Info::Fixed);
    a.checkEqual("03. isGrowHorizontal",   li.isGrowHorizontal(), false);
    a.checkEqual("04. isGrowVertical",     li.isGrowVertical(), false);
    a.checkEqual("05. isIgnored",          li.isIgnored(), false);

    // Give it the desired space; verify
    g.setExtent(Rectangle(100, 200, 35, 30));
    a.checkEqual("11. getExtent", s1.getExtent(), Rectangle(100, 205, 35, 20));

    // Give it too much space
    g.setExtent(Rectangle(100, 200, 300, 40));
    a.checkEqual("21. getExtent", s1.getExtent(), Rectangle(100, 205, 300, 30));

    // Give it too little space
    g.setExtent(Rectangle(100, 200, 35, 24));
    a.checkEqual("31. getExtent", s1.getExtent(), Rectangle(100, 202, 35, 20));

    // Give it way too little space
    g.setExtent(Rectangle(100, 200, 40, 10));
    a.checkEqual("41. getExtent", s1.getExtent(), Rectangle(100, 200, 40, 10));
}
