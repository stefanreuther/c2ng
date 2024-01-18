/**
  *  \file test/gfx/defaultfonttest.cpp
  *  \brief Test for gfx::DefaultFont
  */

#include "gfx/defaultfont.hpp"

#include "afl/test/testrunner.hpp"
#include "gfx/basecontext.hpp"
#include "gfx/canvas.hpp"
#include "gfx/font.hpp"
#include "gfx/palettizedpixmap.hpp"

/** Test metrics.
    A: create default font. Check metrics of text.
    E: metrics must not be zero (which would happen for an empty font). */
AFL_TEST("gfx.DefaultFont:metrics", a)
{
    afl::base::Ref<gfx::Font> font(gfx::createDefaultFont());

    a.checkGreaterThan("01. getTextWidth", font->getTextWidth("abc"), 0);
    a.checkGreaterThan("02. getTextHeight", font->getTextHeight("abc"), 0);

    a.checkGreaterThan("11. getTextWidth", font->getTextWidth("\xD0\x81"), 0);
}

/** Test rendering (regression test).
    A: create default font. Draw some text.
    E: expected pattern produced. */
AFL_TEST("gfx.DefaultFont:outText", a)
{
    afl::base::Ref<gfx::Font> font(gfx::createDefaultFont());

    // create canvas
    afl::base::Ref<gfx::PalettizedPixmap> pix(gfx::PalettizedPixmap::create(10, 10));
    for (int i = 0; i < 256; ++i) {
        pix->setPalette(uint8_t(i), COLORQUAD_FROM_RGBA(i, i, i, gfx::OPAQUE_ALPHA));
    }
    afl::base::Ref<gfx::Canvas> can(pix->makeCanvas());

    // draw
    gfx::BaseContext ctx(*can);
    ctx.setRawColor(7);
    font->outText(ctx, gfx::Point(0, 0), "a");

    // verify
    static const uint8_t EXPECTED[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 7, 7, 7, 7, 7, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 7, 7, 0, 0, 0,
        0, 7, 7, 7, 7, 7, 7, 0, 0, 0,
        7, 7, 0, 0, 0, 7, 7, 0, 0, 0,
        0, 7, 7, 7, 7, 7, 7, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
    a.checkEqualContent<uint8_t>("01. content", pix->pixels(), EXPECTED);
}
