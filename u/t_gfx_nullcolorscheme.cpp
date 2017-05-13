/**
  *  \file u/t_gfx_nullcolorscheme.cpp
  *  \brief Test for gfx::NullColorScheme
  */

#include "gfx/nullcolorscheme.hpp"

#include "t_gfx.hpp"
#include "gfx/pixmapcanvasimpl.hpp"
#include "gfx/palettizedpixmap.hpp"

/** Simple test. */
void
TestGfxNullColorScheme::testIt()
{
    gfx::NullColorScheme<int> testee;

    // Color inquiry
    TS_ASSERT_EQUALS(testee.getColor(99), 99U);
    TS_ASSERT_EQUALS(gfx::NullColorScheme<int>::instance.getColor(77), 77U);

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
    TS_ASSERT_EQUALS(out[0], 1U);
    TS_ASSERT_EQUALS(out[1], 0U);
    TS_ASSERT_EQUALS(out[2], 0U);
    TS_ASSERT_EQUALS(out[3], 0U);

    // - verify using pixels interface
    static const uint8_t EXPECTED_CONTENT[] = {
        1,1,1,
        1,1,1,
        1,0,0,
        1,0,0,
    };
    TS_ASSERT_EQUALS(sizeof(EXPECTED_CONTENT), pix->pixels().size());
    TS_ASSERT_SAME_DATA(EXPECTED_CONTENT, pix->pixels().unsafeData(), sizeof(EXPECTED_CONTENT));
}

