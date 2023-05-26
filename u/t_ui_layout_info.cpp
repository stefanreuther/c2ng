/**
  *  \file u/t_ui_layout_info.cpp
  *  \brief Test for ui::layout::Info
  */

#include "ui/layout/info.hpp"

#include "t_ui_layout.hpp"

using ui::layout::Info;

/** Test initialisation: general case. */
void
TestUiLayoutInfo::testInit()
{
    Info a(gfx::Point(4,5), Info::GrowHorizontal);
    TS_ASSERT_EQUALS(a.getPreferredSize().getX(), 4);
    TS_ASSERT_EQUALS(a.getPreferredSize().getY(), 5);
    TS_ASSERT_EQUALS(a.getGrowthBehaviour(), Info::GrowHorizontal);
    TS_ASSERT(a.isGrowHorizontal());
    TS_ASSERT(!a.isGrowVertical());
    TS_ASSERT(!a.isIgnored());
}

/** Test initialisation: fixed size. */
void
TestUiLayoutInfo::testInitFixed()
{
    Info a(gfx::Point(7,8));
    TS_ASSERT_EQUALS(a.getPreferredSize().getX(), 7);
    TS_ASSERT_EQUALS(a.getPreferredSize().getY(), 8);
    TS_ASSERT_EQUALS(a.getGrowthBehaviour(), Info::Fixed);
    TS_ASSERT(!a.isGrowHorizontal());
    TS_ASSERT(!a.isGrowVertical());
    TS_ASSERT(!a.isIgnored());
}

/** Test initialisation: ignored widget. */
void
TestUiLayoutInfo::testInitIgnored()
{
    Info a;
    TS_ASSERT_EQUALS(a.getPreferredSize().getX(), 0);
    TS_ASSERT_EQUALS(a.getPreferredSize().getY(), 0);
    TS_ASSERT_EQUALS(a.getGrowthBehaviour(), Info::NoLayout);
    TS_ASSERT(!a.isGrowHorizontal());
    TS_ASSERT(!a.isGrowVertical());
    TS_ASSERT(a.isIgnored());
}

/** Test andGrowthBehaviour(). */
void
TestUiLayoutInfo::testAnd()
{
    // NoLayout is neutral element
    TS_ASSERT_EQUALS(Info::andGrowthBehaviour(Info::NoLayout,       Info::NoLayout),       Info::NoLayout);
    TS_ASSERT_EQUALS(Info::andGrowthBehaviour(Info::Fixed,          Info::NoLayout),       Info::Fixed);
    TS_ASSERT_EQUALS(Info::andGrowthBehaviour(Info::GrowHorizontal, Info::NoLayout),       Info::GrowHorizontal);
    TS_ASSERT_EQUALS(Info::andGrowthBehaviour(Info::GrowVertical,   Info::NoLayout),       Info::GrowVertical);
    TS_ASSERT_EQUALS(Info::andGrowthBehaviour(Info::GrowBoth,       Info::NoLayout),       Info::GrowBoth);

    // Fixed wins against everything
    TS_ASSERT_EQUALS(Info::andGrowthBehaviour(Info::NoLayout,       Info::Fixed),          Info::Fixed);
    TS_ASSERT_EQUALS(Info::andGrowthBehaviour(Info::Fixed,          Info::Fixed),          Info::Fixed);
    TS_ASSERT_EQUALS(Info::andGrowthBehaviour(Info::GrowHorizontal, Info::Fixed),          Info::Fixed);
    TS_ASSERT_EQUALS(Info::andGrowthBehaviour(Info::GrowVertical,   Info::Fixed),          Info::Fixed);
    TS_ASSERT_EQUALS(Info::andGrowthBehaviour(Info::GrowBoth,       Info::Fixed),          Info::Fixed);

    // GrowHorizontal cancels GrowVertical/Both
    TS_ASSERT_EQUALS(Info::andGrowthBehaviour(Info::NoLayout,       Info::GrowHorizontal), Info::GrowHorizontal);
    TS_ASSERT_EQUALS(Info::andGrowthBehaviour(Info::Fixed,          Info::GrowHorizontal), Info::Fixed);
    TS_ASSERT_EQUALS(Info::andGrowthBehaviour(Info::GrowHorizontal, Info::GrowHorizontal), Info::GrowHorizontal);
    TS_ASSERT_EQUALS(Info::andGrowthBehaviour(Info::GrowVertical,   Info::GrowHorizontal), Info::Fixed);
    TS_ASSERT_EQUALS(Info::andGrowthBehaviour(Info::GrowBoth,       Info::GrowHorizontal), Info::GrowHorizontal);

    // GrowVertical cancels GrowHorizontal/Both
    TS_ASSERT_EQUALS(Info::andGrowthBehaviour(Info::NoLayout,       Info::GrowVertical),   Info::GrowVertical);
    TS_ASSERT_EQUALS(Info::andGrowthBehaviour(Info::Fixed,          Info::GrowVertical),   Info::Fixed);
    TS_ASSERT_EQUALS(Info::andGrowthBehaviour(Info::GrowHorizontal, Info::GrowVertical),   Info::Fixed);
    TS_ASSERT_EQUALS(Info::andGrowthBehaviour(Info::GrowVertical,   Info::GrowVertical),   Info::GrowVertical);
    TS_ASSERT_EQUALS(Info::andGrowthBehaviour(Info::GrowBoth,       Info::GrowVertical),   Info::GrowVertical);

    // GrowBoth
    TS_ASSERT_EQUALS(Info::andGrowthBehaviour(Info::NoLayout,       Info::GrowBoth),       Info::GrowBoth);
    TS_ASSERT_EQUALS(Info::andGrowthBehaviour(Info::Fixed,          Info::GrowBoth),       Info::Fixed);
    TS_ASSERT_EQUALS(Info::andGrowthBehaviour(Info::GrowHorizontal, Info::GrowBoth),       Info::GrowHorizontal);
    TS_ASSERT_EQUALS(Info::andGrowthBehaviour(Info::GrowVertical,   Info::GrowBoth),       Info::GrowVertical);
    TS_ASSERT_EQUALS(Info::andGrowthBehaviour(Info::GrowBoth,       Info::GrowBoth),       Info::GrowBoth);
}

/** Test makeGrowthBehaviour(). */
void
TestUiLayoutInfo::testMake()
{
    TS_ASSERT_EQUALS(Info::makeGrowthBehaviour(false, false, false), Info::Fixed);
    TS_ASSERT_EQUALS(Info::makeGrowthBehaviour(false, true,  false), Info::GrowVertical);
    TS_ASSERT_EQUALS(Info::makeGrowthBehaviour(true,  false, false), Info::GrowHorizontal);
    TS_ASSERT_EQUALS(Info::makeGrowthBehaviour(true,  true,  false), Info::GrowBoth);
    TS_ASSERT_EQUALS(Info::makeGrowthBehaviour(false, false, true),  Info::NoLayout);
}

