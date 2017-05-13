/**
  *  \file u/t_gfx_palettizedpixmap.cpp
  *  \brief Test for gfx::PalettizedPixmap
  */

#include "gfx/palettizedpixmap.hpp"

#include "t_gfx.hpp"

/** Simple test. */
void
TestGfxPalettizedPixmap::testIt()
{
    // Testee
    afl::base::Ref<gfx::PalettizedPixmap> testee = gfx::PalettizedPixmap::create(5, 7);

    // Test palette
    static const gfx::ColorQuad_t COLORS[] = {
        COLORQUAD_FROM_RGBA(0, 0, 0, 0),      // 0
        COLORQUAD_FROM_RGBA(0, 0, 42, 0),     // 1
        COLORQUAD_FROM_RGBA(170, 0, 0, 0),    // 2
        COLORQUAD_FROM_RGBA(255, 0, 0, 0),    // 3
        COLORQUAD_FROM_RGBA(0, 85, 0, 0),     // 4
        COLORQUAD_FROM_RGBA(0, 170, 0, 0),    // 5
        COLORQUAD_FROM_RGBA(0, 255, 0, 0),    // 6
    };
    testee->setPalette(0, COLORS);
    testee->setPalette(7, COLORQUAD_FROM_RGBA(128, 128, 128, 0));

    // Read palette
    gfx::ColorQuad_t resultColors[3];
    testee->getPalette(6, resultColors);
    TS_ASSERT_EQUALS(resultColors[0], COLORQUAD_FROM_RGBA(0, 255, 0, 0));
    TS_ASSERT_EQUALS(resultColors[1], COLORQUAD_FROM_RGBA(128, 128, 128, 0));
    TS_ASSERT_EQUALS(resultColors[2], 0U);

    // Read palette with wrap
    testee->getPalette(255, resultColors);
    TS_ASSERT_EQUALS(resultColors[0], 0U);
    TS_ASSERT_EQUALS(resultColors[1], COLORQUAD_FROM_RGBA(0, 0, 0, 0));
    TS_ASSERT_EQUALS(resultColors[2], COLORQUAD_FROM_RGBA(0, 0, 42, 0));

    // Write palette with wrap
    resultColors[2] = COLORQUAD_FROM_RGBA(85, 0, 0, 0);
    testee->setPalette(255, resultColors);

    // Nearest color
    TS_ASSERT_EQUALS(testee->findNearestColor(COLORQUAD_FROM_RGBA(0, 0, 0, 0)), 0U);
    TS_ASSERT_EQUALS(testee->findNearestColor(COLORQUAD_FROM_RGBA(85, 0, 0, 0)), 1U);
    TS_ASSERT_EQUALS(testee->findNearestColor(COLORQUAD_FROM_RGBA(100, 0, 0, 0)), 1U);
    TS_ASSERT_EQUALS(testee->findNearestColor(COLORQUAD_FROM_RGBA(0, 200, 0, 0)), 5U);
    TS_ASSERT_EQUALS(testee->findNearestColor(COLORQUAD_FROM_RGBA(100, 100, 100, 0)), 7U);

    // Pixel content
    TS_ASSERT_EQUALS(testee->pixels().size(), 35U);
    TS_ASSERT_EQUALS(testee->getSize(), gfx::Point(5, 7));
    TS_ASSERT_EQUALS(testee->getWidth(), 5);
    TS_ASSERT_EQUALS(testee->getHeight(), 7);
    TS_ASSERT_EQUALS(testee->row(0).size(), 5U);
    TS_ASSERT_EQUALS(*testee->row(0).at(0), 0U);
    TS_ASSERT_EQUALS(testee->row(6).size(), 5U);
    TS_ASSERT_EQUALS(testee->row(7).size(), 0U);

    afl::base::Memory<const uint8_t> pixels = testee->pixels();
    while (const uint8_t* p = pixels.eat()) {
        TS_ASSERT_EQUALS(*p, 0U);
    }

    // Canvas
    afl::base::Ref<gfx::Canvas> can(testee->makeCanvas());
    TS_ASSERT_EQUALS(can->getBitsPerPixel(), 8);

    // Encode/decode
    {
        static const gfx::ColorQuad_t colors[] = {
            COLORQUAD_FROM_RGBA(0, 0, 85, 0),
            COLORQUAD_FROM_RGBA(0, 0, 170, 0),
            COLORQUAD_FROM_RGBA(0, 0, 255, 0)
        };
        gfx::Color_t handles[] = {4,4,4};
        can->setPalette(8, colors, handles);
        TS_ASSERT_EQUALS(handles[0], 8U);
        TS_ASSERT_EQUALS(handles[1], 9U);
        TS_ASSERT_EQUALS(handles[2], 10U);
    }
    {
        static const gfx::Color_t handles[] = {1,5,9};
        gfx::ColorQuad_t colors[4] = {5,5,5,5};
        can->decodeColors(handles, colors);
        TS_ASSERT_EQUALS(colors[0], COLORQUAD_FROM_RGBA(85,0,0,0));
        TS_ASSERT_EQUALS(colors[1], COLORQUAD_FROM_RGBA(0,170,0,0));
        TS_ASSERT_EQUALS(colors[2], COLORQUAD_FROM_RGBA(0,0,170,0));
        TS_ASSERT_EQUALS(colors[3], COLORQUAD_FROM_RGBA(0,0,0,0));
    }
    {
        static const gfx::ColorQuad_t colors[] = {
            COLORQUAD_FROM_RGBA(0,100,0,0),
            COLORQUAD_FROM_RGBA(120,110,130,0)
        };
        gfx::Color_t handles[] = {9,9,9};
        can->encodeColors(colors, handles);
        TS_ASSERT_EQUALS(handles[0], 4U);
        TS_ASSERT_EQUALS(handles[1], 7U);
        TS_ASSERT_EQUALS(handles[2], 0U);
    }

    // Draw
    // - one bar
    can->drawBar(gfx::Rectangle(0, 0, 3, 4), 3, 0, gfx::FillPattern::SOLID, gfx::OPAQUE_ALPHA);
    // - another bar with transparency. Transparency 130 makes the rounding end up at a mixed color of 2.
    can->drawBar(gfx::Rectangle(2, 2, 3, 4), 2, 0, gfx::FillPattern::SOLID, 130);

    // Verify result
    static const uint8_t EXPECTED_CONTENT[] = {
        3,3,3,0,0,
        3,3,3,0,0,
        3,3,2,1,1,
        3,3,2,1,1,
        0,0,1,1,1,
        0,0,1,1,1,
        0,0,0,0,0,
    };
    TS_ASSERT_EQUALS(sizeof(EXPECTED_CONTENT), testee->pixels().size());
    TS_ASSERT_SAME_DATA(EXPECTED_CONTENT, testee->pixels().unsafeData(), sizeof(EXPECTED_CONTENT));
}

