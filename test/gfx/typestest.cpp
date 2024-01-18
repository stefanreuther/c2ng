/**
  *  \file test/gfx/typestest.cpp
  *  \brief Test for gfx::Types
  */

#include "gfx/types.hpp"
#include "afl/test/testrunner.hpp"

/** Test mixColorComponent. */
AFL_TEST("gfx.Types:mixColorComponent", a)
{
    // Mixing anything with opaque alpha must result in that thing
    for (int i = 0; i <= 255; ++i) {
        for (int bg = 0; bg <= 255; ++bg) {
            a.checkEqual("01", gfx::mixColorComponent(bg, i, gfx::OPAQUE_ALPHA), i);
        }
    }

    // Mixing anything with transparent alpha must result in background
    for (int i = 0; i <= 255; ++i) {
        for (int bg = 0; bg <= 255; ++bg) {
            a.checkEqual("11", gfx::mixColorComponent(bg, i, gfx::TRANSPARENT_ALPHA), bg);
        }
    }
}

/** Test mixColor. */
AFL_TEST("gfx.Types:mixColor", a)
{
    // Possible border caes
    a.checkEqual("01", gfx::mixColor(COLORQUAD_FROM_RGBA(  0,   0,   0,   0), COLORQUAD_FROM_RGBA(  0,   0,   0,   0),   0), COLORQUAD_FROM_RGBA(  0,   0,   0,   0));
    a.checkEqual("02", gfx::mixColor(COLORQUAD_FROM_RGBA(255, 255, 255, 255), COLORQUAD_FROM_RGBA(255, 255, 255, 255), 255), COLORQUAD_FROM_RGBA(255, 255, 255, 255));
    a.checkEqual("03", gfx::mixColor(COLORQUAD_FROM_RGBA(255,   0, 255,   0), COLORQUAD_FROM_RGBA(255,   0, 255,   0), 255), COLORQUAD_FROM_RGBA(255,   0, 255,   0));
    a.checkEqual("04", gfx::mixColor(COLORQUAD_FROM_RGBA(  0, 255,   0, 255), COLORQUAD_FROM_RGBA(  0, 255,   0, 255), 255), COLORQUAD_FROM_RGBA(0,   255,   0, 255));
    a.checkEqual("05", gfx::mixColor(COLORQUAD_FROM_RGBA(  0, 255,   0, 255), COLORQUAD_FROM_RGBA(  0, 255,   0, 255),   0), COLORQUAD_FROM_RGBA(0,   255,   0, 255));

    // Some useful cases
    // - 50/50 gray mix
    a.checkEqual("11", gfx::mixColor(COLORQUAD_FROM_RGBA(  0,   0,   0, 255), COLORQUAD_FROM_RGBA(100, 100, 100, 255), 128), COLORQUAD_FROM_RGBA( 50,  50,  50, 255));

    // - 50/50 color mix (both directions)
    a.checkEqual("21", gfx::mixColor(COLORQUAD_FROM_RGBA( 50, 150,   0, 255), COLORQUAD_FROM_RGBA(100, 100, 100, 255), 128), COLORQUAD_FROM_RGBA( 75, 125,  50, 255));
    a.checkEqual("22", gfx::mixColor(COLORQUAD_FROM_RGBA(100, 100, 100, 255), COLORQUAD_FROM_RGBA( 50, 150,   0, 255), 128), COLORQUAD_FROM_RGBA( 75, 125,  50, 255));

    // - color in 25% intensity (both directions)
    a.checkEqual("31", gfx::mixColor(COLORQUAD_FROM_RGBA( 50, 150,   0, 255), COLORQUAD_FROM_RGBA(100, 100, 100, 255),  64), COLORQUAD_FROM_RGBA( 62, 138,  25, 255));
    a.checkEqual("32", gfx::mixColor(COLORQUAD_FROM_RGBA(100, 100, 100, 255), COLORQUAD_FROM_RGBA( 50, 150,   0, 255),  64), COLORQUAD_FROM_RGBA( 88, 112,  75, 255));
}

/** Test addColor. */
AFL_TEST("gfx.Types:addColor", a)
{
    // Standard case
    a.checkEqual("01", gfx::addColor(COLORQUAD_FROM_RGBA(  1,   2,   3,   4), COLORQUAD_FROM_RGBA(  5,   6,   7,   8)), COLORQUAD_FROM_RGBA(  6,   8,  10,  12));

    // Overflow cases
    a.checkEqual("11", gfx::addColor(COLORQUAD_FROM_RGBA(255, 255, 255, 255), COLORQUAD_FROM_RGBA(255, 255, 255, 255)), COLORQUAD_FROM_RGBA(255, 255, 255, 255));
    a.checkEqual("12", gfx::addColor(COLORQUAD_FROM_RGBA(  1,   1,   1,   1), COLORQUAD_FROM_RGBA(255, 255, 255, 255)), COLORQUAD_FROM_RGBA(255, 255, 255, 255));
    a.checkEqual("13", gfx::addColor(COLORQUAD_FROM_RGBA(255, 255, 255, 255), COLORQUAD_FROM_RGBA(  1,   1,   1,   1)), COLORQUAD_FROM_RGBA(255, 255, 255, 255));
    a.checkEqual("14", gfx::addColor(COLORQUAD_FROM_RGBA(255,   0, 255,   0), COLORQUAD_FROM_RGBA(255, 255, 255, 255)), COLORQUAD_FROM_RGBA(255, 255, 255, 255));
    a.checkEqual("15", gfx::addColor(COLORQUAD_FROM_RGBA(  0, 255,   0, 255), COLORQUAD_FROM_RGBA(255, 255, 255, 255)), COLORQUAD_FROM_RGBA(255, 255, 255, 255));
}

/** Test getColorDistance. */
AFL_TEST("gfx.Types:getColorDistance", a)
{
    // Equality
    a.checkEqual("01. same", gfx::getColorDistance(COLORQUAD_FROM_RGBA(  1,   2,   3,   4), COLORQUAD_FROM_RGBA(  1,  2,  3,  4)), 0);

    // Distances: verify monotonicity for Red component
    int last = 0;
    for (int i = 1; i <= 255; ++i) {
        int now = gfx::getColorDistance(COLORQUAD_FROM_RGBA(0, 0, 0, gfx::OPAQUE_ALPHA), COLORQUAD_FROM_RGBA(i, 0, 0, gfx::OPAQUE_ALPHA));
        a.checkGreaterThan("11. red", now, last);
        a.checkEqual("12. red", now, gfx::getColorDistance(COLORQUAD_FROM_RGBA(i, 0, 0, gfx::OPAQUE_ALPHA), COLORQUAD_FROM_RGBA(0, 0, 0, gfx::OPAQUE_ALPHA)));
        last = now;
    }

    // verify monotonicity for Green component
    last = 0;
    for (int i = 1; i <= 255; ++i) {
        int now = gfx::getColorDistance(COLORQUAD_FROM_RGBA(0, 0, 0, gfx::OPAQUE_ALPHA), COLORQUAD_FROM_RGBA(0, i, 0, gfx::OPAQUE_ALPHA));
        a.checkGreaterThan("21. green", now, last);
        a.checkEqual("22. green", now, gfx::getColorDistance(COLORQUAD_FROM_RGBA(0, i, 0, gfx::OPAQUE_ALPHA), COLORQUAD_FROM_RGBA(0, 0, 0, gfx::OPAQUE_ALPHA)));
        last = now;
    }

    // verify monotonicity for Blue component
    last = 0;
    for (int i = 1; i <= 255; ++i) {
        int now = gfx::getColorDistance(COLORQUAD_FROM_RGBA(0, 0, 0, gfx::OPAQUE_ALPHA), COLORQUAD_FROM_RGBA(0, 0, i, gfx::OPAQUE_ALPHA));
        a.checkGreaterThan("31. blue", now, last);
        a.checkEqual("32. blue", now, gfx::getColorDistance(COLORQUAD_FROM_RGBA(0, 0, i, gfx::OPAQUE_ALPHA), COLORQUAD_FROM_RGBA(0, 0, 0, gfx::OPAQUE_ALPHA)));
        last = now;
    }

    // verify monotonicity for Gray
    last = 0;
    for (int i = 1; i <= 255; ++i) {
        int now = gfx::getColorDistance(COLORQUAD_FROM_RGBA(0, 0, 0, gfx::OPAQUE_ALPHA), COLORQUAD_FROM_RGBA(i, i, i, gfx::OPAQUE_ALPHA));
        a.checkGreaterThan("41. gray", now, last);
        a.checkEqual("42. gray", now, gfx::getColorDistance(COLORQUAD_FROM_RGBA(i, i, i, gfx::OPAQUE_ALPHA), COLORQUAD_FROM_RGBA(0, 0, 0, gfx::OPAQUE_ALPHA)));
        a.checkGreaterThan("43. gray", now, gfx::getColorDistance(COLORQUAD_FROM_RGBA(0, 0, 0, gfx::OPAQUE_ALPHA), COLORQUAD_FROM_RGBA(i, 0, 0, gfx::OPAQUE_ALPHA)));
        a.checkGreaterThan("44. gray", now, gfx::getColorDistance(COLORQUAD_FROM_RGBA(0, 0, 0, gfx::OPAQUE_ALPHA), COLORQUAD_FROM_RGBA(0, i, 0, gfx::OPAQUE_ALPHA)));
        a.checkGreaterThan("45. gray", now, gfx::getColorDistance(COLORQUAD_FROM_RGBA(0, 0, 0, gfx::OPAQUE_ALPHA), COLORQUAD_FROM_RGBA(0, 0, i, gfx::OPAQUE_ALPHA)));
        last = now;
    }

    // last now is the maximum possible distance. Differing alpha must still be higher.
    a.checkGreaterThan("51. alpha", gfx::getColorDistance(COLORQUAD_FROM_RGBA(0, 0, 0, 0), COLORQUAD_FROM_RGBA(10, 10, 10, 10)), last);
}

/*
 *  parseColor()
 */

// ok: #rgb
AFL_TEST("gfx.Types:parseColor:rgb", a)
{
    gfx::ColorQuad_t q;
    util::StringParser p("#234");
    a.check("01. parseColor", gfx::parseColor(p, q));
    a.check("02. parseEnd", p.parseEnd());
    a.checkEqual("03. result", q, COLORQUAD_FROM_RGBA(0x22, 0x33, 0x44, 0xFF));
}

// ok: #rrggbb
AFL_TEST("gfx.Types:parseColor:rrggbb", a)
{
    gfx::ColorQuad_t q;
    util::StringParser p("#124567");
    a.check("01. parseColor", gfx::parseColor(p, q));
    a.check("02. parseEnd", p.parseEnd());
    a.checkEqual("03. result", q, COLORQUAD_FROM_RGBA(0x12, 0x45, 0x67, 0xFF));
}

// ok: #rgba
AFL_TEST("gfx.Types:parseColor:rgba", a)
{
    gfx::ColorQuad_t q;
    util::StringParser p("#234A");
    a.check("01. parseColor", gfx::parseColor(p, q));
    a.check("02. parseEnd", p.parseEnd());
    a.checkEqual("03. result", q, COLORQUAD_FROM_RGBA(0x22, 0x33, 0x44, 0xAA));
}

// ok: #rrggbbaa
AFL_TEST("gfx.Types:parseColor:rrggbbaa", a)
{
    gfx::ColorQuad_t q;
    util::StringParser p("#234A95CD");
    a.check("01. parseColor", gfx::parseColor(p, q));
    a.check("02. parseEnd", p.parseEnd());
    a.checkEqual("03. result", q, COLORQUAD_FROM_RGBA(0x23, 0x4A, 0x95, 0xCD));
}

// ok: rgb(r,g,b)
AFL_TEST("gfx.Types:parseColor:rgb-function", a)
{
    gfx::ColorQuad_t q;
    util::StringParser p("rgb ( 1, 2 , 3 )");
    a.check("01. parseColor", gfx::parseColor(p, q));
    a.check("02. parseEnd", p.parseEnd());
    a.checkEqual("03. result", q, COLORQUAD_FROM_RGBA(0x01, 0x02, 0x03, 0xFF));
}

// ok: rgb(r,g,b,a)
AFL_TEST("gfx.Types:parseColor:rgba-function", a)
{
    gfx::ColorQuad_t q;
    util::StringParser p("rgb(11,22,33,44)");
    a.check("01. parseColor", gfx::parseColor(p, q));
    a.check("02. parseEnd", p.parseEnd());
    a.checkEqual("03. result", q, COLORQUAD_FROM_RGBA(11, 22, 33, 44));
}

// ok: using percent
AFL_TEST("gfx.Types:parseColor:rgb-function:percent", a)
{
    gfx::ColorQuad_t q;
    util::StringParser p("rgb(25 % ,22,33,33%)");
    a.check("01. parseColor", gfx::parseColor(p, q));
    a.check("02. parseEnd", p.parseEnd());
    a.checkEqual("03. result", q, COLORQUAD_FROM_RGBA(0x40, 22, 33, 0x54));
}

// failure: out-of-range value
AFL_TEST("gfx.Types:parseColor:error:rgb-function:range", a)
{
    gfx::ColorQuad_t q;
    util::StringParser p("rgb(1000,200,300)");
    a.check("", !gfx::parseColor(p, q));
}

// failure: out-of-range percentage
AFL_TEST("gfx.Types:parseColor:error:rgb-function:percent-range", a)
{
    gfx::ColorQuad_t q;
    util::StringParser p("rgb(10%,20%,300%)");
    a.check("", !gfx::parseColor(p, q));
}

// failure: too few args
AFL_TEST("gfx.Types:parseColor:error:rgb-function:too-few-parameters", a)
{
    gfx::ColorQuad_t q;
    util::StringParser p("rgb(4,5)");
    a.check("", !gfx::parseColor(p, q));
}

// failure: too many args
AFL_TEST("gfx.Types:parseColor:error:rgb-function:too-many-parameters", a)
{
    gfx::ColorQuad_t q;
    util::StringParser p("rgb(4,5,6,7,8)");
    a.check("", !gfx::parseColor(p, q));
}

// failure: too short
AFL_TEST("gfx.Types:parseColor:error:rgb:too-short", a)
{
    gfx::ColorQuad_t q;
    util::StringParser p("#12");
    a.check("", !gfx::parseColor(p, q));
}

// failure: wrong length
AFL_TEST("gfx.Types:parseColor:error:rgb:bad-length", a)
{
    gfx::ColorQuad_t q;
    util::StringParser p("#12345");
    a.check("", !gfx::parseColor(p, q));
}

// failure: bad keyword
AFL_TEST("gfx.Types:parseColor:error:bad-keyword", a)
{
    gfx::ColorQuad_t q;
    util::StringParser p("lolwut?");
    a.check("", !gfx::parseColor(p, q));
}

// failure: bad hex
AFL_TEST("gfx.Types:parseColor:error:bad-hex", a)
{
    gfx::ColorQuad_t q;
    util::StringParser p("#lolwut");
    a.check("", !gfx::parseColor(p, q));
}

// failure: bad number
AFL_TEST("gfx.Types:parseColor:error:bad-number", a)
{
    gfx::ColorQuad_t q;
    util::StringParser p("rgb(lol,wut,wtf)");
    a.check("", !gfx::parseColor(p, q));
}
