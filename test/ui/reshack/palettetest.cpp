/**
  *  \file test/ui/reshack/palettetest.cpp
  *  \brief Test for ui::reshack::Palette
  */

#include "ui/reshack/palette.hpp"

#include "afl/test/testrunner.hpp"
#include "gfx/codec/custom.hpp"
#include "gfx/palettizedpixmap.hpp"
#include "gfx/rgbapixmap.hpp"

using ui::reshack::Palette;

/* Test makeEditable, with compatible palette */
AFL_TEST("ui.reshack.Palette:makeEditable:palettized", a)
{
    // Test pixels
    afl::base::ConstBytes_t bytes = afl::string::toBytes("<hello, world!>");
    a.checkEqual("input size", bytes.size(), 15U);

    // Create pixmap
    afl::base::Ref<gfx::PalettizedPixmap> orig = gfx::PalettizedPixmap::create(3, 5);
    orig->setPalette(0, gfx::codec::Custom::getPalette());
    orig->pixels().copyFrom(bytes);

    // Make editable
    afl::base::Ref<gfx::PalettizedPixmap> result = Palette::makeEditable(orig->makeCanvas());

    // Verify content
    a.checkEqual("result width",  result->getWidth(), 3);
    a.checkEqual("result height", result->getHeight(), 5);
    a.checkEqualContent("result data", afl::base::ConstBytes_t(result->pixels()), bytes);
}

/* Test makeEditable, with RGB source */
AFL_TEST("ui.reshack.Palette:makeEditable:rgba", a)
{
    // Test pixels
    static const gfx::ColorQuad_t PIXELS[] = {
        COLORQUAD_FROM_RGB(255,255,255),
        COLORQUAD_FROM_RGB(0,0,0),
        COLORQUAD_FROM_RGB(255,255,255),
        COLORQUAD_FROM_RGB(0,0,0),
        COLORQUAD_FROM_RGB(255,255,255),
        COLORQUAD_FROM_RGB(0,0,0),
    };
    static const uint8_t BYTES[] = {
        15,0,15,0,15,0
    };

    // Create pixmap
    afl::base::Ref<gfx::RGBAPixmap> orig = gfx::RGBAPixmap::create(3, 2);
    orig->pixels().copyFrom(PIXELS);

    // Make editable
    afl::base::Ref<gfx::PalettizedPixmap> result = Palette::makeEditable(orig->makeCanvas());

    // Verify content
    a.checkEqual("result width",  result->getWidth(), 3);
    a.checkEqual("result height", result->getHeight(), 2);
    a.checkEqualContent("result data", afl::base::ConstBytes_t(result->pixels()), afl::base::ConstBytes_t(BYTES));
}

/* Test copyPalette */
AFL_TEST("ui.reshack.Palette:copyPalette", a)
{
    // Test palette - just some arbitrary numbers
    static const gfx::ColorQuad_t PAL[] = {
        COLORQUAD_FROM_RGB(50,60,70),
        COLORQUAD_FROM_RGB(1,2,3),
        COLORQUAD_FROM_RGB(99,88,77),
    };

    // Create pixmap
    gfx::Color_t handles[10];
    afl::base::Ref<gfx::PalettizedPixmap> orig = gfx::PalettizedPixmap::create(3, 5);
    afl::base::Ref<gfx::Canvas> can = orig->makeCanvas();
    can->setPalette(0, PAL, handles);

    // Copy palette elsewhere
    afl::base::Ref<gfx::PalettizedPixmap> result = gfx::PalettizedPixmap::create(10, 20);
    Palette::copyPalette(*result, *can);

    // Verify palette
    gfx::ColorQuad_t out[3];
    result->getPalette(0, out);

    a.checkEqual("palette entry 0", PAL[0], out[0]);
    a.checkEqual("palette entry 1", PAL[1], out[1]);
    a.checkEqual("palette entry 2", PAL[2], out[2]);
}

/* Test isEditableColor */
AFL_TEST("ui.reshack.Palette:copyPalette", a)
{
    a.check("standard 0",   !Palette::isEditableColor(Palette::StandardPaletteColor, 0));
    a.check("standard 150", !Palette::isEditableColor(Palette::StandardPaletteColor, 150));
    a.check("standard 160",  Palette::isEditableColor(Palette::StandardPaletteColor, 160));
    a.check("standard 200",  Palette::isEditableColor(Palette::StandardPaletteColor, 200));

    a.check("grayscale 0",   !Palette::isEditableColor(Palette::GrayscaleColor, 0));
    a.check("grayscale 150", !Palette::isEditableColor(Palette::GrayscaleColor, 150));
    a.check("grayscale 160", !Palette::isEditableColor(Palette::GrayscaleColor, 160));
    a.check("grayscale 200", !Palette::isEditableColor(Palette::GrayscaleColor, 200));
}
