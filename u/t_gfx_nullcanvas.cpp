/**
  *  \file u/t_gfx_nullcanvas.cpp
  *  \brief Test for gfx::NullCanvas
  */

#include "gfx/nullcanvas.hpp"

#include "t_gfx.hpp"

/** Simple test. */
void
TestGfxNullCanvas::testIt()
{
    // NullCanvas does not do anything.
    // But the object must be creatible.
    gfx::NullCanvas testee;

    // Methods must operate without crashing
    static const gfx::Color_t colors[] = {1,5,9};
    TS_ASSERT_THROWS_NOTHING(testee.drawHLine(gfx::Point(1, 1), 5, 0, 0xFF, gfx::OPAQUE_ALPHA));
    TS_ASSERT_THROWS_NOTHING(testee.drawVLine(gfx::Point(1, 1), 5, 0, 0xFF, gfx::OPAQUE_ALPHA));
    TS_ASSERT_THROWS_NOTHING(testee.drawPixel(gfx::Point(9, 2), 0x123, gfx::OPAQUE_ALPHA));
    TS_ASSERT_THROWS_NOTHING(testee.drawPixels(gfx::Point(9, 2), colors, gfx::OPAQUE_ALPHA));
    TS_ASSERT_THROWS_NOTHING(testee.drawBar(gfx::Rectangle(1,2,3,4), 0x99, 0x77, gfx::FillPattern::SOLID, gfx::OPAQUE_ALPHA));
    TS_ASSERT_THROWS_NOTHING(testee.blit(gfx::Point(9, 9), testee, gfx::Rectangle(0, 0, 77, 77)));

    // Clipping
    TS_ASSERT(!testee.computeClipRect(gfx::Rectangle(3,4,5,6)).exists());

    // Color reading
    gfx::Color_t readColors[3] = {1,2,3};
    TS_ASSERT_THROWS_NOTHING(testee.getPixels(gfx::Point(8,9), readColors));
    TS_ASSERT_EQUALS(readColors[0], 0U);
    TS_ASSERT_EQUALS(readColors[1], 0U);
    TS_ASSERT_EQUALS(readColors[2], 0U);

    // Inquiry
    TS_ASSERT_EQUALS(testee.getSize(), gfx::Point(1, 1));
    TS_ASSERT_EQUALS(testee.getBitsPerPixel(), 1);
    TS_ASSERT(!testee.isVisible(gfx::Rectangle(0, 0, 1, 1)));
    TS_ASSERT(testee.isClipped(gfx::Rectangle(0, 0, 1, 1)));

    // Palette
    static const gfx::ColorQuad_t quadsIn[2] = { COLORQUAD_FROM_RGBA(1,2,3,4), COLORQUAD_FROM_RGBA(9,8,7,6) };
    {
        gfx::Color_t colorsOut[2] = {1,1};
        TS_ASSERT_THROWS_NOTHING(testee.setPalette(33, quadsIn, colorsOut));
        TS_ASSERT_EQUALS(colorsOut[0], 33U);
        TS_ASSERT_EQUALS(colorsOut[1], 34U);
    }
    {
        gfx::Color_t colorsOut[2] = {1,1};
        TS_ASSERT_THROWS_NOTHING(testee.encodeColors(quadsIn, colorsOut));
        TS_ASSERT_EQUALS(colorsOut[0], 0U);
        TS_ASSERT_EQUALS(colorsOut[1], 0U);
    }
    {
        gfx::ColorQuad_t quadsOut[3];
        TS_ASSERT_THROWS_NOTHING(testee.decodeColors(readColors, quadsOut));
        TS_ASSERT_EQUALS(quadsOut[0], 0U);
        TS_ASSERT_EQUALS(quadsOut[1], 0U);
    }

    // Conversion
    afl::base::Ref<gfx::Canvas> can = *new gfx::NullCanvas();
    afl::base::Ref<gfx::Canvas> can2 = testee.convertCanvas(can);
    TS_ASSERT_EQUALS(&*can, &*can2);
}

