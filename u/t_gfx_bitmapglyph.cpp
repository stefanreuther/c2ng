/**
  *  \file u/t_gfx_bitmapglyph.cpp
  *  \brief Test for gfx::BitmapGlyph
  */

#include "gfx/bitmapglyph.hpp"

#include "t_gfx.hpp"
#include "gfx/palettizedpixmap.hpp"
#include "gfx/basecontext.hpp"

#define TS_ASSERT_SAME(got, expected) \
    TS_ASSERT_EQUALS(got.size(), sizeof(expected)); \
    TS_ASSERT_SAME_DATA(got.unsafeData(), expected, sizeof(expected))

/** Main test. */
void
TestGfxBitmapGlyph::testIt()
{
    // Testee
    gfx::BitmapGlyph t(4, 5);

    // Initial state
    TS_ASSERT_EQUALS(t.getWidth(), 4);
    TS_ASSERT_EQUALS(t.getHeight(), 5);
    TS_ASSERT_EQUALS(t.get(0, 0), false);
    TS_ASSERT_EQUALS(t.get(1, 0), false);
    TS_ASSERT_EQUALS(t.get(1, 1), false);

    // Make a glyph
    t.set(0, 0, true); t.set(1, 0, true); t.set(2, 0, true);
    t.set(0, 1, true);                    t.set(2, 1, true);
    t.set(0, 2, true); t.set(1, 2, true); t.set(2, 2, true);
    t.set(0, 3, true); t.set(1, 3, true); t.set(2, 3, true);
    TS_ASSERT_EQUALS(t.get(0, 0), true);
    TS_ASSERT_EQUALS(t.get(1, 0), true);
    TS_ASSERT_EQUALS(t.get(1, 1), false);

    t.addAAHint(0, 0);
    t.addAAHint(0, 3);
    t.addAAHint(2, 0);
    t.addAAHint(2, 3);
    t.set(1, 2, false);
    TS_ASSERT_EQUALS(t.get(0, 0), false);   // reset by AA hint!
    TS_ASSERT_EQUALS(t.get(1, 0), true);
    TS_ASSERT_EQUALS(t.get(1, 1), false);

    // Verify data
    TS_ASSERT_EQUALS(t.getAAData().size(), 8U);
    TS_ASSERT_EQUALS(t.getData().size(), 5U);
    TS_ASSERT_EQUALS(t.getData()[0], 0x40U);
    TS_ASSERT_EQUALS(t.getData()[1], 0xA0U);
    TS_ASSERT_EQUALS(t.getData()[2], 0xA0U);
    TS_ASSERT_EQUALS(t.getData()[3], 0x40U);

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
        TS_ASSERT_SAME(pix->pixels(), EXPECTED);
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
        TS_ASSERT_SAME(pix->pixels(), EXPECTED);
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
        TS_ASSERT_SAME(pix->pixels(), EXPECTED);
    }
}

/** Test construction. */
void
TestGfxBitmapGlyph::testConstruct()
{
    // Initialized with pixels
    {
        static const uint8_t DATA[] = {
            0x80, 0x1F,
            0x81, 0x2F,
            0x80, 0x4F
        };
        gfx::BitmapGlyph g(12, 3, DATA);
        TS_ASSERT_EQUALS(g.getData().size(), 6U);
        TS_ASSERT_EQUALS(g.getData()[0], 0x80U);
        TS_ASSERT_EQUALS(g.getData()[1], 0x1FU);
        TS_ASSERT_EQUALS(g.getWidth(), 12);
        TS_ASSERT_EQUALS(g.getHeight(), 3);
        TS_ASSERT_EQUALS(g.get(0, 0), true);
        TS_ASSERT_EQUALS(g.get(-1, 0), false);

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
        TS_ASSERT_SAME(pix->pixels(), EXPECTED);
    }

    // Size
    TS_ASSERT_EQUALS(gfx::BitmapGlyph::getBytesForSize(0, 0), 0U);
    TS_ASSERT_EQUALS(gfx::BitmapGlyph::getBytesForSize(5, 0), 0U);
    TS_ASSERT_EQUALS(gfx::BitmapGlyph::getBytesForSize(0, 5), 0U);

    TS_ASSERT_EQUALS(gfx::BitmapGlyph::getBytesForSize(1, 1), 1U);
    TS_ASSERT_EQUALS(gfx::BitmapGlyph::getBytesForSize(1, 9), 9U);

    TS_ASSERT_EQUALS(gfx::BitmapGlyph::getBytesForSize(8, 1), 1U);
    TS_ASSERT_EQUALS(gfx::BitmapGlyph::getBytesForSize(8, 9), 9U);

    TS_ASSERT_EQUALS(gfx::BitmapGlyph::getBytesForSize(9, 1), 2U);
    TS_ASSERT_EQUALS(gfx::BitmapGlyph::getBytesForSize(9, 9), 18U);

    TS_ASSERT_EQUALS(gfx::BitmapGlyph::getBytesForSize(100, 100), 1300U);
}
