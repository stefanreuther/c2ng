/**
  *  \file u/t_gfx_types.cpp
  *  \brief Test for gfx::Types
  */

#include "gfx/types.hpp"

#include "t_gfx.hpp"

/** Test mixColorComponent. */
void
TestGfxTypes::testMixColorComponent()
{
    // Mixing anything with opaque alpha must result in that thing
    for (int i = 0; i <= 255; ++i) {
        for (int bg = 0; bg <= 255; ++bg) {
            TS_ASSERT_EQUALS(gfx::mixColorComponent(bg, i, gfx::OPAQUE_ALPHA), i);
        }
    }

    // Mixing anything with transparent alpha must result in background
    for (int i = 0; i <= 255; ++i) {
        for (int bg = 0; bg <= 255; ++bg) {
            TS_ASSERT_EQUALS(gfx::mixColorComponent(bg, i, gfx::TRANSPARENT_ALPHA), bg);
        }
    }
}

/** Test mixColor. */
void
TestGfxTypes::testMixColor()
{
    // Possible border caes
    TS_ASSERT_EQUALS(gfx::mixColor(COLORQUAD_FROM_RGBA(  0,   0,   0,   0), COLORQUAD_FROM_RGBA(  0,   0,   0,   0),   0), COLORQUAD_FROM_RGBA(  0,   0,   0,   0));
    TS_ASSERT_EQUALS(gfx::mixColor(COLORQUAD_FROM_RGBA(255, 255, 255, 255), COLORQUAD_FROM_RGBA(255, 255, 255, 255), 255), COLORQUAD_FROM_RGBA(255, 255, 255, 255));
    TS_ASSERT_EQUALS(gfx::mixColor(COLORQUAD_FROM_RGBA(255,   0, 255,   0), COLORQUAD_FROM_RGBA(255,   0, 255,   0), 255), COLORQUAD_FROM_RGBA(255,   0, 255,   0));
    TS_ASSERT_EQUALS(gfx::mixColor(COLORQUAD_FROM_RGBA(  0, 255,   0, 255), COLORQUAD_FROM_RGBA(  0, 255,   0, 255), 255), COLORQUAD_FROM_RGBA(0,   255,   0, 255));
    TS_ASSERT_EQUALS(gfx::mixColor(COLORQUAD_FROM_RGBA(  0, 255,   0, 255), COLORQUAD_FROM_RGBA(  0, 255,   0, 255),   0), COLORQUAD_FROM_RGBA(0,   255,   0, 255));

    // Some useful cases
    // - 50/50 gray mix
    TS_ASSERT_EQUALS(gfx::mixColor(COLORQUAD_FROM_RGBA(  0,   0,   0, 255), COLORQUAD_FROM_RGBA(100, 100, 100, 255), 128), COLORQUAD_FROM_RGBA( 50,  50,  50, 255));

    // - 50/50 color mix (both directions)
    TS_ASSERT_EQUALS(gfx::mixColor(COLORQUAD_FROM_RGBA( 50, 150,   0, 255), COLORQUAD_FROM_RGBA(100, 100, 100, 255), 128), COLORQUAD_FROM_RGBA( 75, 125,  50, 255));
    TS_ASSERT_EQUALS(gfx::mixColor(COLORQUAD_FROM_RGBA(100, 100, 100, 255), COLORQUAD_FROM_RGBA( 50, 150,   0, 255), 128), COLORQUAD_FROM_RGBA( 75, 125,  50, 255));

    // - color in 25% intensity (both directions)
    TS_ASSERT_EQUALS(gfx::mixColor(COLORQUAD_FROM_RGBA( 50, 150,   0, 255), COLORQUAD_FROM_RGBA(100, 100, 100, 255),  64), COLORQUAD_FROM_RGBA( 62, 138,  25, 255));
    TS_ASSERT_EQUALS(gfx::mixColor(COLORQUAD_FROM_RGBA(100, 100, 100, 255), COLORQUAD_FROM_RGBA( 50, 150,   0, 255),  64), COLORQUAD_FROM_RGBA( 88, 112,  75, 255));
}

/** Test addColor. */
void
TestGfxTypes::testAddColor()
{
    // Standard case
    TS_ASSERT_EQUALS(gfx::addColor(COLORQUAD_FROM_RGBA(  1,   2,   3,   4), COLORQUAD_FROM_RGBA(  5,   6,   7,   8)), COLORQUAD_FROM_RGBA(  6,   8,  10,  12));

    // Overflow cases
    TS_ASSERT_EQUALS(gfx::addColor(COLORQUAD_FROM_RGBA(255, 255, 255, 255), COLORQUAD_FROM_RGBA(255, 255, 255, 255)), COLORQUAD_FROM_RGBA(255, 255, 255, 255));
    TS_ASSERT_EQUALS(gfx::addColor(COLORQUAD_FROM_RGBA(  1,   1,   1,   1), COLORQUAD_FROM_RGBA(255, 255, 255, 255)), COLORQUAD_FROM_RGBA(255, 255, 255, 255));
    TS_ASSERT_EQUALS(gfx::addColor(COLORQUAD_FROM_RGBA(255, 255, 255, 255), COLORQUAD_FROM_RGBA(  1,   1,   1,   1)), COLORQUAD_FROM_RGBA(255, 255, 255, 255));
    TS_ASSERT_EQUALS(gfx::addColor(COLORQUAD_FROM_RGBA(255,   0, 255,   0), COLORQUAD_FROM_RGBA(255, 255, 255, 255)), COLORQUAD_FROM_RGBA(255, 255, 255, 255));
    TS_ASSERT_EQUALS(gfx::addColor(COLORQUAD_FROM_RGBA(  0, 255,   0, 255), COLORQUAD_FROM_RGBA(255, 255, 255, 255)), COLORQUAD_FROM_RGBA(255, 255, 255, 255));
}

/** Test getColorDistance. */
void
TestGfxTypes::testGetColorDistance()
{
    // Equality
    TS_ASSERT_EQUALS(gfx::getColorDistance(COLORQUAD_FROM_RGBA(  1,   2,   3,   4), COLORQUAD_FROM_RGBA(  1,  2,  3,  4)), 0);

    // Distances: verify monotonicity for Red component
    int last = 0;
    for (int i = 1; i <= 255; ++i) {
        int now = gfx::getColorDistance(COLORQUAD_FROM_RGBA(0, 0, 0, gfx::OPAQUE_ALPHA), COLORQUAD_FROM_RGBA(i, 0, 0, gfx::OPAQUE_ALPHA));
        TS_ASSERT(now > last);
        TS_ASSERT_EQUALS(now, gfx::getColorDistance(COLORQUAD_FROM_RGBA(i, 0, 0, gfx::OPAQUE_ALPHA), COLORQUAD_FROM_RGBA(0, 0, 0, gfx::OPAQUE_ALPHA)));
        last = now;
    }

    // verify monotonicity for Green component
    last = 0;
    for (int i = 1; i <= 255; ++i) {
        int now = gfx::getColorDistance(COLORQUAD_FROM_RGBA(0, 0, 0, gfx::OPAQUE_ALPHA), COLORQUAD_FROM_RGBA(0, i, 0, gfx::OPAQUE_ALPHA));
        TS_ASSERT(now > last);
        TS_ASSERT_EQUALS(now, gfx::getColorDistance(COLORQUAD_FROM_RGBA(0, i, 0, gfx::OPAQUE_ALPHA), COLORQUAD_FROM_RGBA(0, 0, 0, gfx::OPAQUE_ALPHA)));
        last = now;
    }

    // verify monotonicity for Blue component
    last = 0;
    for (int i = 1; i <= 255; ++i) {
        int now = gfx::getColorDistance(COLORQUAD_FROM_RGBA(0, 0, 0, gfx::OPAQUE_ALPHA), COLORQUAD_FROM_RGBA(0, 0, i, gfx::OPAQUE_ALPHA));
        TS_ASSERT(now > last);
        TS_ASSERT_EQUALS(now, gfx::getColorDistance(COLORQUAD_FROM_RGBA(0, 0, i, gfx::OPAQUE_ALPHA), COLORQUAD_FROM_RGBA(0, 0, 0, gfx::OPAQUE_ALPHA)));
        last = now;
    }

    // verify monotonicity for Gray
    last = 0;
    for (int i = 1; i <= 255; ++i) {
        int now = gfx::getColorDistance(COLORQUAD_FROM_RGBA(0, 0, 0, gfx::OPAQUE_ALPHA), COLORQUAD_FROM_RGBA(i, i, i, gfx::OPAQUE_ALPHA));
        TS_ASSERT(now > last);
        TS_ASSERT_EQUALS(now, gfx::getColorDistance(COLORQUAD_FROM_RGBA(i, i, i, gfx::OPAQUE_ALPHA), COLORQUAD_FROM_RGBA(0, 0, 0, gfx::OPAQUE_ALPHA)));
        TS_ASSERT(now > gfx::getColorDistance(COLORQUAD_FROM_RGBA(0, 0, 0, gfx::OPAQUE_ALPHA), COLORQUAD_FROM_RGBA(i, 0, 0, gfx::OPAQUE_ALPHA)));
        TS_ASSERT(now > gfx::getColorDistance(COLORQUAD_FROM_RGBA(0, 0, 0, gfx::OPAQUE_ALPHA), COLORQUAD_FROM_RGBA(0, i, 0, gfx::OPAQUE_ALPHA)));
        TS_ASSERT(now > gfx::getColorDistance(COLORQUAD_FROM_RGBA(0, 0, 0, gfx::OPAQUE_ALPHA), COLORQUAD_FROM_RGBA(0, 0, i, gfx::OPAQUE_ALPHA)));
        last = now;
    }

    // last now is the maximum possible distance. Differing alpha must still be higher.
    TS_ASSERT(gfx::getColorDistance(COLORQUAD_FROM_RGBA(0, 0, 0, 0), COLORQUAD_FROM_RGBA(10, 10, 10, 10)) > last);
}

/** Test parseColor(). */
void
TestGfxtypes::testParseColor()
{
    gfx::ColorQuad_t q;

    // ok: #rgb
    {
        util::StringParser p("#234");
        TS_ASSERT(gfx::parseColor(p, q));
        TS_ASSERT(p.parseEnd());
        TS_ASSERT_EQUALS(q, COLORQUAD_FROM_RGBA(0x22, 0x33, 0x44, 0xFF));
    }

    // ok: #rrggbb
    {
        util::StringParser p("#124567");
        TS_ASSERT(gfx::parseColor(p, q));
        TS_ASSERT(p.parseEnd());
        TS_ASSERT_EQUALS(q, COLORQUAD_FROM_RGBA(0x12, 0x45, 0x67, 0xFF));
    }

    // ok: #rgba
    {
        util::StringParser p("#234A");
        TS_ASSERT(gfx::parseColor(p, q));
        TS_ASSERT(p.parseEnd());
        TS_ASSERT_EQUALS(q, COLORQUAD_FROM_RGBA(0x22, 0x33, 0x44, 0xAA));
    }

    // ok: #rrggbbaa
    {
        util::StringParser p("#234A95CD");
        TS_ASSERT(gfx::parseColor(p, q));
        TS_ASSERT(p.parseEnd());
        TS_ASSERT_EQUALS(q, COLORQUAD_FROM_RGBA(0x23, 0x4A, 0x95, 0xCD));
    }

    // ok: rgb(r,g,b)
    {
        util::StringParser p("rgb ( 1, 2 , 3 )");
        TS_ASSERT(gfx::parseColor(p, q));
        TS_ASSERT(p.parseEnd());
        TS_ASSERT_EQUALS(q, COLORQUAD_FROM_RGBA(0x01, 0x02, 0x03, 0xFF));
    }

    // ok: rgb(r,g,b,a)
    {
        util::StringParser p("rgb(11,22,33,44)");
        TS_ASSERT(gfx::parseColor(p, q));
        TS_ASSERT(p.parseEnd());
        TS_ASSERT_EQUALS(q, COLORQUAD_FROM_RGBA(11, 22, 33, 44));
    }

    // ok: using percent
    {
        util::StringParser p("rgb(25 % ,22,33,33%)");
        TS_ASSERT(gfx::parseColor(p, q));
        TS_ASSERT(p.parseEnd());
        TS_ASSERT_EQUALS(q, COLORQUAD_FROM_RGBA(0x40, 22, 33, 0x54));
    }

    // failure: out-of-range value
    {
        util::StringParser p("rgb(1000,200,300)");
        TS_ASSERT(!gfx::parseColor(p, q));
    }

    // failure: out-of-range percentage
    {
        util::StringParser p("rgb(10%,20%,300%)");
        TS_ASSERT(!gfx::parseColor(p, q));
    }

    // failure: too few args
    {
        util::StringParser p("rgb(4,5)");
        TS_ASSERT(!gfx::parseColor(p, q));
    }

    // failure: too many args
    {
        util::StringParser p("rgb(4,5,6,7,8)");
        TS_ASSERT(!gfx::parseColor(p, q));
    }

    // failure: too short
    {
        util::StringParser p("#12");
        TS_ASSERT(!gfx::parseColor(p, q));
    }

    // failure: wrong length
    {
        util::StringParser p("#12345");
        TS_ASSERT(!gfx::parseColor(p, q));
    }

    // failure: bad keyword
    {
        util::StringParser p("lolwut?");
        TS_ASSERT(!gfx::parseColor(p, q));
    }

    // failure: bad hex
    {
        util::StringParser p("#lolwut");
        TS_ASSERT(!gfx::parseColor(p, q));
    }

    // failure: bad number
    {
        util::StringParser p("rgb(lol,wut,wtf)");
        TS_ASSERT(!gfx::parseColor(p, q));
    }
}

