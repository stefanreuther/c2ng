/**
  *  \file u/t_gfx_colorquantizer.cpp
  *  \brief Test for gfx::ColorQuantizer
  */

#include "gfx/colorquantizer.hpp"

#include "t_gfx.hpp"
#include "gfx/rgbapixmap.hpp"

using afl::base::Memory;
using afl::base::Ref;
using gfx::RGBAPixmap;
using gfx::ColorQuad_t;
using gfx::PalettizedPixmap;

/** Test fixed palette.
    A: Configure a fixed palette.
    E: Palette is used correctly, and passed through unchanged. */
void
TestGfxColorQuantizer::testFixed()
{
    static const ColorQuad_t COLORS[] = {
        COLORQUAD_FROM_RGB(100, 0, 0),
        COLORQUAD_FROM_RGB(0, 100, 0),
        COLORQUAD_FROM_RGB(0, 0, 100),
    };

    Ref<RGBAPixmap> in = RGBAPixmap::create(3, 1);
    in->pixels().copyFrom(COLORS);

    Ref<PalettizedPixmap> out = gfx::ColorQuantizer()
        .setPalette(0, COLORS)
        .setUsablePaletteRange(0, 3)
        .setDynamicPaletteRange(0, 0)
        .quantize(*in->makeCanvas());

    TS_ASSERT_EQUALS(out->getWidth(), 3);
    TS_ASSERT_EQUALS(out->getHeight(), 1);
    TS_ASSERT_EQUALS(*out->pixels().at(0), 0);
    TS_ASSERT_EQUALS(*out->pixels().at(1), 1);
    TS_ASSERT_EQUALS(*out->pixels().at(2), 2);

    ColorQuad_t palette[3];
    out->getPalette(0, palette);
    TS_ASSERT_EQUALS(palette[0], COLORS[0]);
    TS_ASSERT_EQUALS(palette[1], COLORS[1]);
    TS_ASSERT_EQUALS(palette[2], COLORS[2]);
}

/** Test fixed palette.
    A: Configure a fully dynamic palette.
    E: Palette is assigned and used correctly. */
void
TestGfxColorQuantizer::testDynamic()
{
    // Three colors, each with a different count to ensure determinism.
    static const ColorQuad_t COLORS[] = {
        COLORQUAD_FROM_RGB(128, 0, 0),
        COLORQUAD_FROM_RGB(0, 128, 0),
        COLORQUAD_FROM_RGB(0, 0, 128),
        COLORQUAD_FROM_RGB(0, 128, 0),
        COLORQUAD_FROM_RGB(0, 128, 0),
        COLORQUAD_FROM_RGB(0, 0, 128),
    };

    Ref<RGBAPixmap> in = RGBAPixmap::create(3, 2);
    in->pixels().copyFrom(COLORS);

    Ref<PalettizedPixmap> out = gfx::ColorQuantizer()
        .setPalette(0, COLORS)
        .setUsablePaletteRange(0, 255)
        .setDynamicPaletteRange(0, 255)
        .quantize(*in->makeCanvas());

    TS_ASSERT_EQUALS(out->getWidth(), 3);
    TS_ASSERT_EQUALS(out->getHeight(), 2);
    TS_ASSERT_EQUALS(*out->pixels().at(0), 2);
    TS_ASSERT_EQUALS(*out->pixels().at(1), 0);
    TS_ASSERT_EQUALS(*out->pixels().at(2), 1);
    TS_ASSERT_EQUALS(*out->pixels().at(3), 0);
    TS_ASSERT_EQUALS(*out->pixels().at(4), 0);
    TS_ASSERT_EQUALS(*out->pixels().at(5), 1);

    ColorQuad_t palette[3];
    out->getPalette(0, palette);
    TS_ASSERT_EQUALS(palette[0], COLORS[1]);
    TS_ASSERT_EQUALS(palette[1], COLORS[2]);
    TS_ASSERT_EQUALS(palette[2], COLORS[0]);
}

/** Test fixed palette.
    A: Configure a fixed palette with a dynamic range in the middle.
    E: Dynamic part is assigned and used correctly. */
void
TestGfxColorQuantizer::testMixed()
{
    static const ColorQuad_t COLORS[] = {
        COLORQUAD_FROM_RGB(128, 0, 0),
        COLORQUAD_FROM_RGB(0, 128, 0),
        COLORQUAD_FROM_RGB(0, 0, 128),
    };

    Ref<RGBAPixmap> in = RGBAPixmap::create(3, 1);
    in->pixels().copyFrom(COLORS);

    Ref<PalettizedPixmap> out = gfx::ColorQuantizer()
        .setPalette(0, COLORS[1])
        .setPalette(2, COLORS[2])
        .setUsablePaletteRange(0, 3)
        .setDynamicPaletteRange(1, 1)
        .quantize(*in->makeCanvas());

    TS_ASSERT_EQUALS(out->getWidth(), 3);
    TS_ASSERT_EQUALS(out->getHeight(), 1);
    TS_ASSERT_EQUALS(*out->pixels().at(0), 1);
    TS_ASSERT_EQUALS(*out->pixels().at(1), 0);
    TS_ASSERT_EQUALS(*out->pixels().at(2), 2);

    ColorQuad_t palette[3];
    out->getPalette(0, palette);
    TS_ASSERT_EQUALS(palette[0], COLORS[1]);
    TS_ASSERT_EQUALS(palette[1], COLORS[0]);
    TS_ASSERT_EQUALS(palette[2], COLORS[2]);
}

/** Test large image.
    This exercises the re-blocking in countColors(). */
void
TestGfxColorQuantizer::testLarge()
{
    Ref<RGBAPixmap> in = RGBAPixmap::create(10000, 1);
    in->pixels().subrange(0, 1000).fill(COLORQUAD_FROM_RGB(128, 0, 0));
    in->pixels().subrange(1000, 4000).fill(COLORQUAD_FROM_RGB(0, 128, 0));
    in->pixels().subrange(5000, 5000).fill(COLORQUAD_FROM_RGB(0, 0, 128));

    Ref<PalettizedPixmap> out = gfx::ColorQuantizer()
        .setUsablePaletteRange(0, 3)
        .setDynamicPaletteRange(0, 255)
        .quantize(*in->makeCanvas());

    TS_ASSERT_EQUALS(out->getWidth(), 10000);
    TS_ASSERT_EQUALS(out->getHeight(), 1);
    TS_ASSERT_EQUALS(*out->pixels().at(0), 2);
    TS_ASSERT_EQUALS(*out->pixels().at(1000), 1);
    TS_ASSERT_EQUALS(*out->pixels().at(5000), 0);

    ColorQuad_t palette[3];
    out->getPalette(0, palette);
    TS_ASSERT_EQUALS(palette[0], COLORQUAD_FROM_RGB(0, 0, 128));
    TS_ASSERT_EQUALS(palette[1], COLORQUAD_FROM_RGB(0, 128, 0));
    TS_ASSERT_EQUALS(palette[2], COLORQUAD_FROM_RGB(128, 0, 0));

}

/** Test dithering.
    A: Configure a fixed palette with two extreme. Provide a picture with an intermediate color.
    E: All palette colors used. */
void
TestGfxColorQuantizer::testDither()
{
    Ref<RGBAPixmap> in = RGBAPixmap::create(100, 100);
    in->pixels().fill(COLORQUAD_FROM_RGB(128, 0, 0));

    Ref<PalettizedPixmap> out = gfx::ColorQuantizer()
        .setUsablePaletteRange(0, 2)
        .setDynamicPaletteRange(0, 0)
        .setPalette(0, COLORQUAD_FROM_RGB(0, 0, 0))
        .setPalette(1, COLORQUAD_FROM_RGB(255, 0, 0))
        .quantize(*in->makeCanvas());

    int black = 0, white = 0;
    Memory<uint8_t> pixels = out->pixels();
    while (const uint8_t* p = pixels.eat()) {
        TS_ASSERT_LESS_THAN(*p, 2);
        if (*p == 0) {
            ++black;
        } else {
            ++white;
        }
    }

    // We actually achieve 5000:5000, but give some slack for imprecision.
    TS_ASSERT_LESS_THAN(4500, black);
    TS_ASSERT_LESS_THAN(4500, white);
}

