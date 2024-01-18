/**
  *  \file test/gfx/colortransformtest.cpp
  *  \brief Test for gfx::ColorTransform
  */

#include "gfx/colortransform.hpp"

#include "afl/test/testrunner.hpp"
#include "gfx/palettizedpixmap.hpp"
#include "gfx/rgbapixmap.hpp"

using afl::base::Ref;
using gfx::RGBAPixmap;
using gfx::Canvas;
using gfx::PalettizedPixmap;
using gfx::ColorQuad_t;
using gfx::Color_t;

/** Test convertToMonochrome, palette-based. */
AFL_TEST("gfx.ColorTransform:convertToMonochrome:palette", a)
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
    a.checkEqual("01. getBitsPerPixel", can->getBitsPerPixel(), 8);

    // Read pixels
    Color_t newPixels[3];
    ColorQuad_t newQuads[3];

    // - first row
    can->getPixels(gfx::Point(0, 0), newPixels);
    can->decodeColors(newPixels, newQuads);
    a.checkEqual("11. pixel", newQuads[0], COLORQUAD_FROM_RGB(0, 0, 0));
    a.checkEqual("12. pixel", newQuads[1], COLORQUAD_FROM_RGB(0, 16, 0));
    a.checkEqual("13. pixel", newQuads[2], COLORQUAD_FROM_RGB(0, 16, 0));

    // - second row
    can->getPixels(gfx::Point(0, 1), newPixels);
    can->decodeColors(newPixels, newQuads);
    a.checkEqual("21. pixel", newQuads[0], COLORQUAD_FROM_RGB(0, 50, 0));
    a.checkEqual("22. pixel", newQuads[1], COLORQUAD_FROM_RGB(0, 50, 0));
    a.checkEqual("23. pixel", newQuads[2], COLORQUAD_FROM_RGB(0, 128, 0));
}

/** Test convertToMonochrome, RGBA-based. */
AFL_TEST("gfx.ColorTransform:convertToMonochrome:rgba", a)
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
    a.checkEqual("01. getBitsPerPixel", can->getBitsPerPixel(), 32);

    // Read pixels
    Color_t newPixels[3];
    ColorQuad_t newQuads[3];

    // - first row
    can->getPixels(gfx::Point(0, 0), newPixels);
    can->decodeColors(newPixels, newQuads);
    a.checkEqual("11. pixel", newQuads[0], COLORQUAD_FROM_RGB(0, 0, 0));
    a.checkEqual("12. pixel", newQuads[1], COLORQUAD_FROM_RGB(0, 16, 0));
    a.checkEqual("13. pixel", newQuads[2], COLORQUAD_FROM_RGB(0, 16, 0));

    // - second row
    can->getPixels(gfx::Point(0, 1), newPixels);
    can->decodeColors(newPixels, newQuads);
    a.checkEqual("21. pixel", newQuads[0], COLORQUAD_FROM_RGB(0, 50, 0));
    a.checkEqual("22. pixel", newQuads[1], COLORQUAD_FROM_RGB(0, 50, 0));
    a.checkEqual("23. pixel", newQuads[2], COLORQUAD_FROM_RGB(0, 128, 0));
}
