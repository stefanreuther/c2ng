/**
  *  \file u/t_util_skincolor.cpp
  *  \brief Test for util::SkinColor
  */

#include "util/skincolor.hpp"

#include "t_util.hpp"

/** Simple tests.
    This is just a type definition, so we mainly test wellformedness of the header file. */
void
TestUtilSkinColor::testIt()
{
    util::SkinColor::Color color = util::SkinColor::Static;
    TS_ASSERT(util::SkinColor::Static != util::SkinColor::Input);
    TS_ASSERT(util::SkinColor::Static < util::SkinColor::NUM_COLORS);
    TS_ASSERT(color == util::SkinColor::Static);

    TS_ASSERT(util::SkinColor::NUM_COLORS > 0);
    TS_ASSERT(util::SkinColor::NUM_COLORS < 1000);
}

