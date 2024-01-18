/**
  *  \file test/util/rich/linkattributetest.cpp
  *  \brief Test for util::rich::LinkAttribute
  */

#include "util/rich/linkattribute.hpp"

#include "afl/test/testrunner.hpp"
#include <memory>

/** Simple test. */
AFL_TEST("util.rich.LinkAttribute", a)
{
    util::rich::LinkAttribute testee("http://foo");
    a.checkEqual("01. getTarget", testee.getTarget(), "http://foo");

    std::auto_ptr<util::rich::LinkAttribute> copy(testee.clone());
    a.checkNonNull("11. clone", copy.get());
    a.checkEqual("12. getTarget", copy->getTarget(), "http://foo");
}
