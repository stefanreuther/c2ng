/**
  *  \file u/t_util_rich_alignmentattribute.cpp
  *  \brief Test for util::rich::AlignmentAttribute
  */

#include <memory>
#include "util/rich/alignmentattribute.hpp"

#include "t_util_rich.hpp"

/** Simple test. */
void
TestUtilRichAlignmentAttribute::testIt()
{
    util::rich::AlignmentAttribute testee(100, 2);
    TS_ASSERT_EQUALS(testee.getWidth(), 100);
    TS_ASSERT_EQUALS(testee.getAlignment(), 2);

    std::auto_ptr<util::rich::AlignmentAttribute> copy(testee.clone());
    TS_ASSERT(copy.get() != 0);
    TS_ASSERT_EQUALS(copy->getWidth(), 100);
    TS_ASSERT_EQUALS(copy->getAlignment(), 2);
}

