/**
  *  \file test/gfx/scantest.cpp
  *  \brief Test for gfx::Scan
  */

#include "gfx/scan.hpp"

#include "afl/test/testrunner.hpp"
#include "gfx/rgbapixmap.hpp"

using afl::base::Ref;
using gfx::RGBAPixmap;
using gfx::Canvas;

/** Test scanning an empty image. */
AFL_TEST("gfx.Scan:scanCanvas:empty", a)
{
    Ref<RGBAPixmap> pix = RGBAPixmap::create(5, 5);
    pix->pixels().fill(COLORQUAD_FROM_RGBA(0, 99, 0, gfx::TRANSPARENT_ALPHA));
    Ref<Canvas> can = pix->makeCanvas();

    int y = 0, minX, maxX;
    a.checkEqual("", gfx::scanCanvas(*can, y, minX, maxX), false);
}

/** Test scanning a small (5x5) image that contains some pixels. */
AFL_TEST("gfx.Scan:scanCanvas:small", a)
{
    Ref<RGBAPixmap> pix = RGBAPixmap::create(5, 5);
    pix->pixels().fill(COLORQUAD_FROM_RGBA(0, 99, 0, gfx::TRANSPARENT_ALPHA));
    *pix->row(2).at(2) = COLORQUAD_FROM_RGBA(1,2,3,4);
    Ref<Canvas> can = pix->makeCanvas();

    int y = 0, minX, maxX;
    a.checkEqual("01. scanCanvas", gfx::scanCanvas(*can, y, minX, maxX), true);
    a.checkEqual("02. y", y, 2);
    a.checkEqual("03. minX", minX, 2);
    a.checkEqual("04. maxX", maxX, 3);
}

/** Test scanning a large (200x200) image that contains some pixels. */
AFL_TEST("gfx.Scan:scanCanvas:large", a)
{
    Ref<RGBAPixmap> pix = RGBAPixmap::create(200, 200);
    pix->pixels().fill(COLORQUAD_FROM_RGBA(0, 99, 0, gfx::TRANSPARENT_ALPHA));
    pix->row(90).subrange(102, 10).fill(COLORQUAD_FROM_RGBA(1,2,3,4));
    Ref<Canvas> can = pix->makeCanvas();

    int y = 0, minX, maxX;
    a.checkEqual("01. scanCanvas", gfx::scanCanvas(*can, y, minX, maxX), true);
    a.checkEqual("02. y", y, 90);
    a.checkEqual("03. minX", minX, 102);
    a.checkEqual("04. maxX", maxX, 112);
}

/** Test scanning a huge (3000x200) image that contains some pixels. */
AFL_TEST("gfx.Scan:scanCanvas:huge", a)
{
    Ref<RGBAPixmap> pix = RGBAPixmap::create(3000, 200);
    pix->pixels().fill(COLORQUAD_FROM_RGBA(0, 99, 0, gfx::TRANSPARENT_ALPHA));
    pix->row(70).subrange(1200, 10).fill(COLORQUAD_FROM_RGBA(1,2,3,4));
    pix->row(70).subrange(2500, 10).fill(COLORQUAD_FROM_RGBA(1,2,3,4));
    Ref<Canvas> can = pix->makeCanvas();

    int y = 0, minX, maxX;
    a.checkEqual("01. scanCanvas", gfx::scanCanvas(*can, y, minX, maxX), true);
    a.checkEqual("02. y", y, 70);
    a.checkEqual("03. minX", minX, 1200);
    a.checkEqual("04. maxX", maxX, 2510);
}
