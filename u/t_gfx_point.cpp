/**
  *  \file u/t_gfx_point.cpp
  *  \brief Test for gfx::Point
  */

#include <sstream>
#include "gfx/point.hpp"

#include "t_gfx.hpp"

/** Simple test. */
void
TestGfxPoint::testIt()
{
    const gfx::Point a(1,2);
    const gfx::Point b(3,4);

    // Construction, equality, inequality
    TS_ASSERT_EQUALS(a.getX(), 1);
    TS_ASSERT_EQUALS(a.getY(), 2);
    TS_ASSERT_EQUALS(b.getX(), 3);
    TS_ASSERT_EQUALS(b.getY(), 4);
    TS_ASSERT(a == a);
    TS_ASSERT(!(a != a));
    TS_ASSERT(a == gfx::Point(1, 2));
    TS_ASSERT(a != b);
    TS_ASSERT(!(a == b));
    TS_ASSERT(a != gfx::Point(1, 3));
    TS_ASSERT(a != gfx::Point(2, 1));
    TS_ASSERT(a != gfx::Point(2, 2));
    TS_ASSERT(!(a == gfx::Point(1, 3)));
    TS_ASSERT(!(a == gfx::Point(2, 1)));
    TS_ASSERT(!(a == gfx::Point(2, 2)));

    // movedBy
    TS_ASSERT(a + gfx::Point(2, 2) == b);
    TS_ASSERT(b + gfx::Point(-2, -2) == a);

    // scaledBy
    TS_ASSERT(a.scaledBy(5,6) == gfx::Point(5, 12));
    TS_ASSERT(b.scaledBy(7,8) == gfx::Point(21, 32));
    TS_ASSERT(a.scaledBy(b)   == gfx::Point(3, 8));

    // modify
    gfx::Point p = a;
    p.setX(9);
    p.setY(10);
    p.addX(11);
    p.addY(12);
    TS_ASSERT_EQUALS(p.getX(), 20);
    TS_ASSERT_EQUALS(p.getY(), 22);

    // +, -
    TS_ASSERT_EQUALS(p + b, gfx::Point(23, 26));
    TS_ASSERT_EQUALS(p - b, gfx::Point(17, 18));

    TS_ASSERT_EQUALS(&(p += a), &p);
    TS_ASSERT_EQUALS(p, gfx::Point(21, 24));

    TS_ASSERT_EQUALS(&(p -= b), &p);
    TS_ASSERT_EQUALS(p, gfx::Point(18, 20));

    // stream
    std::stringstream ss;
    TS_ASSERT_EQUALS(&(ss << p), &ss);
    TS_ASSERT_EQUALS(ss.str(), "18,20");
}
