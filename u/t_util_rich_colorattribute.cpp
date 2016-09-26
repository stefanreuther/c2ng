/**
  *  \file u/t_util_rich_colorattribute.cpp
  *  \brief Test for util::rich::ColorAttribute
  */

#include "util/rich/colorattribute.hpp"

#include "t_util_rich.hpp"

/** Simple/trivial test. */
void
TestUtilRichColorAttribute::testIt()
{
    util::rich::ColorAttribute testee(util::SkinColor::Red);
    TS_ASSERT_EQUALS(testee.getColor(), util::SkinColor::Red);
}

