/**
  *  \file test/gfx/nullcanvastest.cpp
  *  \brief Test for gfx::NullCanvas
  */

#include "gfx/nullcanvas.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test. */
AFL_TEST("gfx.NullCanvas", a)
{
    // NullCanvas does not do anything.
    // But the object must be creatible.
    gfx::NullCanvas testee;

    // Methods must operate without crashing
    static const gfx::Color_t colors[] = {1,5,9};
    AFL_CHECK_SUCCEEDS(a("01. drawHLine"),  testee.drawHLine(gfx::Point(1, 1), 5, 0, 0xFF, gfx::OPAQUE_ALPHA));
    AFL_CHECK_SUCCEEDS(a("02. drawVLine"),  testee.drawVLine(gfx::Point(1, 1), 5, 0, 0xFF, gfx::OPAQUE_ALPHA));
    AFL_CHECK_SUCCEEDS(a("04. drawPixels"), testee.drawPixels(gfx::Point(9, 2), colors, gfx::OPAQUE_ALPHA));
    AFL_CHECK_SUCCEEDS(a("05. drawBar"),    testee.drawBar(gfx::Rectangle(1,2,3,4), 0x99, 0x77, gfx::FillPattern::SOLID, gfx::OPAQUE_ALPHA));
    AFL_CHECK_SUCCEEDS(a("06. blit"),       testee.blit(gfx::Point(9, 9), testee, gfx::Rectangle(0, 0, 77, 77)));

    // Clipping
    a.check("11. computeClipRect", !testee.computeClipRect(gfx::Rectangle(3,4,5,6)).exists());

    // Color reading
    gfx::Color_t readColors[3] = {1,2,3};
    AFL_CHECK_SUCCEEDS(a("21. getPixels"), testee.getPixels(gfx::Point(8,9), readColors));
    a.checkEqual("22. result", readColors[0], 0U);
    a.checkEqual("23. result", readColors[1], 0U);
    a.checkEqual("24. result", readColors[2], 0U);

    // Inquiry
    a.checkEqual("31. getSize",         testee.getSize(), gfx::Point(1, 1));
    a.checkEqual("32. getBitsPerPixel", testee.getBitsPerPixel(), 1);
    a.check("33. isVisible",           !testee.isVisible(gfx::Rectangle(0, 0, 1, 1)));
    a.check("34. isClipped",            testee.isClipped(gfx::Rectangle(0, 0, 1, 1)));

    // Palette
    static const gfx::ColorQuad_t quadsIn[2] = { COLORQUAD_FROM_RGBA(1,2,3,4), COLORQUAD_FROM_RGBA(9,8,7,6) };
    {
        gfx::Color_t colorsOut[2] = {1,1};
        AFL_CHECK_SUCCEEDS(a("41. setPalette"), testee.setPalette(33, quadsIn, colorsOut));
        a.checkEqual("42. result", colorsOut[0], 33U);
        a.checkEqual("43. result", colorsOut[1], 34U);
    }
    {
        gfx::Color_t colorsOut[2] = {1,1};
        AFL_CHECK_SUCCEEDS(a("44. encodeColors"), testee.encodeColors(quadsIn, colorsOut));
        a.checkEqual("45. result", colorsOut[0], 0U);
        a.checkEqual("46. result", colorsOut[1], 0U);
    }
    {
        gfx::ColorQuad_t quadsOut[3];
        AFL_CHECK_SUCCEEDS(a("47. decodeColors"), testee.decodeColors(readColors, quadsOut));
        a.checkEqual("48. result", quadsOut[0], 0U);
        a.checkEqual("49. result", quadsOut[1], 0U);
    }

    // Conversion
    afl::base::Ref<gfx::Canvas> can = *new gfx::NullCanvas();
    afl::base::Ref<gfx::Canvas> can2 = testee.convertCanvas(can);
    a.checkEqual("51. convertCanvas", &*can, &*can2);
}
