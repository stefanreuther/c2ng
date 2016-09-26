/**
  *  \file u/t_util_rich_linkattribute.cpp
  *  \brief Test for util::rich::LinkAttribute
  */

#include "util/rich/linkattribute.hpp"

#include "t_util_rich.hpp"

/** Simple test. */
void
TestUtilRichLinkAttribute::testIt()
{
    util::rich::LinkAttribute testee("http://foo");
    TS_ASSERT_EQUALS(testee.getTarget(), "http://foo");
}

