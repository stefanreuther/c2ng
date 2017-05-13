/**
  *  \file u/t_gfx_rgbapixmap.cpp
  *  \brief Test for gfx::RGBAPixmap
  */

#include "gfx/rgbapixmap.hpp"

#include "t_gfx.hpp"

/** Simple tests. */
void
TestGfxRGBAPixmap::testIt()
{
    // Testee
    afl::base::Ref<gfx::RGBAPixmap> testee = gfx::RGBAPixmap::create(3, 5);

    // Pixel content
    TS_ASSERT_EQUALS(testee->pixels().size(), 15U);
    TS_ASSERT_EQUALS(testee->getSize(), gfx::Point(3, 5));
    TS_ASSERT_EQUALS(testee->getWidth(), 3);
    TS_ASSERT_EQUALS(testee->getHeight(), 5);
    TS_ASSERT_EQUALS(testee->row(0).size(), 3U);
    TS_ASSERT_EQUALS(*testee->row(0).at(0), 0U);
    TS_ASSERT_EQUALS(testee->row(4).size(), 3U);
    TS_ASSERT_EQUALS(testee->row(5).size(), 0U);

    afl::base::Memory<const gfx::ColorQuad_t> pixels = testee->pixels();
    while (const gfx::ColorQuad_t* p = pixels.eat()) {
        TS_ASSERT_EQUALS(*p, 0U);
    }

    // Canvas
    afl::base::Ref<gfx::Canvas> can(testee->makeCanvas());
    TS_ASSERT_EQUALS(can->getBitsPerPixel(), 32);
    TS_ASSERT_EQUALS(can->getSize(), gfx::Point(3, 5));

    // Encode/decode
    {
        static const gfx::ColorQuad_t colors[] = {
            COLORQUAD_FROM_RGBA(50, 0, 0, 0),
            COLORQUAD_FROM_RGBA(0, 50, 0, 0),
            COLORQUAD_FROM_RGBA(0, 0, 50, 0)
        };
        gfx::Color_t handles[] = {4,4,4};
        can->setPalette(8, colors, handles);
        TS_ASSERT_EQUALS(handles[0], COLORQUAD_FROM_RGBA(50, 0, 0, 0));
        TS_ASSERT_EQUALS(handles[1], COLORQUAD_FROM_RGBA(0, 50, 0, 0));
        TS_ASSERT_EQUALS(handles[2], COLORQUAD_FROM_RGBA(0, 0, 50, 0));
    }
    {
        static const gfx::Color_t handles[] = {COLORQUAD_FROM_RGBA(1,2,3,4),COLORQUAD_FROM_RGBA(5,6,7,9)};
        gfx::ColorQuad_t colors[4] = {5,5,5,5};
        can->decodeColors(handles, colors);
        TS_ASSERT_EQUALS(colors[0], COLORQUAD_FROM_RGBA(1,2,3,4));
        TS_ASSERT_EQUALS(colors[1], COLORQUAD_FROM_RGBA(5,6,7,9));
        TS_ASSERT_EQUALS(colors[2], COLORQUAD_FROM_RGBA(0,0,0,0));
        TS_ASSERT_EQUALS(colors[3], COLORQUAD_FROM_RGBA(0,0,0,0));
    }
    {
        static const gfx::ColorQuad_t colors[] = {
            COLORQUAD_FROM_RGBA(50, 0, 0, 0),
            COLORQUAD_FROM_RGBA(0, 50, 0, 0),
            COLORQUAD_FROM_RGBA(0, 0, 50, 0)
        };
        gfx::Color_t handles[] = {4,4,4};
        can->encodeColors(colors, handles);
        TS_ASSERT_EQUALS(handles[0], COLORQUAD_FROM_RGBA(50, 0, 0, 0));
        TS_ASSERT_EQUALS(handles[1], COLORQUAD_FROM_RGBA(0, 50, 0, 0));
        TS_ASSERT_EQUALS(handles[2], COLORQUAD_FROM_RGBA(0, 0, 50, 0));
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
    TS_ASSERT_EQUALS(sizeof(EXPECTED_CONTENT)/sizeof(EXPECTED_CONTENT[0]), testee->pixels().size());
    TS_ASSERT_SAME_DATA(EXPECTED_CONTENT, testee->pixels().unsafeData(), sizeof(EXPECTED_CONTENT));

    // Read pixels
    gfx::ColorQuad_t quads[4];
    can->getPixels(gfx::Point(1, 2), quads);
    TS_ASSERT_EQUALS(quads[0], COLORQUAD_FROM_RGBA(50,50,0,0));
    TS_ASSERT_EQUALS(quads[1], COLORQUAD_FROM_RGBA(0,50,0,0));
    TS_ASSERT_EQUALS(quads[2], COLORQUAD_FROM_RGBA(0,0,0,0));
    TS_ASSERT_EQUALS(quads[3], COLORQUAD_FROM_RGBA(0,0,0,0));

    can->getPixels(gfx::Point(-1, 2), quads);
    TS_ASSERT_EQUALS(quads[0], COLORQUAD_FROM_RGBA(0,0,0,0));
    TS_ASSERT_EQUALS(quads[1], COLORQUAD_FROM_RGBA(100,0,0,0));
    TS_ASSERT_EQUALS(quads[2], COLORQUAD_FROM_RGBA(50,50,0,0));
    TS_ASSERT_EQUALS(quads[3], COLORQUAD_FROM_RGBA(0,50,0,0));

    // Global alpha
    testee->setAlpha(77);
    can->getPixels(gfx::Point(1, 2), quads);
    TS_ASSERT_EQUALS(quads[0], COLORQUAD_FROM_RGBA(50,50,0,77));
    TS_ASSERT_EQUALS(quads[1], COLORQUAD_FROM_RGBA(0,50,0,77));
    TS_ASSERT_EQUALS(quads[2], COLORQUAD_FROM_RGBA(0,0,0,0));
    TS_ASSERT_EQUALS(quads[3], COLORQUAD_FROM_RGBA(0,0,0,0));
}

