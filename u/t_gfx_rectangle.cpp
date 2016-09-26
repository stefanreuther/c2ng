/**
  *  \file u/t_gfx_rectangle.cpp
  *  \brief Test for gfx::Rectangle
  */

#include "gfx/rectangle.hpp"

#include "t_gfx.hpp"

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
    TS_ASSERT(a.getBottomRight() == gfx::Point(40, 60));
    TS_ASSERT(a.getTopLeft() == gfx::Point(10, 20));
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
