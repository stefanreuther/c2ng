/**
  *  \file u/t_gfx_basecontext.cpp
  *  \brief Test for gfx::BaseContext
  */

#include "gfx/basecontext.hpp"

#include "t_gfx.hpp"
#include "gfx/nullcanvas.hpp"
#include "gfx/defaultfont.hpp"
#include "gfx/font.hpp"

/** Test getter/setter. */
void
TestGfxBaseContext::testIt()
{
    // Environment
    gfx::NullCanvas a, b;
    afl::base::Ref<gfx::Font> f(gfx::createDefaultFont());

    // Testee
    gfx::BaseContext testee(a);

    // Initial state
    TS_ASSERT_EQUALS(testee.getRawColor(), 0U);
    TS_ASSERT_EQUALS(testee.isTransparentBackground(), true);
    TS_ASSERT_EQUALS(testee.getLineThickness(), 1);
    TS_ASSERT_EQUALS(testee.getLinePattern(), 0xFF);
    TS_ASSERT(testee.fillPattern().isBlack());
    TS_ASSERT_EQUALS(testee.getAlpha(), gfx::OPAQUE_ALPHA);
    TS_ASSERT_EQUALS(testee.getCursor(), gfx::Point(0, 0));
    TS_ASSERT_EQUALS(testee.getTextAlign(), gfx::Point(0, 0));
    TS_ASSERT(testee.getFont() == 0);
    TS_ASSERT_EQUALS(&testee.canvas(), &a);

    // Reconfigure
    testee.setRawColor(99);
    testee.setSolidBackground();
    testee.setLineThickness(3);
    testee.setLinePattern(0xCC);
    testee.setFillPattern(gfx::FillPattern::GRAY50);
    testee.setAlpha(120);
    testee.setCursor(gfx::Point(100, 200));
    testee.setTextAlign(2, 1);
    testee.useFont(*f);
    testee.useCanvas(b);

    TS_ASSERT_EQUALS(testee.getRawColor(), 99U);
    TS_ASSERT_EQUALS(testee.isTransparentBackground(), false);
    TS_ASSERT_EQUALS(testee.getLineThickness(), 3);
    TS_ASSERT_EQUALS(testee.getLinePattern(), 0xCC);
    TS_ASSERT_EQUALS(testee.fillPattern()[0], gfx::FillPattern::GRAY50[0]);
    TS_ASSERT_EQUALS(testee.fillPattern()[1], gfx::FillPattern::GRAY50[1]);
    TS_ASSERT_EQUALS(testee.fillPattern()[6], gfx::FillPattern::GRAY50[6]);
    TS_ASSERT_EQUALS(testee.fillPattern()[7], gfx::FillPattern::GRAY50[7]);
    TS_ASSERT_EQUALS(testee.getAlpha(), 120);
    TS_ASSERT_EQUALS(testee.getCursor(), gfx::Point(100, 200));
    TS_ASSERT_EQUALS(testee.getTextAlign(), gfx::Point(2, 1));
    TS_ASSERT_EQUALS(testee.getFont(), &*f);
    TS_ASSERT_EQUALS(&testee.canvas(), &b);

    // Background
    testee.setSolidBackground();
    TS_ASSERT(!testee.isTransparentBackground());
    testee.setTransparentBackground();
    TS_ASSERT(testee.isTransparentBackground());

    // Const
    const gfx::BaseContext& bc = testee;
    TS_ASSERT_EQUALS(bc.getRawColor(), testee.getRawColor());
    TS_ASSERT_EQUALS(bc.getLineThickness(), testee.getLineThickness());
    TS_ASSERT_EQUALS(&bc.fillPattern(), &testee.fillPattern());
    TS_ASSERT_EQUALS(&bc.canvas(), &testee.canvas());
    TS_ASSERT_EQUALS(bc.getFont(), testee.getFont());
}

