/**
  *  \file u/t_gfx_antialiased.cpp
  *  \brief Test for gfx::Antialiased
  */

#include <stdio.h>
#include "gfx/antialiased.hpp"

#include "t_gfx.hpp"
#include "gfx/basecontext.hpp"
#include "gfx/rgbapixmap.hpp"

namespace {
    String_t getPixmapRow(const gfx::RGBAPixmap& pix, int y)
    {
        afl::base::Memory<const gfx::ColorQuad_t> row = pix.row(y);
        String_t result;
        while (const gfx::ColorQuad_t* q = row.eat()) {
            switch (*q) {
             case COLORQUAD_FROM_RGB(0,0,0):
                result += '.';
                break;
             case COLORQUAD_FROM_RGB(255,255,255):
                result += '#';
                break;
             case COLORQUAD_FROM_RGB(28,28,28):
                result += 'a';
                break;
             case COLORQUAD_FROM_RGB(57,57,57):
                result += 'b';
                break;
             case COLORQUAD_FROM_RGB(85,85,85):
                result += 'c';
                break;
             case COLORQUAD_FROM_RGB(113,113,113):
                result += 'd';
                break;
             case COLORQUAD_FROM_RGB(114,114,114):
                result += 'e';
                break;
             case COLORQUAD_FROM_RGB(141,141,141):
                result += 'f';
                break;
             case COLORQUAD_FROM_RGB(142,142,142):
                result += 'g';
                break;
             case COLORQUAD_FROM_RGB(153,153,153):
                result += 'f';
                break;
             case COLORQUAD_FROM_RGB(170,170,170):
                result += 'g';
                break;
             case COLORQUAD_FROM_RGB(199,199,199):
                result += 'h';
                break;
             case COLORQUAD_FROM_RGB(227,227,227):
                result += 'i';
                break;
             case COLORQUAD_FROM_RGB(234,234,234):
                result += 'j';
                break;
             default:
                printf("%06X\n", *q);
                result += '?';
                break;
            }
        }
        return result;
    }
}

void
TestGfxAntialiased::testLine()
{
    // Prepare
    afl::base::Ref<gfx::RGBAPixmap> pix(gfx::RGBAPixmap::create(12, 12));
    afl::base::Ref<gfx::Canvas> can(pix->makeCanvas());
    pix->pixels().fill(COLORQUAD_FROM_RGB(0, 0, 0));
    gfx::BaseContext ctx(*can);
    ctx.setRawColor(COLORQUAD_FROM_RGB(255, 255, 255));

    // Horizontal/vertical
    drawLineAA(ctx, gfx::Point(1, 1), gfx::Point(10, 1));
    drawLineAA(ctx, gfx::Point(1, 1), gfx::Point(1, 10));

    // Diagonal (those are anti-aliased)
    drawLineAA(ctx, gfx::Point(1, 1), gfx::Point(10, 5));
    drawLineAA(ctx, gfx::Point(1, 1), gfx::Point(5, 10));

    // Diagonal, thick (not anti-aliased for now)
    ctx.setLineThickness(3);
    drawLineAA(ctx, gfx::Point(1, 1), gfx::Point(10, 10));

    // Verify
    TS_ASSERT_EQUALS(getPixmapRow(*pix,  0), ".#..........");
    TS_ASSERT_EQUALS(getPixmapRow(*pix,  1), ".##########.");
    TS_ASSERT_EQUALS(getPixmapRow(*pix,  2), ".###b.......");
    TS_ASSERT_EQUALS(getPixmapRow(*pix,  3), ".####hc.....");
    TS_ASSERT_EQUALS(getPixmapRow(*pix,  4), ".#b###gie...");
    TS_ASSERT_EQUALS(getPixmapRow(*pix,  5), ".#.h###af#g.");
    TS_ASSERT_EQUALS(getPixmapRow(*pix,  6), ".#.cg###..d.");
    TS_ASSERT_EQUALS(getPixmapRow(*pix,  7), ".#..ia###...");
    TS_ASSERT_EQUALS(getPixmapRow(*pix,  8), ".#..ef.###..");
    TS_ASSERT_EQUALS(getPixmapRow(*pix,  9), ".#...#..###.");
    TS_ASSERT_EQUALS(getPixmapRow(*pix, 10), ".#...gd..##.");
    TS_ASSERT_EQUALS(getPixmapRow(*pix, 11), "..........#.");
}

void
TestGfxAntialiased::testCircle()
{
    // Prepare
    afl::base::Ref<gfx::RGBAPixmap> pix(gfx::RGBAPixmap::create(12, 12));
    afl::base::Ref<gfx::Canvas> can(pix->makeCanvas());
    pix->pixels().fill(COLORQUAD_FROM_RGB(0, 0, 0));
    gfx::BaseContext ctx(*can);
    ctx.setRawColor(COLORQUAD_FROM_RGB(255, 255, 255));

    // Draw
    drawCircleAA(ctx, gfx::Point(5, 5), 4);

    // Verify
    TS_ASSERT_EQUALS(getPixmapRow(*pix,  0), "............");
    TS_ASSERT_EQUALS(getPixmapRow(*pix,  1), "...gi#ig....");
    TS_ASSERT_EQUALS(getPixmapRow(*pix,  2), "..jfa.afj...");
    TS_ASSERT_EQUALS(getPixmapRow(*pix,  3), ".gf.....fg..");
    TS_ASSERT_EQUALS(getPixmapRow(*pix,  4), ".ia.....ai..");
    TS_ASSERT_EQUALS(getPixmapRow(*pix,  5), ".#.......#..");
    TS_ASSERT_EQUALS(getPixmapRow(*pix,  6), ".ia.....ai..");
    TS_ASSERT_EQUALS(getPixmapRow(*pix,  7), ".gf.....fg..");
    TS_ASSERT_EQUALS(getPixmapRow(*pix,  8), "..jfa.afj...");
    TS_ASSERT_EQUALS(getPixmapRow(*pix,  9), "...gi#ig....");
    TS_ASSERT_EQUALS(getPixmapRow(*pix, 10), "............");
    TS_ASSERT_EQUALS(getPixmapRow(*pix, 11), "............");
}

