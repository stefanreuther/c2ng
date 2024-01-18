/**
  *  \file test/util/rich/colorattributetest.cpp
  *  \brief Test for util::rich::ColorAttribute
  */

#include "util/rich/colorattribute.hpp"

#include "afl/test/testrunner.hpp"
#include <memory>

/** Simple/trivial test. */
AFL_TEST("util.rich.ColorAttribute", a)
{
    util::rich::ColorAttribute testee(util::SkinColor::Red);
    a.checkEqual("01. getColor", testee.getColor(), util::SkinColor::Red);

    std::auto_ptr<util::rich::ColorAttribute> copy(testee.clone());
    a.checkNonNull("11. clone", copy.get());
    a.checkEqual("12. getColor", copy->getColor(), util::SkinColor::Red);
}
