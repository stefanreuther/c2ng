/**
  *  \file test/gfx/rgbapixmaptest.cpp
  *  \brief Test for gfx::RGBAPixmap
  */

#include "gfx/rgbapixmap.hpp"
#include "afl/test/testrunner.hpp"

/** Simple tests. */
AFL_TEST("gfx.RGBAPixmap", a)
{
    // Testee
    afl::base::Ref<gfx::RGBAPixmap> testee = gfx::RGBAPixmap::create(3, 5);

    // Pixel content
    a.checkEqual("01. pixel size",   testee->pixels().size(), 15U);
    a.checkEqual("02. getSize",      testee->getSize(), gfx::Point(3, 5));
    a.checkEqual("03. getWidth",     testee->getWidth(), 3);
    a.checkEqual("04. getHeight",    testee->getHeight(), 5);
    a.checkEqual("05. row size",     testee->row(0).size(), 3U);
    a.checkEqual("06. pixel value", *testee->row(0).at(0), 0U);
    a.checkEqual("07. row size",     testee->row(4).size(), 3U);
    a.checkEqual("08. row size",     testee->row(5).size(), 0U);

    afl::base::Memory<const gfx::ColorQuad_t> pixels = testee->pixels();
    while (const gfx::ColorQuad_t* p = pixels.eat()) {
        a.checkEqual("11. pixel value", *p, 0U);
    }

    // Canvas
    afl::base::Ref<gfx::Canvas> can(testee->makeCanvas());
    a.checkEqual("21. getBitsPerPixel", can->getBitsPerPixel(), 32);
    a.checkEqual("22. getSize", can->getSize(), gfx::Point(3, 5));

    // Encode/decode
    {
        static const gfx::ColorQuad_t colors[] = {
            COLORQUAD_FROM_RGBA(50, 0, 0, 0),
            COLORQUAD_FROM_RGBA(0, 50, 0, 0),
            COLORQUAD_FROM_RGBA(0, 0, 50, 0)
        };
        gfx::Color_t handles[] = {4,4,4};
        can->setPalette(8, colors, handles);
        a.checkEqual("31. setPalette", handles[0], COLORQUAD_FROM_RGBA(50, 0, 0, 0));
        a.checkEqual("32. setPalette", handles[1], COLORQUAD_FROM_RGBA(0, 50, 0, 0));
        a.checkEqual("33. setPalette", handles[2], COLORQUAD_FROM_RGBA(0, 0, 50, 0));
    }
    {
        static const gfx::Color_t handles[] = {COLORQUAD_FROM_RGBA(1,2,3,4),COLORQUAD_FROM_RGBA(5,6,7,9)};
        gfx::ColorQuad_t colors[4] = {5,5,5,5};
        can->decodeColors(handles, colors);
        a.checkEqual("34. decodeColors", colors[0], COLORQUAD_FROM_RGBA(1,2,3,4));
        a.checkEqual("35. decodeColors", colors[1], COLORQUAD_FROM_RGBA(5,6,7,9));
        a.checkEqual("36. decodeColors", colors[2], COLORQUAD_FROM_RGBA(0,0,0,0));
        a.checkEqual("37. decodeColors", colors[3], COLORQUAD_FROM_RGBA(0,0,0,0));
    }
    {
        static const gfx::ColorQuad_t colors[] = {
            COLORQUAD_FROM_RGBA(50, 0, 0, 0),
            COLORQUAD_FROM_RGBA(0, 50, 0, 0),
            COLORQUAD_FROM_RGBA(0, 0, 50, 0)
        };
        gfx::Color_t handles[] = {4,4,4};
        can->encodeColors(colors, handles);
        a.checkEqual("38. encodeColors", handles[0], COLORQUAD_FROM_RGBA(50, 0, 0, 0));
        a.checkEqual("39. encodeColors", handles[1], COLORQUAD_FROM_RGBA(0, 50, 0, 0));
        a.checkEqual("40. encodeColors", handles[2], COLORQUAD_FROM_RGBA(0, 0, 50, 0));
    }

    // Draw
    // - one bar
    can->drawBar(gfx::Rectangle(0, 0, 2, 4), COLORQUAD_FROM_RGBA(100,0,0,0), 0, gfx::FillPattern::SOLID, gfx::OPAQUE_ALPHA);
    // - another bar with transparency
    can->drawBar(gfx::Rectangle(1, 2, 3, 7), COLORQUAD_FROM_RGBA(0,100,0,0), 0, gfx::FillPattern::SOLID, 130);

    // Verify result
    static const gfx::ColorQuad_t EXPECTED_CONTENT[] = {
        COLORQUAD_FROM_RGBA(100,0,0,0), COLORQUAD_FROM_RGBA(100,0,0,0), 0,
        COLORQUAD_FROM_RGBA(100,0,0,0), COLORQUAD_FROM_RGBA(100,0,0,0), 0,
        COLORQUAD_FROM_RGBA(100,0,0,0), COLORQUAD_FROM_RGBA(50,50,0,0), COLORQUAD_FROM_RGBA(0,50,0,0),
        COLORQUAD_FROM_RGBA(100,0,0,0), COLORQUAD_FROM_RGBA(50,50,0,0), COLORQUAD_FROM_RGBA(0,50,0,0),
        0,                              COLORQUAD_FROM_RGBA(0,50,0,0),  COLORQUAD_FROM_RGBA(0,50,0,0),
    };
    a.checkEqualContent<gfx::ColorQuad_t>("41. content", testee->pixels(), EXPECTED_CONTENT);

    // Read pixels
    gfx::ColorQuad_t quads[4];
    can->getPixels(gfx::Point(1, 2), quads);
    a.checkEqual("51. getPixels", quads[0], COLORQUAD_FROM_RGBA(50,50,0,0));
    a.checkEqual("52. getPixels", quads[1], COLORQUAD_FROM_RGBA(0,50,0,0));
    a.checkEqual("53. getPixels", quads[2], COLORQUAD_FROM_RGBA(0,0,0,0));
    a.checkEqual("54. getPixels", quads[3], COLORQUAD_FROM_RGBA(0,0,0,0));

    can->getPixels(gfx::Point(-1, 2), quads);
    a.checkEqual("61. getPixels", quads[0], COLORQUAD_FROM_RGBA(0,0,0,0));
    a.checkEqual("62. getPixels", quads[1], COLORQUAD_FROM_RGBA(100,0,0,0));
    a.checkEqual("63. getPixels", quads[2], COLORQUAD_FROM_RGBA(50,50,0,0));
    a.checkEqual("64. getPixels", quads[3], COLORQUAD_FROM_RGBA(0,50,0,0));

    // Global alpha
    testee->setAlpha(77);
    can->getPixels(gfx::Point(1, 2), quads);
    a.checkEqual("71. getPixels", quads[0], COLORQUAD_FROM_RGBA(50,50,0,77));
    a.checkEqual("72. getPixels", quads[1], COLORQUAD_FROM_RGBA(0,50,0,77));
    a.checkEqual("73. getPixels", quads[2], COLORQUAD_FROM_RGBA(0,0,0,0));
    a.checkEqual("74. getPixels", quads[3], COLORQUAD_FROM_RGBA(0,0,0,0));
}
