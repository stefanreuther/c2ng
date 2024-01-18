/**
  *  \file test/gfx/gen/colorrangetest.cpp
  *  \brief Test for gfx::gen::ColorRange
  */

#include "gfx/gen/colorrange.hpp"
#include "afl/test/testrunner.hpp"

/** Get ColorRange::get(), maximum precision. */
AFL_TEST("gfx.gen.ColorRange:get", a)
{
    // Positive delta
    gfx::gen::ColorRange ra(COLORQUAD_FROM_RGBA(0,0,0,0), COLORQUAD_FROM_RGBA(0xFF,0xFF,0xFF,0xFF), 256);
    for (int i = 0; i < 256; ++i) {
        a.checkEqual("01", RED_FROM_COLORQUAD(ra.get(i)), i);
        a.checkEqual("02", GREEN_FROM_COLORQUAD(ra.get(i)), i);
        a.checkEqual("03", BLUE_FROM_COLORQUAD(ra.get(i)), i);
        a.checkEqual("04", ALPHA_FROM_COLORQUAD(ra.get(i)), i);
    }

    // Negative delta
    gfx::gen::ColorRange rb(COLORQUAD_FROM_RGBA(0xFF,0xFF,0xFF,0xFF), COLORQUAD_FROM_RGBA(0,0,0,0), 256);
    for (int i = 0; i < 256; ++i) {
        a.checkEqual("11", RED_FROM_COLORQUAD(rb.get(i)), 255-i);
        a.checkEqual("12", GREEN_FROM_COLORQUAD(rb.get(i)), 255-i);
        a.checkEqual("13", BLUE_FROM_COLORQUAD(rb.get(i)), 255-i);
        a.checkEqual("14", ALPHA_FROM_COLORQUAD(rb.get(i)), 255-i);
    }
}

/** Get ColorRange::get(), limited precision. */
AFL_TEST("gfx.gen.ColorRange:get:limit", a)
{
    gfx::gen::ColorRange ra(COLORQUAD_FROM_RGBA(0,0,0,0xFF), COLORQUAD_FROM_RGBA(0xFF,0x80,100,0), 10);

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
            a.checkEqual("", ra.get(i), ex[x].value);
            ++i;
        }
    }
}

/** Test ColorRange::get(), simple case. */
AFL_TEST("gfx.gen.ColorRange:get:simple", a)
{
    gfx::gen::ColorRange testee(COLORQUAD_FROM_RGBA(0x12,0x34,0x45,0x67));
    for (int i = 0; i < 256; ++i) {
        a.checkEqual("", testee.get(i), COLORQUAD_FROM_RGBA(0x12,0x34,0x45,0x67));
    }
}

/** Test ColorRange::get(), one section. */
AFL_TEST("gfx.gen.ColorRange:get:unit", a)
{
    gfx::gen::ColorRange testee(COLORQUAD_FROM_RGBA(0,0,0,0xFF), COLORQUAD_FROM_RGBA(0xFF,0x80,100,0), 1);
    for (int i = 0; i < 256; ++i) {
        a.checkEqual("", testee.get(i), COLORQUAD_FROM_RGBA(0,0,0,0xFF));
    }
}

/** Test ColorRange::parse(). */
AFL_TEST("gfx.gen.ColorRange:parse", a)
{
    // Initialisation
    gfx::gen::ColorRange testee;
    a.checkEqual("01. getStartColor", testee.getStartColor(), COLORQUAD_FROM_RGBA(0,0,0,0));
    a.checkEqual("02. getEndColor",   testee.getEndColor(), COLORQUAD_FROM_RGBA(0,0,0,0));
    a.checkEqual("03. getNumSteps",   testee.getNumSteps(), gfx::gen::ColorRange::MAX_STEPS);

    // Parse
    {
        util::StringParser p("#321608");
        a.checkEqual("11. parse",         testee.parse(p), true);
        a.checkEqual("12. parseEnd",      p.parseEnd(), true);
        a.checkEqual("13. getStartColor", testee.getStartColor(), COLORQUAD_FROM_RGBA(0x32,0x16,8,255));
        a.checkEqual("14. getEndColor",   testee.getEndColor(),   COLORQUAD_FROM_RGBA(0x32,0x16,8,255));
        a.checkEqual("15. getNumSteps",   testee.getNumSteps(),   gfx::gen::ColorRange::MAX_STEPS);
    }

    {
        util::StringParser p("#119-#442/3x");
        a.checkEqual("21. parse",          testee.parse(p), true);
        a.checkEqual("22. parseCharacter", p.parseCharacter('x'), true);
        a.checkEqual("23. parseEnd",       p.parseEnd(), true);
        a.checkEqual("24. getStartColor",  testee.getStartColor(), COLORQUAD_FROM_RGBA(0x11,0x11,0x99,255));
        a.checkEqual("25. getEndColor",    testee.getEndColor(),   COLORQUAD_FROM_RGBA(0x44,0x44,0x22,255));
        a.checkEqual("26. getNumSteps",    testee.getNumSteps(),   3);
    }
}

/** Test ColorRange::parse(), error case. */
AFL_TEST("gfx.gen.ColorRange:parse:error:missing-second-color", a)
{
    gfx::gen::ColorRange testee;
    util::StringParser p("#123-");
    a.checkEqual("01. parse", testee.parse(p), false);
    a.check("02. parseEnd", p.parseEnd());
}

AFL_TEST("gfx.gen.ColorRange:parse:error:bad-character", a)
{
    gfx::gen::ColorRange testee;
    util::StringParser p("*");
    a.checkEqual("03. parse", testee.parse(p), false);
    a.check("04. parseCharacter", p.parseCharacter('*'));
}

AFL_TEST("gfx.gen.ColorRange:parse:error:missing-count", a)
{
    gfx::gen::ColorRange testee;
    util::StringParser p("#123/x");
    a.checkEqual("05. parse", testee.parse(p), false);
    a.check("06. parseCharacter", p.parseCharacter('x'));
}
