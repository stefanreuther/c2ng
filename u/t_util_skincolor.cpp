/**
  *  \file u/t_util_skincolor.cpp
  *  \brief Test for util::SkinColor
  */

#include "util/skincolor.hpp"

#include "t_util.hpp"

/** Simple tests. */
void
TestUtilSkinColor::testIt()
{
    // Basic tests of the type
    util::SkinColor::Color color = util::SkinColor::Static;
    TS_ASSERT(util::SkinColor::Static != util::SkinColor::Input);
    TS_ASSERT(util::SkinColor::Static < util::SkinColor::NUM_COLORS);
    TS_ASSERT(color == util::SkinColor::Static);

    TS_ASSERT(util::SkinColor::NUM_COLORS > 0);
    TS_ASSERT(util::SkinColor::NUM_COLORS < 1000);
}

/** Test parse(). */
void
TestUtilSkinColor::testParse()
{
    util::SkinColor::Color c = util::SkinColor::Static;

    TS_ASSERT_EQUALS(util::SkinColor::parse("red", c), true);
    TS_ASSERT_EQUALS(c, util::SkinColor::Red);

    TS_ASSERT_EQUALS(util::SkinColor::parse("link-color", c), true);
    TS_ASSERT_EQUALS(c, util::SkinColor::Link);

    // Case-sensitive
    TS_ASSERT_EQUALS(util::SkinColor::parse("RED", c), false);
    TS_ASSERT_EQUALS(c, util::SkinColor::Link);  // unchanged

    // Invalid
    TS_ASSERT_EQUALS(util::SkinColor::parse("whatever", c), false);
    TS_ASSERT_EQUALS(c, util::SkinColor::Link);  // unchanged
}

