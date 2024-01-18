/**
  *  \file test/util/rich/styleattributetest.cpp
  *  \brief Test for util::rich::StyleAttribute
  */

#include "util/rich/styleattribute.hpp"

#include "afl/test/testrunner.hpp"
#include <memory>

/** Simple test. */
AFL_TEST("util.rich.StyleAttribute", a)
{
    util::rich::StyleAttribute testee(util::rich::StyleAttribute::Bold);
    a.checkEqual("01. getStyle", testee.getStyle(), util::rich::StyleAttribute::Bold);

    std::auto_ptr<util::rich::StyleAttribute> copy(testee.clone());
    a.checkNonNull("11. clone", copy.get());
    a.checkEqual("12. getStyle", copy->getStyle(), util::rich::StyleAttribute::Bold);
}
