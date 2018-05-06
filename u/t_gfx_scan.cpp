/**
  *  \file u/t_gfx_scan.cpp
  *  \brief Test for gfx::Scan
  */

#include "gfx/scan.hpp"

#include "t_gfx.hpp"
#include "gfx/rgbapixmap.hpp"

using afl::base::Ref;
using gfx::RGBAPixmap;
using gfx::Canvas;

/** Test scanning an empty image. */
void
TestGfxScan::testScanEmpty()
{
    Ref<RGBAPixmap> pix = RGBAPixmap::create(5, 5);
    pix->pixels().fill(COLORQUAD_FROM_RGBA(0, 99, 0, gfx::TRANSPARENT_ALPHA));
    Ref<Canvas> can = pix->makeCanvas();

    int y = 0, minX, maxX;
    TS_ASSERT_EQUALS(gfx::scanCanvas(*can, y, minX, maxX), false);
}

/** Test scanning a small (5x5) image that contains some pixels. */
void
TestGfxScan::testScanSmall()
{
    Ref<RGBAPixmap> pix = RGBAPixmap::create(5, 5);
    pix->pixels().fill(COLORQUAD_FROM_RGBA(0, 99, 0, gfx::TRANSPARENT_ALPHA));
    *pix->row(2).at(2) = COLORQUAD_FROM_RGBA(1,2,3,4);
    Ref<Canvas> can = pix->makeCanvas();

    int y = 0, minX, maxX;
    TS_ASSERT_EQUALS(gfx::scanCanvas(*can, y, minX, maxX), true);
    TS_ASSERT_EQUALS(y, 2);
    TS_ASSERT_EQUALS(minX, 2);
    TS_ASSERT_EQUALS(maxX, 3);
}

/** Test scanning a large (200x200) image that contains some pixels. */
void
TestGfxScan::testScanLarge()
{
    Ref<RGBAPixmap> pix = RGBAPixmap::create(200, 200);
    pix->pixels().fill(COLORQUAD_FROM_RGBA(0, 99, 0, gfx::TRANSPARENT_ALPHA));
    pix->row(90).subrange(102, 10).fill(COLORQUAD_FROM_RGBA(1,2,3,4));
    Ref<Canvas> can = pix->makeCanvas();

    int y = 0, minX, maxX;
    TS_ASSERT_EQUALS(gfx::scanCanvas(*can, y, minX, maxX), true);
    TS_ASSERT_EQUALS(y, 90);
    TS_ASSERT_EQUALS(minX, 102);
    TS_ASSERT_EQUALS(maxX, 112);
}

/** Test scanning a huge (3000x200) image that contains some pixels. */
void
TestGfxScan::testScanHuge()
{
    Ref<RGBAPixmap> pix = RGBAPixmap::create(3000, 200);
    pix->pixels().fill(COLORQUAD_FROM_RGBA(0, 99, 0, gfx::TRANSPARENT_ALPHA));
    pix->row(70).subrange(1200, 10).fill(COLORQUAD_FROM_RGBA(1,2,3,4));
    pix->row(70).subrange(2500, 10).fill(COLORQUAD_FROM_RGBA(1,2,3,4));
    Ref<Canvas> can = pix->makeCanvas();

    int y = 0, minX, maxX;
    TS_ASSERT_EQUALS(gfx::scanCanvas(*can, y, minX, maxX), true);
    TS_ASSERT_EQUALS(y, 70);
    TS_ASSERT_EQUALS(minX, 1200);
    TS_ASSERT_EQUALS(maxX, 2510);
}
