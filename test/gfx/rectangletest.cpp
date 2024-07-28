/**
  *  \file test/gfx/rectangletest.cpp
  *  \brief Test for gfx::Rectangle
  */

#include "gfx/rectangle.hpp"

#include "afl/test/testrunner.hpp"

/** Basic tests. */
AFL_TEST("gfx.Rectangle:basics", a)
{
    // Constructors
    gfx::Rectangle nullr;
    gfx::Rectangle ra(10, 20, 30, 40);
    gfx::Rectangle rb(ra);
    gfx::Rectangle rc(ra);
    gfx::Rectangle rd(gfx::Point(10, 20), gfx::Point(30, 40));

    // Query
    a.checkEqual("01. getBottomY", ra.getBottomY(), 60);
    a.checkEqual("02. getRightX",  ra.getRightX(), 40);
    a.checkEqual("03. getLeftX",   ra.getLeftX(), 10);
    a.checkEqual("04. getTopY",    ra.getTopY(), 20);
    a.checkEqual("05. getWidth",   ra.getWidth(), 30);
    a.checkEqual("06. getHeight",  ra.getHeight(), 40);
    a.check("07. getBottomRight",  ra.getBottomRight() == gfx::Point(40, 60));
    a.check("08. getTopLeft",      ra.getTopLeft() == gfx::Point(10, 20));
    a.check("09. getTopRight",     ra.getTopRight() == gfx::Point(40, 20));
    a.check("10. getBottomLeft",   ra.getBottomLeft() == gfx::Point(10, 60));
    a.check("11. getCenter",       ra.getCenter() == gfx::Point(25, 40));
    a.check("12. getSize",         ra.getSize() == gfx::Point(30, 40));

    // Equality
    a.check("21. eq", ra == ra);
    a.check("22. eq", ra == rb);
    a.check("23. eq", ra == rc);
    a.check("24. eq", ra == rd);
    a.check("25. ne", ra != nullr);
    a.check("26. ne", ra != gfx::Rectangle(10, 20, 30, 0));
    a.check("27. ne", ra != gfx::Rectangle(10, 20, 0, 40));
    a.check("28. ne", ra != gfx::Rectangle(10, 0, 30, 40));
    a.check("29. ne", ra != gfx::Rectangle(0, 20, 30, 40));

    // exists
    a.check("31. exists", !nullr.exists());
    a.check("32. exists", ra.exists());

    // contains
    a.check("41. contains", !nullr.contains(1,1));
    a.check("42. contains", !nullr.contains(0,0));
    a.check("43. contains", !ra.contains(1,1));
    a.check("44. contains", !ra.contains(0,0));
    a.check("45. contains", !ra.contains(10,19));
    a.check("46. contains", ra.contains(10,20));
    a.check("47. contains", !ra.contains(40,20));

    a.check("51. contains", ra.contains(gfx::Point(10,20)));
    a.check("52. contains", !ra.contains(gfx::Point(40,20)));

    a.check("61. contains", ra.contains(nullr));
    a.check("62. contains", ra.contains(rb));
    a.check("63. contains", !nullr.contains(ra));
    a.check("64. contains", ra.contains(gfx::Rectangle(10, 20, 10, 10)));
    a.check("65. contains", !ra.contains(gfx::Rectangle(10, 20, 30, 41)));

    // intersect
    {
        gfx::Rectangle z(ra);
        z.intersect(gfx::Rectangle(0, 0, 15, 35));
        a.check("71. intersect", z == gfx::Rectangle(10, 20, 5, 15));

        gfx::Rectangle y(ra);
        y.intersect(gfx::Rectangle(0, 0, 95, 35));
        a.check("81. intersect", y == gfx::Rectangle(10, 20, 30, 15));

        gfx::Rectangle x(ra);
        x.intersect(gfx::Rectangle(0, 0, 15, 95));
        a.check("91. intersect", x == gfx::Rectangle(10, 20, 5, 40));
    }
}

/** Test modification operations. */
AFL_TEST("gfx.Rectangle:modify", a)
{
    gfx::Rectangle ra(10, 5, 30, 20);

    // Set components
    ra.setLeftX(20);
    ra.setTopY(10);
    ra.setWidth(100);
    ra.setHeight(50);
    a.checkEqual("01. set", ra, gfx::Rectangle(20, 10, 100, 50));

    // Include
    // - no change
    ra.include(gfx::Point(30, 20));
    ra.include(gfx::Rectangle(30, 20, 5, 5));
    a.checkEqual("11. include", ra, gfx::Rectangle(20, 10, 100, 50));

    // - to the right/bottom
    ra.include(gfx::Point(130, 60));
    a.checkEqual("21. include", ra, gfx::Rectangle(20, 10, 111, 51));
    ra.include(gfx::Rectangle(100, 100, 70, 60));
    a.checkEqual("22. include", ra, gfx::Rectangle(20, 10, 150, 150));

    // - to the left/top
    ra.include(gfx::Rectangle(15, 8, 2, 2));
    a.checkEqual("31. include", ra, gfx::Rectangle(15, 8, 155, 152));
    ra.include(gfx::Rectangle(10, 5, 20, 30));
    a.checkEqual("32. include", ra, gfx::Rectangle(10, 5, 160, 155));
    ra.include(gfx::Point(1, 1));
    a.checkEqual("33. include", ra, gfx::Rectangle(1, 1, 169, 159));

    // - across
    ra.include(gfx::Rectangle(0, 20, 1000, 30));
    a.checkEqual("41. include", ra, gfx::Rectangle(0, 1, 1000, 159));

    // Move
    a.checkEqual("51. moveTo", ra.moveTo(gfx::Point(10, 20)), gfx::Point(10, 19));
    a.checkEqual("52. moveTo", ra, gfx::Rectangle(10, 20, 1000, 159));
    ra.moveBy(gfx::Point(30, -5));
    a.checkEqual("53. moveBy", ra, gfx::Rectangle(40, 15, 1000, 159));

    // Grow
    ra.grow(-10, -5);
    a.checkEqual("61. grow", ra, gfx::Rectangle(50, 20, 980, 149));
    ra.grow(5, 1);
    a.checkEqual("62. grow", ra, gfx::Rectangle(45, 19, 990, 151));

    // Intersect
    a.check("71. isIntersecting", ra.isIntersecting(ra));
    a.check("72. isIntersecting", ra.isIntersecting(gfx::Rectangle(0, 0, 100, 100)));
    a.check("73. isIntersecting", ra.isIntersecting(gfx::Rectangle(500, 100, 1000, 1000)));
    a.check("74. isIntersecting", ra.isIntersecting(gfx::Rectangle(500, 100, 10, 10)));
    a.check("75. isIntersecting", !ra.isIntersecting(gfx::Rectangle(0, 0, 45, 19)));
    a.check("76. isIntersecting", ra.isIntersecting(gfx::Rectangle(0, 0, 46, 20)));
}

/** Test alignment functions. */
AFL_TEST("gfx.Rectangle:align", a)
{
    gfx::Rectangle t(0, 0, 50, 20);

    // centerWithin: large area
    t.centerWithin(gfx::Rectangle(30, 30, 100, 100));
    a.checkEqual("01. centerWithin", t, gfx::Rectangle(55, 70, 50, 20));

    // centerWithin: it's idempotent!
    t.centerWithin(gfx::Rectangle(30, 30, 100, 100));
    a.checkEqual("11. centerWithin", t, gfx::Rectangle(55, 70, 50, 20));

    // centerWithin: small area
    t.centerWithin(gfx::Rectangle(30, 30, 10, 10));
    a.checkEqual("21. centerWithin", t, gfx::Rectangle(10, 25, 50, 20));

    // moveToEdge
    t.moveToEdge(gfx::Rectangle(0, 0, 640, 480), gfx::LeftAlign, gfx::TopAlign, 10);
    a.checkEqual("31. moveToEdge", t, gfx::Rectangle(10, 10, 50, 20));

    t.moveToEdge(gfx::Rectangle(0, 0, 640, 480), gfx::RightAlign, gfx::MiddleAlign, 20);
    a.checkEqual("41. moveToEdge", t, gfx::Rectangle(570, 230, 50, 20));

    // moveIntoRectangle
    t.moveIntoRectangle(gfx::Rectangle(0, 0, 100, 100));
    a.checkEqual("51. moveIntoRectangle", t, gfx::Rectangle(50, 80, 50, 20));

    t.moveIntoRectangle(gfx::Rectangle(200, 200, 30, 100));
    a.checkEqual("61. moveIntoRectangle", t, gfx::Rectangle(200, 200, 50, 20));
}

/** Test split functions. */

// Regular cases
AFL_TEST("gfx.Rectangle:split:normal", a)
{
    gfx::Rectangle t(0, 0, 100, 100);

    // consumeX
    t.consumeX(10);
    a.checkEqual("01. consumeX", t, gfx::Rectangle(10, 0, 90, 100));

    // splitX
    a.checkEqual("11. splitX", t.splitX(20), gfx::Rectangle(10, 0, 20, 100));
    a.checkEqual("12. splitX", t, gfx::Rectangle(30, 0, 70, 100));

    // consumeY
    t.consumeY(40);
    a.checkEqual("21. consumeY", t, gfx::Rectangle(30, 40, 70, 60));

    // splitY
    a.checkEqual("31. splitY", t.splitY(50), gfx::Rectangle(30, 40, 70, 50));
    a.checkEqual("32. splitY", t, gfx::Rectangle(30, 90, 70, 10));
}

// Underflow
AFL_TEST("gfx.Rectangle:split:underflow", a)
{
    gfx::Rectangle t(0, 0, 100, 100);
    a.check("01. splitX", !t.splitX(-10).exists());
    a.check("02. splitY", !t.splitY(-10).exists());
    t.consumeX(-20);
    t.consumeY(-20);
    a.checkEqual("03. result", t, gfx::Rectangle(0, 0, 100, 100));
}

// Overflow
AFL_TEST("gfx.Rectangle:splitX:overflow", a)
{
    gfx::Rectangle t(0, 0, 100, 100);
    a.checkEqual("01. splitX", t.splitX(200), gfx::Rectangle(0, 0, 100, 100));
    a.check("02. exists", !t.exists());
}

AFL_TEST("gfx.Rectangle:splitY:overflow", a)
{
    gfx::Rectangle t(0, 0, 100, 100);
    a.checkEqual("01. splitY", t.splitY(200), gfx::Rectangle(0, 0, 100, 100));
    a.check("02. exists", !t.exists());
}

AFL_TEST("gfx.Rectangle:consumeX:overflow", a)
{
    gfx::Rectangle t(0, 0, 100, 100);
    t.consumeX(101);
    a.check("", !t.exists());
}

AFL_TEST("gfx.Rectangle:consumeY:overflow", a)
{
    gfx::Rectangle t(0, 0, 100, 100);
    t.consumeY(102);
    a.check("", !t.exists());
}

/*
 *  Test splitBottomY, splitRightX.
 */

AFL_TEST("gfx.Rectangle:splitBottom:normal", a)
{
    gfx::Rectangle t(0, 0, 100, 100);

    // consumeX
    t.consumeRightX(10);
    a.checkEqual("01. consumeRightX", t, gfx::Rectangle(0, 0, 90, 100));

    // splitX
    a.checkEqual("11. splitRightX", t.splitRightX(20), gfx::Rectangle(70, 0, 20, 100));
    a.checkEqual("12. splitRightX", t, gfx::Rectangle(0, 0, 70, 100));

    // consumeY
    t.consumeBottomY(40);
    a.checkEqual("21. consumeBottomY", t, gfx::Rectangle(0, 0, 70, 60));

    // splitY
    a.checkEqual("31. splitBottomY", t.splitBottomY(50), gfx::Rectangle(0, 10, 70, 50));
    a.checkEqual("32. splitBottomY", t, gfx::Rectangle(0, 0, 70, 10));
}

// Underflow
AFL_TEST("gfx.Rectangle:splitBottom:underflow", a)
{
    gfx::Rectangle t(0, 0, 100, 100);
    a.check("01. splitRightX", !t.splitRightX(-10).exists());
    a.check("02. splitBottomY", !t.splitBottomY(-10).exists());
    t.consumeRightX(-20);
    t.consumeBottomY(-20);
    a.checkEqual("03. result", t, gfx::Rectangle(0, 0, 100, 100));
}

// Overflow
AFL_TEST("gfx.Rectangle:splitRightX:overflow", a)
{
    gfx::Rectangle t(0, 0, 100, 100);
    a.checkEqual("01. splitRightX", t.splitRightX(200), gfx::Rectangle(0, 0, 100, 100));
    a.check("02. exists", !t.exists());
}

AFL_TEST("gfx.Rectangle:splitBottomY:overflow", a)
{
    gfx::Rectangle t(0, 0, 100, 100);
    a.checkEqual("01. splitBottomY", t.splitBottomY(200), gfx::Rectangle(0, 0, 100, 100));
    a.check("02. exists", !t.exists());
}

AFL_TEST("gfx.Rectangle:consumeRightX:overflow", a)
{
    gfx::Rectangle t(0, 0, 100, 100);
    t.consumeRightX(101);
    a.check("", !t.exists());
}

AFL_TEST("gfx.Rectangle:consumeBottomY:overflow", a)
{
    gfx::Rectangle t(0, 0, 100, 100);
    t.consumeBottomY(102);
    a.check("", !t.exists());
}


/*
 *  include()
 */

// empty + nonempty
AFL_TEST("gfx.Rectangle:include:empty+nonempty", a)
{
    gfx::Rectangle ra(20, 30, 0, 0);     // empty
    gfx::Rectangle rb(1, 2, 3, 4);
    ra.include(rb);
    a.checkEqual("", ra, gfx::Rectangle(1, 2, 3, 4));
}

// nonempty + empty
AFL_TEST("gfx.Rectangle:include:nonempty+empty", a)
{
    gfx::Rectangle ra(7, 8, 9, 10);
    gfx::Rectangle rb(20, 30, 0, 0);     // empty
    ra.include(rb);
    a.checkEqual("", ra, gfx::Rectangle(7, 8, 9, 10));
}

// empty + empty
AFL_TEST("gfx.Rectangle:empty+empty", a)
{
    gfx::Rectangle ra(7, 8, 0, 0);       // empty
    gfx::Rectangle rb(20, 30, 0, 0);     // empty
    ra.include(rb);
    a.check("", !ra.exists());
}

// nonempty + nonempty
AFL_TEST("gfx.Rectangle:nonempty+nonempty", a)
{
    gfx::Rectangle ra(7, 8, 9, 10);
    gfx::Rectangle rb(1, 2, 3, 4);
    ra.include(rb);
    a.checkEqual("", ra, gfx::Rectangle(1, 2, 15, 16));
}
