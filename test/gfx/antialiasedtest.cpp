/**
  *  \file test/gfx/antialiasedtest.cpp
  *  \brief Test for gfx::Antialiased
  */

#include "gfx/antialiased.hpp"

#include "afl/test/testrunner.hpp"
#include "gfx/basecontext.hpp"
#include "gfx/rgbapixmap.hpp"
#include <stdio.h>

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

AFL_TEST("gfx.Antialiased:drawLineAA", a)
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
    a.checkEqual("01", getPixmapRow(*pix,  0), ".#..........");
    a.checkEqual("02", getPixmapRow(*pix,  1), ".##########.");
    a.checkEqual("03", getPixmapRow(*pix,  2), ".###b.......");
    a.checkEqual("04", getPixmapRow(*pix,  3), ".####hc.....");
    a.checkEqual("05", getPixmapRow(*pix,  4), ".#b###gie...");
    a.checkEqual("06", getPixmapRow(*pix,  5), ".#.h###af#g.");
    a.checkEqual("07", getPixmapRow(*pix,  6), ".#.cg###..d.");
    a.checkEqual("08", getPixmapRow(*pix,  7), ".#..ia###...");
    a.checkEqual("09", getPixmapRow(*pix,  8), ".#..ef.###..");
    a.checkEqual("10", getPixmapRow(*pix,  9), ".#...#..###.");
    a.checkEqual("11", getPixmapRow(*pix, 10), ".#...gd..##.");
    a.checkEqual("12", getPixmapRow(*pix, 11), "..........#.");
}

AFL_TEST("gfx.Antialiased:drawCircleAA", a)
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
    a.checkEqual("01", getPixmapRow(*pix,  0), "............");
    a.checkEqual("02", getPixmapRow(*pix,  1), "...gi#ig....");
    a.checkEqual("03", getPixmapRow(*pix,  2), "..jfa.afj...");
    a.checkEqual("04", getPixmapRow(*pix,  3), ".gf.....fg..");
    a.checkEqual("05", getPixmapRow(*pix,  4), ".ia.....ai..");
    a.checkEqual("06", getPixmapRow(*pix,  5), ".#.......#..");
    a.checkEqual("07", getPixmapRow(*pix,  6), ".ia.....ai..");
    a.checkEqual("08", getPixmapRow(*pix,  7), ".gf.....fg..");
    a.checkEqual("09", getPixmapRow(*pix,  8), "..jfa.afj...");
    a.checkEqual("10", getPixmapRow(*pix,  9), "...gi#ig....");
    a.checkEqual("11", getPixmapRow(*pix, 10), "............");
    a.checkEqual("12", getPixmapRow(*pix, 11), "............");
}
