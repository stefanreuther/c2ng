/**
  *  \file u/t_util_rich_colorattribute.cpp
  *  \brief Test for util::rich::ColorAttribute
  */

#include <memory>
#include "util/rich/colorattribute.hpp"

#include "t_util_rich.hpp"

/** Simple/trivial test. */
void
TestUtilRichColorAttribute::testIt()
{
    util::rich::ColorAttribute testee(util::SkinColor::Red);
    TS_ASSERT_EQUALS(testee.getColor(), util::SkinColor::Red);

    std::auto_ptr<util::rich::ColorAttribute> copy(testee.clone());
    TS_ASSERT(copy.get() != 0);
    TS_ASSERT_EQUALS(copy->getColor(), util::SkinColor::Red);
}

