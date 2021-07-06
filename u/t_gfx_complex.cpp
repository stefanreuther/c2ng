/**
  *  \file u/t_gfx_complex.cpp
  *  \brief Test for gfx::Complex
  */

#include "gfx/complex.hpp"

#include "t_gfx.hpp"
#include "afl/base/ref.hpp"
#include "gfx/basecontext.hpp"
#include "gfx/canvas.hpp"
#include "gfx/palettizedpixmap.hpp"

using afl::base::Ref;
using afl::string::fromBytes;
using gfx::BaseContext;
using gfx::Canvas;
using gfx::PalettizedPixmap;
using gfx::Point;

/** Test drawFilledPolygon: triangle. */
void
TestGfxComplex::testFillPolyTriangle1()
{
    Ref<PalettizedPixmap> pix = PalettizedPixmap::create(25, 25);
    Ref<Canvas> can = pix->makeCanvas();
    BaseContext ctx(*can);
    ctx.setRawColor('#');
    pix->pixels().fill('.');

    Point pts[] = { Point(5, 5), Point(20, 5), Point(20, 20) };
    drawFilledPolygon(ctx, pts);

    TS_ASSERT_EQUALS(fromBytes(pix->row( 4)), ".........................");
    TS_ASSERT_EQUALS(fromBytes(pix->row( 5)), ".....###############.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row( 6)), "......##############.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row( 7)), ".......#############.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row( 8)), "........############.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row( 9)), ".........###########.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(10)), "..........##########.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(11)), "...........#########.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(12)), "............########.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(13)), ".............#######.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(14)), "..............######.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(15)), "...............#####.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(16)), "................####.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(17)), ".................###.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(18)), "..................##.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(19)), "...................#.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(20)), ".........................");
}

/** Test drawFilledPolygon: triangle, other point order. */
void
TestGfxComplex::testFillPolyTriangle2()
{
    Ref<PalettizedPixmap> pix = PalettizedPixmap::create(25, 25);
    Ref<Canvas> can = pix->makeCanvas();
    BaseContext ctx(*can);
    ctx.setRawColor('#');
    pix->pixels().fill('.');

    Point pts[] = { Point(20, 5), Point(5, 5), Point(20, 20) };
    drawFilledPolygon(ctx, pts);

    TS_ASSERT_EQUALS(fromBytes(pix->row( 4)), ".........................");
    TS_ASSERT_EQUALS(fromBytes(pix->row( 5)), ".....###############.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row( 6)), "......##############.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row( 7)), ".......#############.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row( 8)), "........############.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row( 9)), ".........###########.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(10)), "..........##########.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(11)), "...........#########.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(12)), "............########.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(13)), ".............#######.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(14)), "..............######.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(15)), "...............#####.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(16)), "................####.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(17)), ".................###.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(18)), "..................##.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(19)), "...................#.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(20)), ".........................");
}

/** Test drawFilledPolygon: triangle, with pattern. */
void
TestGfxComplex::testFillPolyTrianglePattern()
{
    Ref<PalettizedPixmap> pix = PalettizedPixmap::create(25, 25);
    Ref<Canvas> can = pix->makeCanvas();
    BaseContext ctx(*can);
    ctx.setRawColor('#');
    ctx.setFillPattern(gfx::FillPattern::GRAY25);
    pix->pixels().fill('.');

    Point pts[] = { Point(5, 5), Point(20, 5), Point(20, 20) };
    drawFilledPolygon(ctx, pts);

    TS_ASSERT_EQUALS(fromBytes(pix->row( 4)), ".........................");
    TS_ASSERT_EQUALS(fromBytes(pix->row( 5)), ".........................");
    TS_ASSERT_EQUALS(fromBytes(pix->row( 6)), ".......#.#.#.#.#.#.#.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row( 7)), ".........................");
    TS_ASSERT_EQUALS(fromBytes(pix->row( 8)), "........#.#.#.#.#.#......");
    TS_ASSERT_EQUALS(fromBytes(pix->row( 9)), ".........................");
    TS_ASSERT_EQUALS(fromBytes(pix->row(10)), "...........#.#.#.#.#.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(11)), ".........................");
    TS_ASSERT_EQUALS(fromBytes(pix->row(12)), "............#.#.#.#......");
    TS_ASSERT_EQUALS(fromBytes(pix->row(13)), ".........................");
    TS_ASSERT_EQUALS(fromBytes(pix->row(14)), "...............#.#.#.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(15)), ".........................");
    TS_ASSERT_EQUALS(fromBytes(pix->row(16)), "................#.#......");
    TS_ASSERT_EQUALS(fromBytes(pix->row(17)), ".........................");
    TS_ASSERT_EQUALS(fromBytes(pix->row(18)), "...................#.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(19)), ".........................");
    TS_ASSERT_EQUALS(fromBytes(pix->row(20)), ".........................");
}

/** Test drawFilledPolygon: pentagram (classic self-intersecting). */
void
TestGfxComplex::testFillPolyPentagram()
{
    Ref<PalettizedPixmap> pix = PalettizedPixmap::create(40, 40);
    Ref<Canvas> can = pix->makeCanvas();
    BaseContext ctx(*can);
    ctx.setRawColor('#');
    pix->pixels().fill('.');

    Point pts[] = { Point(20, 5), Point(30, 30), Point(5, 12), Point(35, 12), Point(10, 30) };
    drawFilledPolygon(ctx, pts);

    TS_ASSERT_EQUALS(fromBytes(pix->row( 5)), "........................................");
    TS_ASSERT_EQUALS(fromBytes(pix->row( 6)), "........................................");
    TS_ASSERT_EQUALS(fromBytes(pix->row( 7)), "...................##...................");
    TS_ASSERT_EQUALS(fromBytes(pix->row( 8)), "...................##...................");
    TS_ASSERT_EQUALS(fromBytes(pix->row( 9)), "..................####..................");
    TS_ASSERT_EQUALS(fromBytes(pix->row(10)), "..................####..................");
    TS_ASSERT_EQUALS(fromBytes(pix->row(11)), "..................####..................");
    TS_ASSERT_EQUALS(fromBytes(pix->row(12)), ".....############......############.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(13)), "......###########......###########......");
    TS_ASSERT_EQUALS(fromBytes(pix->row(14)), "........########........########........");
    TS_ASSERT_EQUALS(fromBytes(pix->row(15)), ".........#######........#######.........");
    TS_ASSERT_EQUALS(fromBytes(pix->row(16)), "...........#####........#####...........");
    TS_ASSERT_EQUALS(fromBytes(pix->row(17)), "............###..........###............");
    TS_ASSERT_EQUALS(fromBytes(pix->row(18)), ".............##..........##.............");
    TS_ASSERT_EQUALS(fromBytes(pix->row(19)), "..............#..........#..............");
    TS_ASSERT_EQUALS(fromBytes(pix->row(20)), "..............##........##..............");
    TS_ASSERT_EQUALS(fromBytes(pix->row(21)), "..............####.....###..............");
    TS_ASSERT_EQUALS(fromBytes(pix->row(22)), ".............######..######.............");
    TS_ASSERT_EQUALS(fromBytes(pix->row(23)), ".............##############.............");
    TS_ASSERT_EQUALS(fromBytes(pix->row(24)), "............######....######............");
    TS_ASSERT_EQUALS(fromBytes(pix->row(25)), "............#####......#####............");
    TS_ASSERT_EQUALS(fromBytes(pix->row(26)), "............####........####............");
    TS_ASSERT_EQUALS(fromBytes(pix->row(27)), "...........###............###...........");
    TS_ASSERT_EQUALS(fromBytes(pix->row(28)), "...........##..............##...........");
    TS_ASSERT_EQUALS(fromBytes(pix->row(29)), "..........#..................#..........");
    TS_ASSERT_EQUALS(fromBytes(pix->row(30)), "........................................");

}

/** Test drawFilledPolygon: some random polygon. */
void
TestGfxComplex::testFillPolyPolygon()
{
    Ref<PalettizedPixmap> pix = PalettizedPixmap::create(25, 45);
    Ref<Canvas> can = pix->makeCanvas();
    BaseContext ctx(*can);
    ctx.setRawColor('#');
    pix->pixels().fill('.');

    Point pts[] = { Point(9, 6), Point(21, 12), Point(15, 27), Point(18, 36), Point(6, 42), Point(3, 18) };
    drawFilledPolygon(ctx, pts);

    TS_ASSERT_EQUALS(fromBytes(pix->row( 6)), ".........................");
    TS_ASSERT_EQUALS(fromBytes(pix->row( 7)), ".........##..............");
    TS_ASSERT_EQUALS(fromBytes(pix->row( 8)), "........#####............");
    TS_ASSERT_EQUALS(fromBytes(pix->row( 9)), "........#######..........");
    TS_ASSERT_EQUALS(fromBytes(pix->row(10)), ".......##########........");
    TS_ASSERT_EQUALS(fromBytes(pix->row(11)), ".......############......");
    TS_ASSERT_EQUALS(fromBytes(pix->row(12)), "......###############....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(13)), "......###############....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(14)), ".....###############.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(15)), ".....###############.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(16)), "....###############......");
    TS_ASSERT_EQUALS(fromBytes(pix->row(17)), "....###############......");
    TS_ASSERT_EQUALS(fromBytes(pix->row(18)), "...################......");
    TS_ASSERT_EQUALS(fromBytes(pix->row(19)), "...###############.......");
    TS_ASSERT_EQUALS(fromBytes(pix->row(20)), "...###############.......");
    TS_ASSERT_EQUALS(fromBytes(pix->row(21)), "...##############........");
    TS_ASSERT_EQUALS(fromBytes(pix->row(22)), "....#############........");
    TS_ASSERT_EQUALS(fromBytes(pix->row(23)), "....#############........");
    TS_ASSERT_EQUALS(fromBytes(pix->row(24)), "....############.........");
    TS_ASSERT_EQUALS(fromBytes(pix->row(25)), "....############.........");
    TS_ASSERT_EQUALS(fromBytes(pix->row(26)), "....###########..........");
    TS_ASSERT_EQUALS(fromBytes(pix->row(27)), "....###########..........");
    TS_ASSERT_EQUALS(fromBytes(pix->row(28)), "....###########..........");
    TS_ASSERT_EQUALS(fromBytes(pix->row(29)), "....############.........");
    TS_ASSERT_EQUALS(fromBytes(pix->row(30)), ".....###########.........");
    TS_ASSERT_EQUALS(fromBytes(pix->row(31)), ".....###########.........");
    TS_ASSERT_EQUALS(fromBytes(pix->row(32)), ".....############........");
    TS_ASSERT_EQUALS(fromBytes(pix->row(33)), ".....############........");
    TS_ASSERT_EQUALS(fromBytes(pix->row(34)), ".....############........");
    TS_ASSERT_EQUALS(fromBytes(pix->row(35)), ".....#############.......");
    TS_ASSERT_EQUALS(fromBytes(pix->row(36)), ".....#############.......");
    TS_ASSERT_EQUALS(fromBytes(pix->row(37)), ".....###########.........");
    TS_ASSERT_EQUALS(fromBytes(pix->row(38)), "......########...........");
    TS_ASSERT_EQUALS(fromBytes(pix->row(39)), "......######.............");
    TS_ASSERT_EQUALS(fromBytes(pix->row(40)), "......####...............");
    TS_ASSERT_EQUALS(fromBytes(pix->row(41)), "......##.................");
    TS_ASSERT_EQUALS(fromBytes(pix->row(42)), ".........................");
}

/** Test drawFilledPolygon: a square. */
void
TestGfxComplex::testFillPolySquare()
{
    Ref<PalettizedPixmap> pix = PalettizedPixmap::create(25, 25);
    Ref<Canvas> can = pix->makeCanvas();
    BaseContext ctx(*can);
    ctx.setRawColor('#');
    pix->pixels().fill('.');

    Point pts[] = { Point(5, 5), Point(20, 5), Point(20, 20), Point(5, 20) };
    drawFilledPolygon(ctx, pts);

    TS_ASSERT_EQUALS(fromBytes(pix->row( 4)), ".........................");
    TS_ASSERT_EQUALS(fromBytes(pix->row( 5)), ".....###############.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row( 6)), ".....###############.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row( 7)), ".....###############.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row( 8)), ".....###############.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row( 9)), ".....###############.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(10)), ".....###############.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(11)), ".....###############.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(12)), ".....###############.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(13)), ".....###############.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(14)), ".....###############.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(15)), ".....###############.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(16)), ".....###############.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(17)), ".....###############.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(18)), ".....###############.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(19)), ".....###############.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(20)), ".........................");
}

/** Test drawFilledPolygon: a rhombe. */
void
TestGfxComplex::testFillPolyRhombe()
{
    Ref<PalettizedPixmap> pix = PalettizedPixmap::create(45, 45);
    Ref<Canvas> can = pix->makeCanvas();
    BaseContext ctx(*can);
    ctx.setRawColor('#');
    pix->pixels().fill('.');

    Point pts[] = { Point(20, 5), Point(40, 20), Point(20, 40), Point(5, 20) };
    drawFilledPolygon(ctx, pts);

    TS_ASSERT_EQUALS(fromBytes(pix->row( 5)), ".............................................");
    TS_ASSERT_EQUALS(fromBytes(pix->row( 6)), "...................##........................");
    TS_ASSERT_EQUALS(fromBytes(pix->row( 7)), "..................#####......................");
    TS_ASSERT_EQUALS(fromBytes(pix->row( 8)), ".................#######.....................");
    TS_ASSERT_EQUALS(fromBytes(pix->row( 9)), "................#########....................");
    TS_ASSERT_EQUALS(fromBytes(pix->row(10)), "...............############..................");
    TS_ASSERT_EQUALS(fromBytes(pix->row(11)), "..............##############.................");
    TS_ASSERT_EQUALS(fromBytes(pix->row(12)), ".............################................");
    TS_ASSERT_EQUALS(fromBytes(pix->row(13)), "............###################..............");
    TS_ASSERT_EQUALS(fromBytes(pix->row(14)), "...........#####################.............");
    TS_ASSERT_EQUALS(fromBytes(pix->row(15)), "..........#######################............");
    TS_ASSERT_EQUALS(fromBytes(pix->row(16)), ".........##########################..........");
    TS_ASSERT_EQUALS(fromBytes(pix->row(17)), "........############################.........");
    TS_ASSERT_EQUALS(fromBytes(pix->row(18)), ".......##############################........");
    TS_ASSERT_EQUALS(fromBytes(pix->row(19)), "......#################################......");
    TS_ASSERT_EQUALS(fromBytes(pix->row(20)), ".....###################################.....");
    TS_ASSERT_EQUALS(fromBytes(pix->row(21)), "......#################################......");
    TS_ASSERT_EQUALS(fromBytes(pix->row(22)), ".......###############################.......");
    TS_ASSERT_EQUALS(fromBytes(pix->row(23)), ".......##############################........");
    TS_ASSERT_EQUALS(fromBytes(pix->row(24)), "........############################.........");
    TS_ASSERT_EQUALS(fromBytes(pix->row(25)), ".........##########################..........");
    TS_ASSERT_EQUALS(fromBytes(pix->row(26)), "..........########################...........");
    TS_ASSERT_EQUALS(fromBytes(pix->row(27)), "..........#######################............");
    TS_ASSERT_EQUALS(fromBytes(pix->row(28)), "...........#####################.............");
    TS_ASSERT_EQUALS(fromBytes(pix->row(29)), "............###################..............");
    TS_ASSERT_EQUALS(fromBytes(pix->row(30)), ".............#################...............");
    TS_ASSERT_EQUALS(fromBytes(pix->row(31)), ".............################................");
    TS_ASSERT_EQUALS(fromBytes(pix->row(32)), "..............##############.................");
    TS_ASSERT_EQUALS(fromBytes(pix->row(33)), "...............############..................");
    TS_ASSERT_EQUALS(fromBytes(pix->row(34)), "................##########...................");
    TS_ASSERT_EQUALS(fromBytes(pix->row(35)), "................#########....................");
    TS_ASSERT_EQUALS(fromBytes(pix->row(36)), ".................#######.....................");
    TS_ASSERT_EQUALS(fromBytes(pix->row(37)), "..................#####......................");
    TS_ASSERT_EQUALS(fromBytes(pix->row(38)), "...................###.......................");
    TS_ASSERT_EQUALS(fromBytes(pix->row(39)), "...................##........................");
    TS_ASSERT_EQUALS(fromBytes(pix->row(40)), ".............................................");
}

