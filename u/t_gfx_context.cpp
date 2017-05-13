/**
  *  \file u/t_gfx_context.cpp
  *  \brief Test for gfx::Context
  */

#include "gfx/context.hpp"

#include "t_gfx.hpp"
#include "gfx/nullcolorscheme.hpp"
#include "gfx/palettizedpixmap.hpp"

/** Simple test. */
void
TestGfxContext::testIt()
{
    // Environment
    afl::base::Ref<gfx::PalettizedPixmap> pix = gfx::PalettizedPixmap::create(3, 3);
    afl::base::Ref<gfx::Canvas> can(pix->makeCanvas());
    gfx::NullColorScheme<int> cs;

    // Testee
    gfx::Context<int> testee(*can, cs);

    // Base test
    testee.setLineThickness(3);
    testee.setTextAlign(2, 1);
    TS_ASSERT_EQUALS(testee.getLineThickness(), 3);
    TS_ASSERT_EQUALS(testee.getTextAlign(), gfx::Point(2, 1));
    TS_ASSERT_EQUALS(&testee.canvas(), &*can);

    // Context test
    testee.setColor(3);
    TS_ASSERT_EQUALS(testee.getRawColor(), 3U);
    TS_ASSERT_EQUALS(&testee.colorScheme(), &cs);

    gfx::NullColorScheme<int> other;
    testee.useColorScheme(other);
    TS_ASSERT_EQUALS(&testee.colorScheme(), &other);
}

