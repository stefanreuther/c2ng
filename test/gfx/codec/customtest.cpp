/**
  *  \file test/gfx/codec/customtest.cpp
  *  \brief Test for gfx::codec::Custom
  */

#include "gfx/codec/custom.hpp"

#include "afl/io/internalstream.hpp"
#include "afl/test/testrunner.hpp"
#include "gfx/palettizedpixmap.hpp"

using gfx::codec::Custom;

/* Tests for loading are in CCImageLoader. */

AFL_TEST("gfx.codec.Custom:save", a)
{
    // Create a palettized pixmap.
    // Make it 3x5 pixels.
    afl::base::Ref<gfx::Canvas> can = gfx::PalettizedPixmap::create(3, 5)->makeCanvas();

    // Populate the pixmap
    const size_t NUM_COLORS = 3;
    static const gfx::ColorQuad_t palette[NUM_COLORS] = {
        COLORQUAD_FROM_RGB(64,  0, 64),
        COLORQUAD_FROM_RGB( 0, 48,  0),
        COLORQUAD_FROM_RGB( 0,  0, 64)
    };
    gfx::Color_t colors[NUM_COLORS];
    can->setPalette(0, palette, colors);

    // Verify that we got a palettized pixmap
    a.checkEqual("01. color", colors[0], 0U);
    a.checkEqual("02. color", colors[1], 1U);
    a.checkEqual("03. color", colors[2], 2U);

    // Draw some pixels
    can->drawBar(gfx::Rectangle(0, 0, 100, 100), 0, 0, gfx::FillPattern::SOLID, gfx::OPAQUE_ALPHA);
    can->drawPixel(gfx::Point(1, 1), 1, gfx::OPAQUE_ALPHA);
    can->drawPixel(gfx::Point(1, 3), 2, gfx::OPAQUE_ALPHA);
    can->drawPixel(gfx::Point(1, 4), 2, gfx::OPAQUE_ALPHA);

    // Shape:
    //    000
    //    010
    //    000
    //    020
    //    020

    // 4-bit uncompressed
    {
        afl::io::InternalStream out;
        Custom(Custom::FourBit, false).save(*can, out);

        static const uint8_t EXPECTED[] = {
            'C','C',3,0,5,0,
            16,0,16, 0,0,16, 0,12,0, 0,0,0, 0,0,0,
            0xAA, 0x0A,
            0xCA, 0x0A,
            0xAA, 0x0A,
            0xBA, 0x0A,
            0xBA, 0x0A,
        };
        a.checkEqualContent<uint8_t>("11. 4-bit uncompressed", out.getContent(), EXPECTED);
    }

    // 4-bit compressed
    {
        afl::io::InternalStream out;
        Custom(Custom::FourBit, true).save(*can, out);

        static const uint8_t EXPECTED[] = {
            31,0,0,0,
            31,0,255,
            'C','C',3,0,5,0,
            16,0,16, 0,0,16, 0,12, 255,7,0,
            0xAA, 0x0A,
            0xCA, 0x0A,
            0xAA, 0x0A,
            0xBA, 0x0A,
            0xBA, 0x0A,
            0,0,
        };
        a.checkEqualContent<uint8_t>("21. 4-bit compressed", out.getContent(), EXPECTED);
    }

    // 8-bit uncompressed
    {
        afl::io::InternalStream out;
        Custom(Custom::EightBit, false).save(*can, out);

        // (0,48,0) is mapped to (0,44,0) = 0xA2 (external) = 130 (internal)
        // (64,0,64) is allocated dynamically as 0xC0
        // (0,0,64) is allocated dynamically as 0xC1
        static const uint8_t EXPECTED[] = {
            'C','D',3,0,5,0,
            16,0,16, 0,0,16, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
            0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
            0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
            0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
            0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
            0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
            0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
            0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
            0xC0, 0xC0, 0xC0,
            0xC0, 0xA2, 0xC0,
            0xC0, 0xC0, 0xC0,
            0xC0, 0xC1, 0xC0,
            0xC0, 0xC1, 0xC0,
        };
        a.checkEqualContent<uint8_t>("31. 8-bit uncompressed", out.getContent(), EXPECTED);
    }

    // 8-bit compressed
    {
        afl::io::InternalStream out;
        Custom(Custom::EightBit, true).save(*can, out);

        static const uint8_t EXPECTED[] = {
            213,0,0,0,
            213,0,255,
            'C','D',3,0,5,0,
            16,0,16, 0,0,16, 255,186,0,
            255,4,0xC0,
            0xA2,
            255,5,0xC0,
            0xC1, 0xC0,
            0xC0, 0xC1, 0xC0,
            0,0,
        };
        a.checkEqualContent<uint8_t>("41. 8-bit compressed", out.getContent(), EXPECTED);
    }
}
