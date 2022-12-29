/**
  *  \file u/t_gfx_colortransform.cpp
  *  \brief Test for gfx::ColorTransform
  */

#include "gfx/colortransform.hpp"

#include "t_gfx.hpp"
#include "gfx/palettizedpixmap.hpp"
#include "gfx/rgbapixmap.hpp"

using afl::base::Ref;
using gfx::RGBAPixmap;
using gfx::Canvas;
using gfx::PalettizedPixmap;
using gfx::ColorQuad_t;
using gfx::Color_t;

/** Test convertToMonochrome, palette-based. */
void
TestGfxColorTransform::testPalette()
{
    // Set up a small pixmap
    Ref<PalettizedPixmap> pix = PalettizedPixmap::create(3, 2);
    ColorQuad_t origPalette[] = { COLORQUAD_FROM_RGB(0, 0, 0),
                                  COLORQUAD_FROM_RGB(100, 0, 0),
                                  COLORQUAD_FROM_RGB(100, 100, 100),
                                  COLORQUAD_FROM_RGB(255, 255, 255) };
    uint8_t origPixels[] = { 0, 1, 1, 2, 2, 3 };
    pix->setPalette(0, origPalette);
    pix->pixels().copyFrom(origPixels);

    // Transform
    Ref<Canvas> can = convertToMonochrome(*pix->makeCanvas(), COLORQUAD_FROM_RGB(0, 128, 0));

    // Must still be palettized
    TS_ASSERT_EQUALS(can->getBitsPerPixel(), 8);

    // Read pixels
    Color_t newPixels[3];
    ColorQuad_t newQuads[3];

    // - first row
    can->getPixels(gfx::Point(0, 0), newPixels);
    can->decodeColors(newPixels, newQuads);
    TS_ASSERT_EQUALS(newQuads[0], COLORQUAD_FROM_RGB(0, 0, 0));
    TS_ASSERT_EQUALS(newQuads[1], COLORQUAD_FROM_RGB(0, 16, 0));
    TS_ASSERT_EQUALS(newQuads[2], COLORQUAD_FROM_RGB(0, 16, 0));

    // - second row
    can->getPixels(gfx::Point(0, 1), newPixels);
    can->decodeColors(newPixels, newQuads);
    TS_ASSERT_EQUALS(newQuads[0], COLORQUAD_FROM_RGB(0, 50, 0));
    TS_ASSERT_EQUALS(newQuads[1], COLORQUAD_FROM_RGB(0, 50, 0));
    TS_ASSERT_EQUALS(newQuads[2], COLORQUAD_FROM_RGB(0, 128, 0));
}

/** Test convertToMonochrome, RGBA-based. */
void
TestGfxColorTransform::testRGBA()
{
    // Set up a small pixmap
    Ref<RGBAPixmap> pix = RGBAPixmap::create(3, 2);
    ColorQuad_t origPixels[] = {
        COLORQUAD_FROM_RGB(0, 0, 0),
        COLORQUAD_FROM_RGB(100, 0, 0),
        COLORQUAD_FROM_RGB(100, 0, 0),
        COLORQUAD_FROM_RGB(100, 100, 100),
        COLORQUAD_FROM_RGB(100, 100, 100),
        COLORQUAD_FROM_RGB(255, 255, 255),
    };
    pix->pixels().copyFrom(origPixels);

    // Transform
    Ref<Canvas> can = convertToMonochrome(*pix->makeCanvas(), COLORQUAD_FROM_RGB(0, 128, 0));

    // Must still be truecolor
    TS_ASSERT_EQUALS(can->getBitsPerPixel(), 32);

    // Read pixels
    Color_t newPixels[3];
    ColorQuad_t newQuads[3];

    // - first row
    can->getPixels(gfx::Point(0, 0), newPixels);
    can->decodeColors(newPixels, newQuads);
    TS_ASSERT_EQUALS(newQuads[0], COLORQUAD_FROM_RGB(0, 0, 0));
    TS_ASSERT_EQUALS(newQuads[1], COLORQUAD_FROM_RGB(0, 16, 0));
    TS_ASSERT_EQUALS(newQuads[2], COLORQUAD_FROM_RGB(0, 16, 0));

    // - second row
    can->getPixels(gfx::Point(0, 1), newPixels);
    can->decodeColors(newPixels, newQuads);
    TS_ASSERT_EQUALS(newQuads[0], COLORQUAD_FROM_RGB(0, 50, 0));
    TS_ASSERT_EQUALS(newQuads[1], COLORQUAD_FROM_RGB(0, 50, 0));
    TS_ASSERT_EQUALS(newQuads[2], COLORQUAD_FROM_RGB(0, 128, 0));
}

