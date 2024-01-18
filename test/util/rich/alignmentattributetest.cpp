/**
  *  \file test/util/rich/alignmentattributetest.cpp
  *  \brief Test for util::rich::AlignmentAttribute
  */

#include "util/rich/alignmentattribute.hpp"

#include "afl/test/testrunner.hpp"
#include <memory>

/** Simple test. */
AFL_TEST("util.rich.AlignmentAttribute", a)
{
    util::rich::AlignmentAttribute testee(100, 2);
    a.checkEqual("01. getWidth", testee.getWidth(), 100);
    a.checkEqual("02. getAlignment", testee.getAlignment(), 2);

    std::auto_ptr<util::rich::AlignmentAttribute> copy(testee.clone());
    a.checkNonNull("11. clone", copy.get());
    a.checkEqual("12. getWidth", copy->getWidth(), 100);
    a.checkEqual("13. getAlignment", copy->getAlignment(), 2);
}
