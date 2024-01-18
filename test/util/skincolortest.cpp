/**
  *  \file test/util/skincolortest.cpp
  *  \brief Test for util::SkinColor
  */

#include "util/skincolor.hpp"
#include "afl/test/testrunner.hpp"

/** Simple tests. */
AFL_TEST("util.SkinColor:basics", a)
{
    // Basic tests of the type
    util::SkinColor::Color color = util::SkinColor::Static;
    a.check("01", util::SkinColor::Static != util::SkinColor::Input);
    a.check("02", util::SkinColor::Static < util::SkinColor::NUM_COLORS);
    a.check("03", color == util::SkinColor::Static);

    a.check("11", util::SkinColor::NUM_COLORS > 0);
    a.check("12", util::SkinColor::NUM_COLORS < 1000);
}

/** Test parse(). */
AFL_TEST("util.SkinColor:parse", a)
{
    util::SkinColor::Color c = util::SkinColor::Static;

    a.checkEqual("01", util::SkinColor::parse("red", c), true);
    a.checkEqual("02", c, util::SkinColor::Red);

    a.checkEqual("11", util::SkinColor::parse("link-color", c), true);
    a.checkEqual("12", c, util::SkinColor::Link);

    // Case-sensitive
    a.checkEqual("21", util::SkinColor::parse("RED", c), false);
    a.checkEqual("22", c, util::SkinColor::Link);  // unchanged

    // Invalid
    a.checkEqual("31", util::SkinColor::parse("whatever", c), false);
    a.checkEqual("32", c, util::SkinColor::Link);  // unchanged
}
