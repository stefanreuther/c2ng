/**
  *  \file test/gfx/nullresourceprovidertest.cpp
  *  \brief Test for gfx::NullResourceProvider
  */

#include "gfx/nullresourceprovider.hpp"
#include "afl/test/testrunner.hpp"

/** Simple sanity test. */
AFL_TEST("gfx.NullResourceProvider", a)
{
    gfx::NullResourceProvider testee;

    // Image request
    bool flag = false;
    a.checkNull("01. getImage", testee.getImage("x", &flag).get());
    a.check("02. getImage", flag);

    // Font request
    afl::base::Ref<gfx::Font> f = testee.getFont(gfx::FontRequest());
    a.checkNonNull("11. getFont", &f.get());
    a.checkGreaterThan("12. getFont", f->getTextWidth("abc"), 0);
}
