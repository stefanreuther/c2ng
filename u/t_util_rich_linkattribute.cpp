/**
  *  \file u/t_util_rich_linkattribute.cpp
  *  \brief Test for util::rich::LinkAttribute
  */

#include <memory>
#include "util/rich/linkattribute.hpp"

#include "t_util_rich.hpp"

/** Simple test. */
void
TestUtilRichLinkAttribute::testIt()
{
    util::rich::LinkAttribute testee("http://foo");
    TS_ASSERT_EQUALS(testee.getTarget(), "http://foo");

    std::auto_ptr<util::rich::LinkAttribute> copy(testee.clone());
    TS_ASSERT(copy.get() != 0);
    TS_ASSERT_EQUALS(copy->getTarget(), "http://foo");
}

