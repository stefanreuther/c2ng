/**
  *  \file u/t_gfx_nullresourceprovider.cpp
  *  \brief Test for gfx::NullResourceProvider
  */

#include "gfx/nullresourceprovider.hpp"

#include "t_gfx.hpp"

/** Simple sanity test. */
void
TestGfxNullResourceProvider::testIt()
{
    gfx::NullResourceProvider testee;

    // Image request
    bool flag = false;
    TS_ASSERT(testee.getImage("x", &flag).get() == 0);
    TS_ASSERT(flag);

    // Font request
    afl::base::Ref<gfx::Font> f = testee.getFont(gfx::FontRequest());
    TS_ASSERT(&f.get() != 0);
    TS_ASSERT(f->getTextWidth("abc") > 0);
}

