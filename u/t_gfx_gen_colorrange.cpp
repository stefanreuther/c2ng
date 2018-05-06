/**
  *  \file u/t_gfx_gen_colorrange.cpp
  *  \brief Test for gfx::gen::ColorRange
  */

#include "gfx/gen/colorrange.hpp"

#include "t_gfx_gen.hpp"

/** Get ColorRange::get(), maximum precision. */
void
TestGfxGenColorRange::testGetMax()
{
    // Positive delta
    gfx::gen::ColorRange a(COLORQUAD_FROM_RGBA(0,0,0,0), COLORQUAD_FROM_RGBA(0xFF,0xFF,0xFF,0xFF), 256);
    for (int i = 0; i < 256; ++i) {
        TS_ASSERT_EQUALS(RED_FROM_COLORQUAD(a.get(i)), i);
        TS_ASSERT_EQUALS(GREEN_FROM_COLORQUAD(a.get(i)), i);
        TS_ASSERT_EQUALS(BLUE_FROM_COLORQUAD(a.get(i)), i);
        TS_ASSERT_EQUALS(ALPHA_FROM_COLORQUAD(a.get(i)), i);
    }

    // Negative delta
    gfx::gen::ColorRange b(COLORQUAD_FROM_RGBA(0xFF,0xFF,0xFF,0xFF), COLORQUAD_FROM_RGBA(0,0,0,0), 256);
    for (int i = 0; i < 256; ++i) {
        TS_ASSERT_EQUALS(RED_FROM_COLORQUAD(b.get(i)), 255-i);
        TS_ASSERT_EQUALS(GREEN_FROM_COLORQUAD(b.get(i)), 255-i);
        TS_ASSERT_EQUALS(BLUE_FROM_COLORQUAD(b.get(i)), 255-i);
        TS_ASSERT_EQUALS(ALPHA_FROM_COLORQUAD(b.get(i)), 255-i);
    }
}

/** Get ColorRange::get(), limited precision. */
void
TestGfxGenColorRange::testGetLimit()
{
    gfx::gen::ColorRange a(COLORQUAD_FROM_RGBA(0,0,0,0xFF), COLORQUAD_FROM_RGBA(0xFF,0x80,100,0), 10);

    struct Expect {
        int limit;
        gfx::ColorQuad_t value;
    };
    static const Expect ex[] = {
        { 26,  COLORQUAD_FROM_RGBA(  0,   0,  0, 255) },
        { 52,  COLORQUAD_FROM_RGBA( 28,  14, 11, 227) },
        { 77,  COLORQUAD_FROM_RGBA( 56,  28, 22, 199) },
        { 103, COLORQUAD_FROM_RGBA( 85,  42, 33, 170) },
        { 128, COLORQUAD_FROM_RGBA(113,  56, 44, 142) },
        { 154, COLORQUAD_FROM_RGBA(141,  71, 55, 114) },
        { 180, COLORQUAD_FROM_RGBA(170,  85, 66,  85) },
        { 205, COLORQUAD_FROM_RGBA(198,  99, 77,  57) },
        { 231, COLORQUAD_FROM_RGBA(226, 113, 88,  29) },
        { 256, COLORQUAD_FROM_RGBA(255, 128, 100,  0) },
    };

    int i = 0;
    for (size_t x = 0; x < sizeof(ex)/sizeof(ex[0]); ++x) {
        while (i < ex[x].limit) {
            TS_ASSERT_EQUALS(a.get(i), ex[x].value);
            ++i;
        }
    }
}

/** Test ColorRange::get(), simple case. */
void
TestGfxGenColorRange::testGetSimple()
{
    gfx::gen::ColorRange testee(COLORQUAD_FROM_RGBA(0x12,0x34,0x45,0x67));
    for (int i = 0; i < 256; ++i) {
        TS_ASSERT_EQUALS(testee.get(i), COLORQUAD_FROM_RGBA(0x12,0x34,0x45,0x67));
    }
}

/** Test ColorRange::get(), one section. */
void
TestGfxGenColorRange::testGetOne()
{
    gfx::gen::ColorRange testee(COLORQUAD_FROM_RGBA(0,0,0,0xFF), COLORQUAD_FROM_RGBA(0xFF,0x80,100,0), 1);
    for (int i = 0; i < 256; ++i) {
        TS_ASSERT_EQUALS(testee.get(i), COLORQUAD_FROM_RGBA(0,0,0,0xFF));
    }
}

/** Test ColorRange::parse(). */
void
TestGfxGenColorRange::testParse()
{
    // Initialisation
    gfx::gen::ColorRange testee;
    TS_ASSERT_EQUALS(testee.getStartColor(), COLORQUAD_FROM_RGBA(0,0,0,0));
    TS_ASSERT_EQUALS(testee.getEndColor(), COLORQUAD_FROM_RGBA(0,0,0,0));
    TS_ASSERT_EQUALS(testee.getNumSteps(), gfx::gen::ColorRange::MAX_STEPS);

    // Parse
    {
        util::StringParser p("#321608");
        TS_ASSERT_EQUALS(testee.parse(p), true);
        TS_ASSERT_EQUALS(p.parseEnd(), true);
        TS_ASSERT_EQUALS(testee.getStartColor(), COLORQUAD_FROM_RGBA(0x32,0x16,8,255));
        TS_ASSERT_EQUALS(testee.getEndColor(),   COLORQUAD_FROM_RGBA(0x32,0x16,8,255));
        TS_ASSERT_EQUALS(testee.getNumSteps(),   gfx::gen::ColorRange::MAX_STEPS);
    }

    {
        util::StringParser p("#119-#442/3x");
        TS_ASSERT_EQUALS(testee.parse(p), true);
        TS_ASSERT_EQUALS(p.parseCharacter('x'), true);
        TS_ASSERT_EQUALS(p.parseEnd(), true);
        TS_ASSERT_EQUALS(testee.getStartColor(), COLORQUAD_FROM_RGBA(0x11,0x11,0x99,255));
        TS_ASSERT_EQUALS(testee.getEndColor(),   COLORQUAD_FROM_RGBA(0x44,0x44,0x22,255));
        TS_ASSERT_EQUALS(testee.getNumSteps(),   3);
    }
}

/** Test ColorRange::parse(), error case. */
void
TestGfxGenColorRange::testParseError()
{
    gfx::gen::ColorRange testee;
    {
        util::StringParser p("#123-");
        TS_ASSERT_EQUALS(testee.parse(p), false);
        TS_ASSERT(p.parseEnd());
    }
    {
        util::StringParser p("*");
        TS_ASSERT_EQUALS(testee.parse(p), false);
        TS_ASSERT(p.parseCharacter('*'));
    }
    {
        util::StringParser p("#123/x");
        TS_ASSERT_EQUALS(testee.parse(p), false);
        TS_ASSERT(p.parseCharacter('x'));
    }
}
