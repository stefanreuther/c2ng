/**
  *  \file test/ui/layout/infotest.cpp
  *  \brief Test for ui::layout::Info
  */

#include "ui/layout/info.hpp"
#include "afl/test/testrunner.hpp"

using ui::layout::Info;

/** Test initialisation: general case. */
AFL_TEST("ui.layout.Info:init", a)
{
    Info li(gfx::Point(4,5), Info::GrowHorizontal);
    a.checkEqual("01",                     li.getPreferredSize().getX(), 4);
    a.checkEqual("02",                     li.getPreferredSize().getY(), 5);
    a.checkEqual("03. getGrowthBehaviour", li.getGrowthBehaviour(), Info::GrowHorizontal);
    a.check     ("04. isGrowHorizontal",   li.isGrowHorizontal());
    a.check     ("05. isGrowVertical",    !li.isGrowVertical());
    a.check     ("06. isIgnored",         !li.isIgnored());
}

/** Test initialisation: fixed size. */
AFL_TEST("ui.layout.Info:init:fixed", a)
{
    Info li(gfx::Point(7,8));
    a.checkEqual("01",                     li.getPreferredSize().getX(), 7);
    a.checkEqual("02",                     li.getPreferredSize().getY(), 8);
    a.checkEqual("03. getGrowthBehaviour", li.getGrowthBehaviour(), Info::Fixed);
    a.check     ("04. isGrowHorizontal",  !li.isGrowHorizontal());
    a.check     ("05. isGrowVertical",    !li.isGrowVertical());
    a.check     ("06. isIgnored",         !li.isIgnored());
}

/** Test initialisation: ignored widget. */
AFL_TEST("ui.layout.Info:init:ignored", a)
{
    Info li;
    a.checkEqual("01",                     li.getPreferredSize().getX(), 0);
    a.checkEqual("02",                     li.getPreferredSize().getY(), 0);
    a.checkEqual("03. getGrowthBehaviour", li.getGrowthBehaviour(), Info::NoLayout);
    a.check     ("04. isGrowHorizontal",  !li.isGrowHorizontal());
    a.check     ("05. isGrowVertical",    !li.isGrowVertical());
    a.check     ("06. isIgnored",          li.isIgnored());
}

/** Test andGrowthBehaviour(). */
AFL_TEST("ui.layout.Info:andGrowthBehaviour", a)
{
    // NoLayout is neutral element
    a.checkEqual("01", Info::andGrowthBehaviour(Info::NoLayout,       Info::NoLayout),       Info::NoLayout);
    a.checkEqual("02", Info::andGrowthBehaviour(Info::Fixed,          Info::NoLayout),       Info::Fixed);
    a.checkEqual("03", Info::andGrowthBehaviour(Info::GrowHorizontal, Info::NoLayout),       Info::GrowHorizontal);
    a.checkEqual("04", Info::andGrowthBehaviour(Info::GrowVertical,   Info::NoLayout),       Info::GrowVertical);
    a.checkEqual("05", Info::andGrowthBehaviour(Info::GrowBoth,       Info::NoLayout),       Info::GrowBoth);

    // Fixed wins against everything
    a.checkEqual("11", Info::andGrowthBehaviour(Info::NoLayout,       Info::Fixed),          Info::Fixed);
    a.checkEqual("12", Info::andGrowthBehaviour(Info::Fixed,          Info::Fixed),          Info::Fixed);
    a.checkEqual("13", Info::andGrowthBehaviour(Info::GrowHorizontal, Info::Fixed),          Info::Fixed);
    a.checkEqual("14", Info::andGrowthBehaviour(Info::GrowVertical,   Info::Fixed),          Info::Fixed);
    a.checkEqual("15", Info::andGrowthBehaviour(Info::GrowBoth,       Info::Fixed),          Info::Fixed);

    // GrowHorizontal cancels GrowVertical/Both
    a.checkEqual("21", Info::andGrowthBehaviour(Info::NoLayout,       Info::GrowHorizontal), Info::GrowHorizontal);
    a.checkEqual("22", Info::andGrowthBehaviour(Info::Fixed,          Info::GrowHorizontal), Info::Fixed);
    a.checkEqual("23", Info::andGrowthBehaviour(Info::GrowHorizontal, Info::GrowHorizontal), Info::GrowHorizontal);
    a.checkEqual("24", Info::andGrowthBehaviour(Info::GrowVertical,   Info::GrowHorizontal), Info::Fixed);
    a.checkEqual("25", Info::andGrowthBehaviour(Info::GrowBoth,       Info::GrowHorizontal), Info::GrowHorizontal);

    // GrowVertical cancels GrowHorizontal/Both
    a.checkEqual("31", Info::andGrowthBehaviour(Info::NoLayout,       Info::GrowVertical),   Info::GrowVertical);
    a.checkEqual("32", Info::andGrowthBehaviour(Info::Fixed,          Info::GrowVertical),   Info::Fixed);
    a.checkEqual("33", Info::andGrowthBehaviour(Info::GrowHorizontal, Info::GrowVertical),   Info::Fixed);
    a.checkEqual("34", Info::andGrowthBehaviour(Info::GrowVertical,   Info::GrowVertical),   Info::GrowVertical);
    a.checkEqual("35", Info::andGrowthBehaviour(Info::GrowBoth,       Info::GrowVertical),   Info::GrowVertical);

    // GrowBoth
    a.checkEqual("41", Info::andGrowthBehaviour(Info::NoLayout,       Info::GrowBoth),       Info::GrowBoth);
    a.checkEqual("42", Info::andGrowthBehaviour(Info::Fixed,          Info::GrowBoth),       Info::Fixed);
    a.checkEqual("43", Info::andGrowthBehaviour(Info::GrowHorizontal, Info::GrowBoth),       Info::GrowHorizontal);
    a.checkEqual("44", Info::andGrowthBehaviour(Info::GrowVertical,   Info::GrowBoth),       Info::GrowVertical);
    a.checkEqual("45", Info::andGrowthBehaviour(Info::GrowBoth,       Info::GrowBoth),       Info::GrowBoth);
}

/** Test makeGrowthBehaviour(). */
AFL_TEST("ui.layout.Info:makeGrowthBehaviour", a)
{
    a.checkEqual("01", Info::makeGrowthBehaviour(false, false, false), Info::Fixed);
    a.checkEqual("02", Info::makeGrowthBehaviour(false, true,  false), Info::GrowVertical);
    a.checkEqual("03", Info::makeGrowthBehaviour(true,  false, false), Info::GrowHorizontal);
    a.checkEqual("04", Info::makeGrowthBehaviour(true,  true,  false), Info::GrowBoth);
    a.checkEqual("05", Info::makeGrowthBehaviour(false, false, true),  Info::NoLayout);
}
