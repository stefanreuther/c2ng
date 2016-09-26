/**
  *  \file u/t_gfx_point.cpp
  *  \brief Test for gfx::Point
  */

#include "gfx/point.hpp"

#include "t_gfx.hpp"

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
    TS_ASSERT(a == gfx::Point(1,2));
    TS_ASSERT(a != b);

    // movedBy
    TS_ASSERT(a + gfx::Point(2, 2) == b);
    TS_ASSERT(b + gfx::Point(-2, -2) == a);

    // scaledBy
    TS_ASSERT(a.scaledBy(5,6) == gfx::Point(5, 12));
    TS_ASSERT(b.scaledBy(7,8) == gfx::Point(21, 32));
}
