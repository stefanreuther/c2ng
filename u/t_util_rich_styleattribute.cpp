/**
  *  \file u/t_util_rich_styleattribute.cpp
  *  \brief Test for util::rich::StyleAttribute
  */

#include <memory>
#include "util/rich/styleattribute.hpp"

#include "t_util_rich.hpp"

/** Simple test. */
void
TestUtilRichStyleAttribute::testIt()
{
    util::rich::StyleAttribute testee(util::rich::StyleAttribute::Bold);
    TS_ASSERT_EQUALS(testee.getStyle(), util::rich::StyleAttribute::Bold);

    std::auto_ptr<util::rich::StyleAttribute> copy(testee.clone());
    TS_ASSERT(copy.get() != 0);
    TS_ASSERT_EQUALS(copy->getStyle(), util::rich::StyleAttribute::Bold);
}

