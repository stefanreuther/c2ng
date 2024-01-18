/**
  *  \file test/gfx/nullcolorschemetest.cpp
  *  \brief Test for gfx::NullColorScheme
  */

#include "gfx/nullcolorscheme.hpp"

#include "afl/test/testrunner.hpp"
#include "gfx/palettizedpixmap.hpp"
#include "gfx/pixmapcanvasimpl.hpp"

/** Simple test. */
AFL_TEST("gfx.NullColorScheme", a)
{
    gfx::NullColorScheme<int> testee;

    // Color inquiry
    a.checkEqual("01. getColor", testee.getColor(99), 99U);
    a.checkEqual("02. getColor", gfx::NullColorScheme<int>::instance.getColor(77), 77U);

    // Drawing
    // - set up a canvas
    afl::base::Ref<gfx::PalettizedPixmap> pix = gfx::PalettizedPixmap::create(3, 4);
    pix->setPalette(1, COLORQUAD_FROM_RGBA(99,99,88,77));
    pix->pixels().fill(1);
    afl::base::Ref<gfx::Canvas> can = pix->makeCanvas();

    // - draw on it
    testee.drawBackground(*can, gfx::Rectangle(1, 2, 7, 7));

    // - read using canvas interface
    gfx::Color_t out[4];
    can->getPixels(gfx::Point(0, 3), out);
    a.checkEqual("11. getPixels", out[0], 1U);
    a.checkEqual("12. getPixels", out[1], 0U);
    a.checkEqual("13. getPixels", out[2], 0U);
    a.checkEqual("14. getPixels", out[3], 0U);

    // - verify using pixels interface
    static const uint8_t EXPECTED_CONTENT[] = {
        1,1,1,
        1,1,1,
        1,0,0,
        1,0,0,
    };
    a.checkEqualContent<uint8_t>("21. pixels", pix->pixels(), EXPECTED_CONTENT);
}
