/**
  *  \file test/gfx/contexttest.cpp
  *  \brief Test for gfx::Context
  */

#include "gfx/context.hpp"

#include "afl/test/testrunner.hpp"
#include "gfx/nullcolorscheme.hpp"
#include "gfx/palettizedpixmap.hpp"

/** Simple test. */
AFL_TEST("gfx.Context", a)
{
    // Environment
    afl::base::Ref<gfx::PalettizedPixmap> pix = gfx::PalettizedPixmap::create(3, 3);
    afl::base::Ref<gfx::Canvas> can(pix->makeCanvas());
    gfx::NullColorScheme<int> cs;

    // Testee
    gfx::Context<int> testee(*can, cs);

    // Base test
    testee.setLineThickness(3);
    testee.setTextAlign(gfx::RightAlign, gfx::MiddleAlign);
    a.checkEqual("01. getLineThickness", testee.getLineThickness(), 3);
    a.checkEqual("02. getTextAlign", testee.getTextAlign(), gfx::Point(2, 1));
    a.checkEqual("03. canvas", &testee.canvas(), &*can);

    // Context test
    testee.setColor(3);
    a.checkEqual("11. getRawColor", testee.getRawColor(), 3U);
    a.checkEqual("12. colorScheme", &testee.colorScheme(), &cs);

    gfx::NullColorScheme<int> other;
    testee.useColorScheme(other);
    a.checkEqual("21. colorScheme", &testee.colorScheme(), &other);
}
