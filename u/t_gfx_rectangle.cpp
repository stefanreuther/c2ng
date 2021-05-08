/**
  *  \file u/t_gfx_rectangle.cpp
  *  \brief Test for gfx::Rectangle
  */

#include <sstream>
#include "gfx/rectangle.hpp"

#include "t_gfx.hpp"

/** Basic tests. */
void
TestGfxRectangle::testIt()
{
    // Constructors
    gfx::Rectangle nullr;
    gfx::Rectangle a(10, 20, 30, 40);
    gfx::Rectangle b(a);
    gfx::Rectangle c(a);
    gfx::Rectangle d(gfx::Point(10, 20), gfx::Point(30, 40));

    // Query
    TS_ASSERT_EQUALS(a.getBottomY(), 60);
    TS_ASSERT_EQUALS(a.getRightX(), 40);
    TS_ASSERT_EQUALS(a.getLeftX(), 10);
    TS_ASSERT_EQUALS(a.getTopY(), 20);
    TS_ASSERT_EQUALS(a.getWidth(), 30);
    TS_ASSERT_EQUALS(a.getHeight(), 40);
    TS_ASSERT(a.getBottomRight() == gfx::Point(40, 60));
    TS_ASSERT(a.getTopLeft() == gfx::Point(10, 20));
    TS_ASSERT(a.getTopRight() == gfx::Point(40, 20));
    TS_ASSERT(a.getBottomLeft() == gfx::Point(10, 60));
    TS_ASSERT(a.getCenter() == gfx::Point(25, 40));
    TS_ASSERT(a.getSize() == gfx::Point(30, 40));

    // Equality
    TS_ASSERT(a == a);
    TS_ASSERT(a == b);
    TS_ASSERT(a == c);
    TS_ASSERT(a == d);
    TS_ASSERT(a != nullr);
    TS_ASSERT(a != gfx::Rectangle(10, 20, 30, 0));
    TS_ASSERT(a != gfx::Rectangle(10, 20, 0, 40));
    TS_ASSERT(a != gfx::Rectangle(10, 0, 30, 40));
    TS_ASSERT(a != gfx::Rectangle(0, 20, 30, 40));

    // exists
    TS_ASSERT(!nullr.exists());
    TS_ASSERT(a.exists());

    // contains
    TS_ASSERT(!nullr.contains(1,1));
    TS_ASSERT(!nullr.contains(0,0));
    TS_ASSERT(!a.contains(1,1));
    TS_ASSERT(!a.contains(0,0));
    TS_ASSERT(!a.contains(10,19));
    TS_ASSERT(a.contains(10,20));
    TS_ASSERT(!a.contains(40,20));

    TS_ASSERT(a.contains(gfx::Point(10,20)));
    TS_ASSERT(!a.contains(gfx::Point(40,20)));


    TS_ASSERT(a.contains(nullr));
    TS_ASSERT(a.contains(b));
    TS_ASSERT(!nullr.contains(a));
    TS_ASSERT(a.contains(gfx::Rectangle(10, 20, 10, 10)));
    TS_ASSERT(!a.contains(gfx::Rectangle(10, 20, 30, 41)));

    // ex clipSize
    {
        gfx::Rectangle z(a);
        z.intersect(gfx::Rectangle(0, 0, 15, 35));
        TS_ASSERT(z == gfx::Rectangle(10, 20, 5, 15));

        gfx::Rectangle y(a);
        y.intersect(gfx::Rectangle(0, 0, 95, 35));
        TS_ASSERT(y == gfx::Rectangle(10, 20, 30, 15));

        gfx::Rectangle x(a);
        x.intersect(gfx::Rectangle(0, 0, 15, 95));
        TS_ASSERT(x == gfx::Rectangle(10, 20, 5, 40));
    }
}

/** Test modification operations. */
void
TestGfxRectangle::testModify()
{
    gfx::Rectangle a(10, 5, 30, 20);

    // Set components
    a.setLeftX(20);
    a.setTopY(10);
    a.setWidth(100);
    a.setHeight(50);
    TS_ASSERT_EQUALS(a, gfx::Rectangle(20, 10, 100, 50));

    // Include
    // - no change
    a.include(gfx::Point(30, 20));
    a.include(gfx::Rectangle(30, 20, 5, 5));
    TS_ASSERT_EQUALS(a, gfx::Rectangle(20, 10, 100, 50));

    // - to the right/bottom
    a.include(gfx::Point(130, 60));
    TS_ASSERT_EQUALS(a, gfx::Rectangle(20, 10, 111, 51));
    a.include(gfx::Rectangle(100, 100, 70, 60));
    TS_ASSERT_EQUALS(a, gfx::Rectangle(20, 10, 150, 150));

    // - to the left/top
    a.include(gfx::Rectangle(15, 8, 2, 2));
    TS_ASSERT_EQUALS(a, gfx::Rectangle(15, 8, 155, 152));
    a.include(gfx::Rectangle(10, 5, 20, 30));
    TS_ASSERT_EQUALS(a, gfx::Rectangle(10, 5, 160, 155));
    a.include(gfx::Point(1, 1));
    TS_ASSERT_EQUALS(a, gfx::Rectangle(1, 1, 169, 159));

    // - across
    a.include(gfx::Rectangle(0, 20, 1000, 30));
    TS_ASSERT_EQUALS(a, gfx::Rectangle(0, 1, 1000, 159));

    // Move
    TS_ASSERT_EQUALS(a.moveTo(gfx::Point(10, 20)), gfx::Point(10, 19));
    TS_ASSERT_EQUALS(a, gfx::Rectangle(10, 20, 1000, 159));
    a.moveBy(gfx::Point(30, -5));
    TS_ASSERT_EQUALS(a, gfx::Rectangle(40, 15, 1000, 159));

    // Grow
    a.grow(-10, -5);
    TS_ASSERT_EQUALS(a, gfx::Rectangle(50, 20, 980, 149));
    a.grow(5, 1);
    TS_ASSERT_EQUALS(a, gfx::Rectangle(45, 19, 990, 151));

    // Intersect
    TS_ASSERT(a.isIntersecting(a));
    TS_ASSERT(a.isIntersecting(gfx::Rectangle(0, 0, 100, 100)));
    TS_ASSERT(a.isIntersecting(gfx::Rectangle(500, 100, 1000, 1000)));
    TS_ASSERT(a.isIntersecting(gfx::Rectangle(500, 100, 10, 10)));
    TS_ASSERT(!a.isIntersecting(gfx::Rectangle(0, 0, 45, 19)));
    TS_ASSERT(a.isIntersecting(gfx::Rectangle(0, 0, 46, 20)));
}

/** Test alignment functions. */
void
TestGfxRectangle::testAlign()
{
    gfx::Rectangle t(0, 0, 50, 20);

    // centerWithin: large area
    t.centerWithin(gfx::Rectangle(30, 30, 100, 100));
    TS_ASSERT_EQUALS(t, gfx::Rectangle(55, 70, 50, 20));

    // centerWithin: it's idempotent!
    t.centerWithin(gfx::Rectangle(30, 30, 100, 100));
    TS_ASSERT_EQUALS(t, gfx::Rectangle(55, 70, 50, 20));

    // centerWithin: small area
    t.centerWithin(gfx::Rectangle(30, 30, 10, 10));
    TS_ASSERT_EQUALS(t, gfx::Rectangle(10, 25, 50, 20));

    // moveToEdge
    t.moveToEdge(gfx::Rectangle(0, 0, 640, 480), gfx::LeftAlign, gfx::TopAlign, 10);
    TS_ASSERT_EQUALS(t, gfx::Rectangle(10, 10, 50, 20));

    t.moveToEdge(gfx::Rectangle(0, 0, 640, 480), gfx::RightAlign, gfx::MiddleAlign, 20);
    TS_ASSERT_EQUALS(t, gfx::Rectangle(570, 230, 50, 20));

    // moveIntoRectangle
    t.moveIntoRectangle(gfx::Rectangle(0, 0, 100, 100));
    TS_ASSERT_EQUALS(t, gfx::Rectangle(50, 80, 50, 20));

    t.moveIntoRectangle(gfx::Rectangle(200, 200, 30, 100));
    TS_ASSERT_EQUALS(t, gfx::Rectangle(200, 200, 50, 20));
}

/** Test split functions. */
void
TestGfxRectangle::testSplit()
{
    // Regular cases
    {
        gfx::Rectangle t(0, 0, 100, 100);

        // consumeX
        t.consumeX(10);
        TS_ASSERT_EQUALS(t, gfx::Rectangle(10, 0, 90, 100));

        // splitX
        TS_ASSERT_EQUALS(t.splitX(20), gfx::Rectangle(10, 0, 20, 100));
        TS_ASSERT_EQUALS(t, gfx::Rectangle(30, 0, 70, 100));

        // consumeY
        t.consumeY(40);
        TS_ASSERT_EQUALS(t, gfx::Rectangle(30, 40, 70, 60));
    
        // splitY
        TS_ASSERT_EQUALS(t.splitY(50), gfx::Rectangle(30, 40, 70, 50));
        TS_ASSERT_EQUALS(t, gfx::Rectangle(30, 90, 70, 10));
    }

    // Underflow
    {
        gfx::Rectangle t(0, 0, 100, 100);
        TS_ASSERT(!t.splitX(-10).exists());
        TS_ASSERT(!t.splitY(-10).exists());
        t.consumeX(-20);
        t.consumeY(-20);
        TS_ASSERT_EQUALS(t, gfx::Rectangle(0, 0, 100, 100));
    }

    // Overflow
    {
        gfx::Rectangle t(0, 0, 100, 100);
        TS_ASSERT_EQUALS(t.splitX(200), gfx::Rectangle(0, 0, 100, 100));
        TS_ASSERT(!t.exists());
    }
    {
        gfx::Rectangle t(0, 0, 100, 100);
        TS_ASSERT_EQUALS(t.splitY(200), gfx::Rectangle(0, 0, 100, 100));
        TS_ASSERT(!t.exists());
    }
    {
        gfx::Rectangle t(0, 0, 100, 100);
        t.consumeX(101);
        TS_ASSERT(!t.exists());
    }
    {
        gfx::Rectangle t(0, 0, 100, 100);
        t.consumeY(102);
        TS_ASSERT(!t.exists());
    }
}

/** Test formatting. */
void
TestGfxRectangle::testFormat()
{
    std::stringstream os;
    os << gfx::Rectangle(10, 20, 30, 40);
    TS_ASSERT_EQUALS(os.str(), "30x40+10+20");
}

/** Test splitBottomY, splitRightX. */
void
TestGfxRectangle::testSplit2()
{
    {
        gfx::Rectangle t(0, 0, 100, 100);

        // consumeX
        t.consumeRightX(10);
        TS_ASSERT_EQUALS(t, gfx::Rectangle(0, 0, 90, 100));

        // splitX
        TS_ASSERT_EQUALS(t.splitRightX(20), gfx::Rectangle(70, 0, 20, 100));
        TS_ASSERT_EQUALS(t, gfx::Rectangle(0, 0, 70, 100));

        // consumeY
        t.consumeBottomY(40);
        TS_ASSERT_EQUALS(t, gfx::Rectangle(0, 0, 70, 60));
    
        // splitY
        TS_ASSERT_EQUALS(t.splitBottomY(50), gfx::Rectangle(0, 10, 70, 50));
        TS_ASSERT_EQUALS(t, gfx::Rectangle(0, 0, 70, 10));
    }
    // Underflow
    {
        gfx::Rectangle t(0, 0, 100, 100);
        TS_ASSERT(!t.splitRightX(-10).exists());
        TS_ASSERT(!t.splitBottomY(-10).exists());
        t.consumeRightX(-20);
        t.consumeBottomY(-20);
        TS_ASSERT_EQUALS(t, gfx::Rectangle(0, 0, 100, 100));
    }

    // Overflow
    {
        gfx::Rectangle t(0, 0, 100, 100);
        TS_ASSERT_EQUALS(t.splitRightX(200), gfx::Rectangle(0, 0, 100, 100));
        TS_ASSERT(!t.exists());
    }
    {
        gfx::Rectangle t(0, 0, 100, 100);
        TS_ASSERT_EQUALS(t.splitBottomY(200), gfx::Rectangle(0, 0, 100, 100));
        TS_ASSERT(!t.exists());
    }
    {
        gfx::Rectangle t(0, 0, 100, 100);
        t.consumeRightX(101);
        TS_ASSERT(!t.exists());
    }
    {
        gfx::Rectangle t(0, 0, 100, 100);
        t.consumeBottomY(102);
        TS_ASSERT(!t.exists());
    }
}

/** Test include(). */
void
TestGfxRectangle::testInclude()
{
    // empty + nonempty
    {
        gfx::Rectangle a(20, 30, 0, 0);     // empty
        gfx::Rectangle b(1, 2, 3, 4);
        a.include(b);
        TS_ASSERT_EQUALS(a, gfx::Rectangle(1, 2, 3, 4));
    }

    // nonempty + empty
    {
        gfx::Rectangle a(7, 8, 9, 10);
        gfx::Rectangle b(20, 30, 0, 0);     // empty
        a.include(b);
        TS_ASSERT_EQUALS(a, gfx::Rectangle(7, 8, 9, 10));
    }

    // empty + empty
    {
        gfx::Rectangle a(7, 8, 0, 0);       // empty
        gfx::Rectangle b(20, 30, 0, 0);     // empty
        a.include(b);
        TS_ASSERT(!a.exists());
    }

    // nonempty + nonempty
    {
        gfx::Rectangle a(7, 8, 9, 10);
        gfx::Rectangle b(1, 2, 3, 4);
        a.include(b);
        TS_ASSERT_EQUALS(a, gfx::Rectangle(1, 2, 15, 16));
    }
}
