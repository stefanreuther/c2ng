/**
  *  \file test/gfx/bitmapglyphtest.cpp
  *  \brief Test for gfx::BitmapGlyph
  */

#include "gfx/bitmapglyph.hpp"

#include "afl/test/testrunner.hpp"
#include "gfx/basecontext.hpp"
#include "gfx/palettizedpixmap.hpp"

/** Main test. */
AFL_TEST("gfx.BitmapGlyph:basics", a)
{
    // Testee
    gfx::BitmapGlyph t(4, 5);

    // Initial state
    a.checkEqual("01. getWidth", t.getWidth(), 4);
    a.checkEqual("02. getHeight", t.getHeight(), 5);
    a.checkEqual("03. pixel", t.get(0, 0), false);
    a.checkEqual("04. pixel", t.get(1, 0), false);
    a.checkEqual("05. pixel", t.get(1, 1), false);

    // Make a glyph
    t.set(0, 0, true); t.set(1, 0, true); t.set(2, 0, true);
    t.set(0, 1, true);                    t.set(2, 1, true);
    t.set(0, 2, true); t.set(1, 2, true); t.set(2, 2, true);
    t.set(0, 3, true); t.set(1, 3, true); t.set(2, 3, true);
    a.checkEqual("11. pixel", t.get(0, 0), true);
    a.checkEqual("12. pixel", t.get(1, 0), true);
    a.checkEqual("13. pixel", t.get(1, 1), false);

    t.addAAHint(0, 0);
    t.addAAHint(0, 3);
    t.addAAHint(2, 0);
    t.addAAHint(2, 3);
    t.set(1, 2, false);
    a.checkEqual("21. pixel", t.get(0, 0), false);   // reset by AA hint!
    a.checkEqual("22. pixel", t.get(1, 0), true);
    a.checkEqual("23. pixel", t.get(1, 1), false);

    // Verify data
    a.checkEqual("31. getAAData", t.getAAData().size(), 8U);
    a.checkEqual("32. data", t.getData().size(), 5U);
    a.checkEqual("33. data", t.getData()[0], 0x40U);
    a.checkEqual("34. data", t.getData()[1], 0xA0U);
    a.checkEqual("35. data", t.getData()[2], 0xA0U);
    a.checkEqual("36. data", t.getData()[3], 0x40U);

    // Draw normally
    {
        // - make palettized pixmap with sensible palette
        afl::base::Ref<gfx::PalettizedPixmap> pix(gfx::PalettizedPixmap::create(8, 8));
        for (int i = 0; i < 256; ++i) {
            pix->setPalette(uint8_t(i), COLORQUAD_FROM_RGBA(i, i, i, gfx::OPAQUE_ALPHA));
        }
        afl::base::Ref<gfx::Canvas> can(pix->makeCanvas());

        // - draw
        gfx::BaseContext ctx(*can);
        ctx.setRawColor(8);
        t.draw(ctx, gfx::Point(1, 2));

        // - verify
        static const uint8_t EXPECTED[] = {
            0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,
            0,4,8,4,0,0,0,0,
            0,8,0,8,0,0,0,0,
            0,8,0,8,0,0,0,0,
            0,4,8,4,0,0,0,0,
            0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,
        };
        a.checkEqualContent<uint8_t>("41. draw normally", pix->pixels(), EXPECTED);
    }

    // Draw normally with alpha
    {
        // - make palettized pixmap with sensible palette
        afl::base::Ref<gfx::PalettizedPixmap> pix(gfx::PalettizedPixmap::create(8, 8));
        for (int i = 0; i < 256; ++i) {
            pix->setPalette(uint8_t(i), COLORQUAD_FROM_RGBA(i, i, i, gfx::OPAQUE_ALPHA));
        }
        afl::base::Ref<gfx::Canvas> can(pix->makeCanvas());

        // - draw
        gfx::BaseContext ctx(*can);
        ctx.setRawColor(8);
        ctx.setAlpha(192);
        t.draw(ctx, gfx::Point(1, 2));

        // - verify
        static const uint8_t EXPECTED[] = {
            0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,
            0,3,6,3,0,0,0,0,
            0,6,0,6,0,0,0,0,
            0,6,0,6,0,0,0,0,
            0,3,6,3,0,0,0,0,
            0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,
        };
        a.checkEqualContent<uint8_t>("51. draw with alpha", pix->pixels(), EXPECTED);
    }

    // Draw with predefined colors
    {
        // - make palettized pixmap (no palette needed)
        afl::base::Ref<gfx::PalettizedPixmap> pix(gfx::PalettizedPixmap::create(8, 8));
        afl::base::Ref<gfx::Canvas> can(pix->makeCanvas());

        // - draw
        t.drawColored(*can, gfx::Point(1, 2), 5, 7);

        // - verify
        static const uint8_t EXPECTED[] = {
            0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,
            0,7,5,7,0,0,0,0,
            0,5,0,5,0,0,0,0,
            0,5,0,5,0,0,0,0,
            0,7,5,7,0,0,0,0,
            0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,
        };
        a.checkEqualContent<uint8_t>("61. drawColored", pix->pixels(), EXPECTED);
    }
}

/** Test construction. */
AFL_TEST("gfx.BitmapGlyph:construct", a)
{
    // Initialized with pixels
    static const uint8_t DATA[] = {
        0x80, 0x1F,
        0x81, 0x2F,
        0x80, 0x4F
    };
    gfx::BitmapGlyph g(12, 3, DATA);
    a.checkEqual("01. data", g.getData().size(), 6U);
    a.checkEqual("02. data", g.getData()[0], 0x80U);
    a.checkEqual("03. data", g.getData()[1], 0x1FU);
    a.checkEqual("04. getWidth", g.getWidth(), 12);
    a.checkEqual("05. getHeight", g.getHeight(), 3);
    a.checkEqual("06. pixel", g.get(0, 0), true);
    a.checkEqual("07. pixel", g.get(-1, 0), false);

    // - make palettized pixmap (no palette needed)
    afl::base::Ref<gfx::PalettizedPixmap> pix(gfx::PalettizedPixmap::create(20, 4));
    afl::base::Ref<gfx::Canvas> can(pix->makeCanvas());

    // - draw
    g.drawColored(*can, gfx::Point(0, 0), 1, 2);

    // - verify
    static const uint8_t EXPECTED[] = {
        1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,
        1,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,
        1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    };
    a.checkEqualContent<uint8_t>("11. drawColored", pix->pixels(), EXPECTED);
}

AFL_TEST("gfx.BitmapGlyph:getBytesForSize", a)
{
    a.checkEqual("01", gfx::BitmapGlyph::getBytesForSize(0, 0), 0U);
    a.checkEqual("02", gfx::BitmapGlyph::getBytesForSize(5, 0), 0U);
    a.checkEqual("03", gfx::BitmapGlyph::getBytesForSize(0, 5), 0U);

    a.checkEqual("11", gfx::BitmapGlyph::getBytesForSize(1, 1), 1U);
    a.checkEqual("12", gfx::BitmapGlyph::getBytesForSize(1, 9), 9U);

    a.checkEqual("21", gfx::BitmapGlyph::getBytesForSize(8, 1), 1U);
    a.checkEqual("22", gfx::BitmapGlyph::getBytesForSize(8, 9), 9U);

    a.checkEqual("31", gfx::BitmapGlyph::getBytesForSize(9, 1), 2U);
    a.checkEqual("32", gfx::BitmapGlyph::getBytesForSize(9, 9), 18U);

    a.checkEqual("41", gfx::BitmapGlyph::getBytesForSize(100, 100), 1300U);
}
