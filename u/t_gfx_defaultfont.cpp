/**
  *  \file u/t_gfx_defaultfont.cpp
  *  \brief Test for gfx::DefaultFont
  */

#include "gfx/defaultfont.hpp"

#include "t_gfx.hpp"
#include "gfx/font.hpp"

/** Simple test.
    Acceptance criterium is that rendering text produces nonzero metric (like an empty font would do). */
void
TestGfxDefaultFont::testIt()
{
    afl::base::Ref<gfx::Font> font(gfx::createDefaultFont());
    TS_ASSERT(font->getTextWidth("abc") > 0);
    TS_ASSERT(font->getTextHeight("abc") > 0);

    TS_ASSERT(font->getTextWidth("\xD0\x81") > 0);
}

