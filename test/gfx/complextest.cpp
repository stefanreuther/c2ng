/**
  *  \file test/gfx/complextest.cpp
  *  \brief Test for gfx::Complex
  */

#include "gfx/complex.hpp"

#include "afl/base/ref.hpp"
#include "afl/test/testrunner.hpp"
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
AFL_TEST("gfx.Complex:drawFilledPolygon:triangle", a)
{
    Ref<PalettizedPixmap> pix = PalettizedPixmap::create(25, 25);
    Ref<Canvas> can = pix->makeCanvas();
    BaseContext ctx(*can);
    ctx.setRawColor('#');
    pix->pixels().fill('.');

    Point pts[] = { Point(5, 5), Point(20, 5), Point(20, 20) };
    drawFilledPolygon(ctx, pts);

    a.checkEqual("01", fromBytes(pix->row( 4)), ".........................");
    a.checkEqual("02", fromBytes(pix->row( 5)), ".....###############.....");
    a.checkEqual("03", fromBytes(pix->row( 6)), "......##############.....");
    a.checkEqual("04", fromBytes(pix->row( 7)), ".......#############.....");
    a.checkEqual("05", fromBytes(pix->row( 8)), "........############.....");
    a.checkEqual("06", fromBytes(pix->row( 9)), ".........###########.....");
    a.checkEqual("07", fromBytes(pix->row(10)), "..........##########.....");
    a.checkEqual("08", fromBytes(pix->row(11)), "...........#########.....");
    a.checkEqual("09", fromBytes(pix->row(12)), "............########.....");
    a.checkEqual("10", fromBytes(pix->row(13)), ".............#######.....");
    a.checkEqual("11", fromBytes(pix->row(14)), "..............######.....");
    a.checkEqual("12", fromBytes(pix->row(15)), "...............#####.....");
    a.checkEqual("13", fromBytes(pix->row(16)), "................####.....");
    a.checkEqual("14", fromBytes(pix->row(17)), ".................###.....");
    a.checkEqual("15", fromBytes(pix->row(18)), "..................##.....");
    a.checkEqual("16", fromBytes(pix->row(19)), "...................#.....");
    a.checkEqual("17", fromBytes(pix->row(20)), ".........................");
}

/** Test drawFilledPolygon: triangle, other point order. */
AFL_TEST("gfx.Complex:drawFilledPolygon:triangle2", a)
{
    Ref<PalettizedPixmap> pix = PalettizedPixmap::create(25, 25);
    Ref<Canvas> can = pix->makeCanvas();
    BaseContext ctx(*can);
    ctx.setRawColor('#');
    pix->pixels().fill('.');

    Point pts[] = { Point(20, 5), Point(5, 5), Point(20, 20) };
    drawFilledPolygon(ctx, pts);

    a.checkEqual("01", fromBytes(pix->row( 4)), ".........................");
    a.checkEqual("02", fromBytes(pix->row( 5)), ".....###############.....");
    a.checkEqual("03", fromBytes(pix->row( 6)), "......##############.....");
    a.checkEqual("04", fromBytes(pix->row( 7)), ".......#############.....");
    a.checkEqual("05", fromBytes(pix->row( 8)), "........############.....");
    a.checkEqual("06", fromBytes(pix->row( 9)), ".........###########.....");
    a.checkEqual("07", fromBytes(pix->row(10)), "..........##########.....");
    a.checkEqual("08", fromBytes(pix->row(11)), "...........#########.....");
    a.checkEqual("09", fromBytes(pix->row(12)), "............########.....");
    a.checkEqual("10", fromBytes(pix->row(13)), ".............#######.....");
    a.checkEqual("11", fromBytes(pix->row(14)), "..............######.....");
    a.checkEqual("12", fromBytes(pix->row(15)), "...............#####.....");
    a.checkEqual("13", fromBytes(pix->row(16)), "................####.....");
    a.checkEqual("14", fromBytes(pix->row(17)), ".................###.....");
    a.checkEqual("15", fromBytes(pix->row(18)), "..................##.....");
    a.checkEqual("16", fromBytes(pix->row(19)), "...................#.....");
    a.checkEqual("17", fromBytes(pix->row(20)), ".........................");
}

/** Test drawFilledPolygon: triangle, with pattern. */
AFL_TEST("gfx.Complex:drawFilledPolygon:patterned-triangle", a)
{
    Ref<PalettizedPixmap> pix = PalettizedPixmap::create(25, 25);
    Ref<Canvas> can = pix->makeCanvas();
    BaseContext ctx(*can);
    ctx.setRawColor('#');
    ctx.setFillPattern(gfx::FillPattern::GRAY25);
    pix->pixels().fill('.');

    Point pts[] = { Point(5, 5), Point(20, 5), Point(20, 20) };
    drawFilledPolygon(ctx, pts);

    a.checkEqual("01", fromBytes(pix->row( 4)), ".........................");
    a.checkEqual("02", fromBytes(pix->row( 5)), ".........................");
    a.checkEqual("03", fromBytes(pix->row( 6)), ".......#.#.#.#.#.#.#.....");
    a.checkEqual("04", fromBytes(pix->row( 7)), ".........................");
    a.checkEqual("05", fromBytes(pix->row( 8)), "........#.#.#.#.#.#......");
    a.checkEqual("06", fromBytes(pix->row( 9)), ".........................");
    a.checkEqual("07", fromBytes(pix->row(10)), "...........#.#.#.#.#.....");
    a.checkEqual("08", fromBytes(pix->row(11)), ".........................");
    a.checkEqual("09", fromBytes(pix->row(12)), "............#.#.#.#......");
    a.checkEqual("10", fromBytes(pix->row(13)), ".........................");
    a.checkEqual("11", fromBytes(pix->row(14)), "...............#.#.#.....");
    a.checkEqual("12", fromBytes(pix->row(15)), ".........................");
    a.checkEqual("13", fromBytes(pix->row(16)), "................#.#......");
    a.checkEqual("14", fromBytes(pix->row(17)), ".........................");
    a.checkEqual("15", fromBytes(pix->row(18)), "...................#.....");
    a.checkEqual("16", fromBytes(pix->row(19)), ".........................");
    a.checkEqual("17", fromBytes(pix->row(20)), ".........................");
}

/** Test drawFilledPolygon: pentagram (classic self-intersecting). */
AFL_TEST("gfx.Complex:drawFilledPolygon:pentagram", a)
{
    Ref<PalettizedPixmap> pix = PalettizedPixmap::create(40, 40);
    Ref<Canvas> can = pix->makeCanvas();
    BaseContext ctx(*can);
    ctx.setRawColor('#');
    pix->pixels().fill('.');

    Point pts[] = { Point(20, 5), Point(30, 30), Point(5, 12), Point(35, 12), Point(10, 30) };
    drawFilledPolygon(ctx, pts);

    a.checkEqual("01", fromBytes(pix->row( 5)), "........................................");
    a.checkEqual("02", fromBytes(pix->row( 6)), "........................................");
    a.checkEqual("03", fromBytes(pix->row( 7)), "...................##...................");
    a.checkEqual("04", fromBytes(pix->row( 8)), "...................##...................");
    a.checkEqual("05", fromBytes(pix->row( 9)), "..................####..................");
    a.checkEqual("06", fromBytes(pix->row(10)), "..................####..................");
    a.checkEqual("07", fromBytes(pix->row(11)), "..................####..................");
    a.checkEqual("08", fromBytes(pix->row(12)), ".....############......############.....");
    a.checkEqual("09", fromBytes(pix->row(13)), "......###########......###########......");
    a.checkEqual("10", fromBytes(pix->row(14)), "........########........########........");
    a.checkEqual("11", fromBytes(pix->row(15)), ".........#######........#######.........");
    a.checkEqual("12", fromBytes(pix->row(16)), "...........#####........#####...........");
    a.checkEqual("13", fromBytes(pix->row(17)), "............###..........###............");
    a.checkEqual("14", fromBytes(pix->row(18)), ".............##..........##.............");
    a.checkEqual("15", fromBytes(pix->row(19)), "..............#..........#..............");
    a.checkEqual("16", fromBytes(pix->row(20)), "..............##........##..............");
    a.checkEqual("17", fromBytes(pix->row(21)), "..............####.....###..............");
    a.checkEqual("18", fromBytes(pix->row(22)), ".............######..######.............");
    a.checkEqual("19", fromBytes(pix->row(23)), ".............##############.............");
    a.checkEqual("20", fromBytes(pix->row(24)), "............######....######............");
    a.checkEqual("21", fromBytes(pix->row(25)), "............#####......#####............");
    a.checkEqual("22", fromBytes(pix->row(26)), "............####........####............");
    a.checkEqual("23", fromBytes(pix->row(27)), "...........###............###...........");
    a.checkEqual("24", fromBytes(pix->row(28)), "...........##..............##...........");
    a.checkEqual("25", fromBytes(pix->row(29)), "..........#..................#..........");
    a.checkEqual("26", fromBytes(pix->row(30)), "........................................");

}

/** Test drawFilledPolygon: some random polygon. */
AFL_TEST("gfx.Complex:drawFilledPolygon:polygon", a)
{
    Ref<PalettizedPixmap> pix = PalettizedPixmap::create(25, 45);
    Ref<Canvas> can = pix->makeCanvas();
    BaseContext ctx(*can);
    ctx.setRawColor('#');
    pix->pixels().fill('.');

    Point pts[] = { Point(9, 6), Point(21, 12), Point(15, 27), Point(18, 36), Point(6, 42), Point(3, 18) };
    drawFilledPolygon(ctx, pts);

    a.checkEqual("01", fromBytes(pix->row( 6)), ".........................");
    a.checkEqual("02", fromBytes(pix->row( 7)), ".........##..............");
    a.checkEqual("03", fromBytes(pix->row( 8)), "........#####............");
    a.checkEqual("04", fromBytes(pix->row( 9)), "........#######..........");
    a.checkEqual("05", fromBytes(pix->row(10)), ".......##########........");
    a.checkEqual("06", fromBytes(pix->row(11)), ".......############......");
    a.checkEqual("07", fromBytes(pix->row(12)), "......###############....");
    a.checkEqual("08", fromBytes(pix->row(13)), "......###############....");
    a.checkEqual("09", fromBytes(pix->row(14)), ".....###############.....");
    a.checkEqual("10", fromBytes(pix->row(15)), ".....###############.....");
    a.checkEqual("11", fromBytes(pix->row(16)), "....###############......");
    a.checkEqual("12", fromBytes(pix->row(17)), "....###############......");
    a.checkEqual("13", fromBytes(pix->row(18)), "...################......");
    a.checkEqual("14", fromBytes(pix->row(19)), "...###############.......");
    a.checkEqual("15", fromBytes(pix->row(20)), "...###############.......");
    a.checkEqual("16", fromBytes(pix->row(21)), "...##############........");
    a.checkEqual("17", fromBytes(pix->row(22)), "....#############........");
    a.checkEqual("18", fromBytes(pix->row(23)), "....#############........");
    a.checkEqual("19", fromBytes(pix->row(24)), "....############.........");
    a.checkEqual("20", fromBytes(pix->row(25)), "....############.........");
    a.checkEqual("21", fromBytes(pix->row(26)), "....###########..........");
    a.checkEqual("22", fromBytes(pix->row(27)), "....###########..........");
    a.checkEqual("23", fromBytes(pix->row(28)), "....###########..........");
    a.checkEqual("24", fromBytes(pix->row(29)), "....############.........");
    a.checkEqual("25", fromBytes(pix->row(30)), ".....###########.........");
    a.checkEqual("26", fromBytes(pix->row(31)), ".....###########.........");
    a.checkEqual("27", fromBytes(pix->row(32)), ".....############........");
    a.checkEqual("28", fromBytes(pix->row(33)), ".....############........");
    a.checkEqual("29", fromBytes(pix->row(34)), ".....############........");
    a.checkEqual("30", fromBytes(pix->row(35)), ".....#############.......");
    a.checkEqual("31", fromBytes(pix->row(36)), ".....#############.......");
    a.checkEqual("32", fromBytes(pix->row(37)), ".....###########.........");
    a.checkEqual("33", fromBytes(pix->row(38)), "......########...........");
    a.checkEqual("34", fromBytes(pix->row(39)), "......######.............");
    a.checkEqual("35", fromBytes(pix->row(40)), "......####...............");
    a.checkEqual("36", fromBytes(pix->row(41)), "......##.................");
    a.checkEqual("37", fromBytes(pix->row(42)), ".........................");
}

/** Test drawFilledPolygon: a square. */
AFL_TEST("gfx.Complex:drawFilledPolygon:square", a)
{
    Ref<PalettizedPixmap> pix = PalettizedPixmap::create(25, 25);
    Ref<Canvas> can = pix->makeCanvas();
    BaseContext ctx(*can);
    ctx.setRawColor('#');
    pix->pixels().fill('.');

    Point pts[] = { Point(5, 5), Point(20, 5), Point(20, 20), Point(5, 20) };
    drawFilledPolygon(ctx, pts);

    a.checkEqual("01", fromBytes(pix->row( 4)), ".........................");
    a.checkEqual("02", fromBytes(pix->row( 5)), ".....###############.....");
    a.checkEqual("03", fromBytes(pix->row( 6)), ".....###############.....");
    a.checkEqual("04", fromBytes(pix->row( 7)), ".....###############.....");
    a.checkEqual("05", fromBytes(pix->row( 8)), ".....###############.....");
    a.checkEqual("06", fromBytes(pix->row( 9)), ".....###############.....");
    a.checkEqual("07", fromBytes(pix->row(10)), ".....###############.....");
    a.checkEqual("08", fromBytes(pix->row(11)), ".....###############.....");
    a.checkEqual("09", fromBytes(pix->row(12)), ".....###############.....");
    a.checkEqual("10", fromBytes(pix->row(13)), ".....###############.....");
    a.checkEqual("11", fromBytes(pix->row(14)), ".....###############.....");
    a.checkEqual("12", fromBytes(pix->row(15)), ".....###############.....");
    a.checkEqual("13", fromBytes(pix->row(16)), ".....###############.....");
    a.checkEqual("14", fromBytes(pix->row(17)), ".....###############.....");
    a.checkEqual("15", fromBytes(pix->row(18)), ".....###############.....");
    a.checkEqual("16", fromBytes(pix->row(19)), ".....###############.....");
    a.checkEqual("17", fromBytes(pix->row(20)), ".........................");
}

/** Test drawFilledPolygon: a rhombe. */
AFL_TEST("gfx.Complex:drawFilledPolygon:rhombe", a)
{
    Ref<PalettizedPixmap> pix = PalettizedPixmap::create(45, 45);
    Ref<Canvas> can = pix->makeCanvas();
    BaseContext ctx(*can);
    ctx.setRawColor('#');
    pix->pixels().fill('.');

    Point pts[] = { Point(20, 5), Point(40, 20), Point(20, 40), Point(5, 20) };
    drawFilledPolygon(ctx, pts);

    a.checkEqual("01", fromBytes(pix->row( 5)), ".............................................");
    a.checkEqual("02", fromBytes(pix->row( 6)), "...................##........................");
    a.checkEqual("03", fromBytes(pix->row( 7)), "..................#####......................");
    a.checkEqual("04", fromBytes(pix->row( 8)), ".................#######.....................");
    a.checkEqual("05", fromBytes(pix->row( 9)), "................#########....................");
    a.checkEqual("06", fromBytes(pix->row(10)), "...............############..................");
    a.checkEqual("07", fromBytes(pix->row(11)), "..............##############.................");
    a.checkEqual("08", fromBytes(pix->row(12)), ".............################................");
    a.checkEqual("09", fromBytes(pix->row(13)), "............###################..............");
    a.checkEqual("10", fromBytes(pix->row(14)), "...........#####################.............");
    a.checkEqual("11", fromBytes(pix->row(15)), "..........#######################............");
    a.checkEqual("12", fromBytes(pix->row(16)), ".........##########################..........");
    a.checkEqual("13", fromBytes(pix->row(17)), "........############################.........");
    a.checkEqual("14", fromBytes(pix->row(18)), ".......##############################........");
    a.checkEqual("15", fromBytes(pix->row(19)), "......#################################......");
    a.checkEqual("16", fromBytes(pix->row(20)), ".....###################################.....");
    a.checkEqual("17", fromBytes(pix->row(21)), "......#################################......");
    a.checkEqual("18", fromBytes(pix->row(22)), ".......###############################.......");
    a.checkEqual("19", fromBytes(pix->row(23)), ".......##############################........");
    a.checkEqual("20", fromBytes(pix->row(24)), "........############################.........");
    a.checkEqual("21", fromBytes(pix->row(25)), ".........##########################..........");
    a.checkEqual("22", fromBytes(pix->row(26)), "..........########################...........");
    a.checkEqual("23", fromBytes(pix->row(27)), "..........#######################............");
    a.checkEqual("24", fromBytes(pix->row(28)), "...........#####################.............");
    a.checkEqual("25", fromBytes(pix->row(29)), "............###################..............");
    a.checkEqual("26", fromBytes(pix->row(30)), ".............#################...............");
    a.checkEqual("27", fromBytes(pix->row(31)), ".............################................");
    a.checkEqual("28", fromBytes(pix->row(32)), "..............##############.................");
    a.checkEqual("29", fromBytes(pix->row(33)), "...............############..................");
    a.checkEqual("30", fromBytes(pix->row(34)), "................##########...................");
    a.checkEqual("31", fromBytes(pix->row(35)), "................#########....................");
    a.checkEqual("32", fromBytes(pix->row(36)), ".................#######.....................");
    a.checkEqual("33", fromBytes(pix->row(37)), "..................#####......................");
    a.checkEqual("34", fromBytes(pix->row(38)), "...................###.......................");
    a.checkEqual("35", fromBytes(pix->row(39)), "...................##........................");
    a.checkEqual("36", fromBytes(pix->row(40)), ".............................................");
}
