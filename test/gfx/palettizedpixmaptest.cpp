/**
  *  \file test/gfx/palettizedpixmaptest.cpp
  *  \brief Test for gfx::PalettizedPixmap
  */

#include "gfx/palettizedpixmap.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test. */
AFL_TEST("gfx.PalettizedPixmap", a)
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
    a.checkEqual("01. getPalette", resultColors[0], COLORQUAD_FROM_RGBA(0, 255, 0, 0));
    a.checkEqual("02. getPalette", resultColors[1], COLORQUAD_FROM_RGBA(128, 128, 128, 0));
    a.checkEqual("03. getPalette", resultColors[2], 0U);

    // Read palette with wrap
    testee->getPalette(255, resultColors);
    a.checkEqual("11. getPalette", resultColors[0], 0U);
    a.checkEqual("12. getPalette", resultColors[1], COLORQUAD_FROM_RGBA(0, 0, 0, 0));
    a.checkEqual("13. getPalette", resultColors[2], COLORQUAD_FROM_RGBA(0, 0, 42, 0));

    // Write palette with wrap
    resultColors[2] = COLORQUAD_FROM_RGBA(85, 0, 0, 0);
    testee->setPalette(255, resultColors);

    // Nearest color
    a.checkEqual("21. findNearestColor", testee->findNearestColor(COLORQUAD_FROM_RGBA(0, 0, 0, 0)), 0U);
    a.checkEqual("22. findNearestColor", testee->findNearestColor(COLORQUAD_FROM_RGBA(85, 0, 0, 0)), 1U);
    a.checkEqual("23. findNearestColor", testee->findNearestColor(COLORQUAD_FROM_RGBA(100, 0, 0, 0)), 1U);
    a.checkEqual("24. findNearestColor", testee->findNearestColor(COLORQUAD_FROM_RGBA(0, 200, 0, 0)), 5U);
    a.checkEqual("25. findNearestColor", testee->findNearestColor(COLORQUAD_FROM_RGBA(100, 100, 100, 0)), 7U);

    // Pixel content
    a.checkEqual("31. pixel size",   testee->pixels().size(), 35U);
    a.checkEqual("32. getSize",      testee->getSize(), gfx::Point(5, 7));
    a.checkEqual("33. getWidth",     testee->getWidth(), 5);
    a.checkEqual("34. getHeight",    testee->getHeight(), 7);
    a.checkEqual("35. row size",     testee->row(0).size(), 5U);
    a.checkEqual("36. pixel value", *testee->row(0).at(0), 0U);
    a.checkEqual("37. row size",     testee->row(6).size(), 5U);
    a.checkEqual("38. row size",     testee->row(7).size(), 0U);

    afl::base::Memory<const uint8_t> pixels = testee->pixels();
    while (const uint8_t* p = pixels.eat()) {
        a.checkEqual("41. pixel value", *p, 0U);
    }

    // Canvas
    afl::base::Ref<gfx::Canvas> can(testee->makeCanvas());
    a.checkEqual("51. getBitsPerPixel", can->getBitsPerPixel(), 8);

    // Encode/decode
    {
        static const gfx::ColorQuad_t colors[] = {
            COLORQUAD_FROM_RGBA(0, 0, 85, 0),
            COLORQUAD_FROM_RGBA(0, 0, 170, 0),
            COLORQUAD_FROM_RGBA(0, 0, 255, 0)
        };
        gfx::Color_t handles[] = {4,4,4};
        can->setPalette(8, colors, handles);
        a.checkEqual("61. setPalette", handles[0], 8U);
        a.checkEqual("62. setPalette", handles[1], 9U);
        a.checkEqual("63. setPalette", handles[2], 10U);
    }
    {
        static const gfx::Color_t handles[] = {1,5,9};
        gfx::ColorQuad_t colors[4] = {5,5,5,5};
        can->decodeColors(handles, colors);
        a.checkEqual("64. decodeColors", colors[0], COLORQUAD_FROM_RGBA(85,0,0,0));
        a.checkEqual("65. decodeColors", colors[1], COLORQUAD_FROM_RGBA(0,170,0,0));
        a.checkEqual("66. decodeColors", colors[2], COLORQUAD_FROM_RGBA(0,0,170,0));
        a.checkEqual("67. decodeColors", colors[3], COLORQUAD_FROM_RGBA(0,0,0,0));
    }
    {
        static const gfx::ColorQuad_t colors[] = {
            COLORQUAD_FROM_RGBA(0,100,0,0),
            COLORQUAD_FROM_RGBA(120,110,130,0)
        };
        gfx::Color_t handles[] = {9,9,9};
        can->encodeColors(colors, handles);
        a.checkEqual("68. encodeColors", handles[0], 4U);
        a.checkEqual("69. encodeColors", handles[1], 7U);
        a.checkEqual("70. encodeColors", handles[2], 0U);
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
    a.checkEqualContent<uint8_t>("71. content", testee->pixels(), EXPECTED_CONTENT);
}
