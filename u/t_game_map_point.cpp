/**
  *  \file u/t_game_map_point.cpp
  *  \brief Test for game::map::Point
  */

#include "game/map/point.hpp"

#include "t_game_map.hpp"

void
TestGameMapPoint::testBasics()
{
    // ex GameCoordTestSuite::testBasics()
    game::map::Point a(10, 20);
    game::map::Point b(20, 30);
    game::map::Point c(10, 20);

    TS_ASSERT(a != b);
    TS_ASSERT(a == c);
    TS_ASSERT(b != a);
    TS_ASSERT(b != c);
    TS_ASSERT(c == a);
    TS_ASSERT(c != b);

    TS_ASSERT_EQUALS(a.getX(), 10);
    TS_ASSERT_EQUALS(a.getY(), 20);
    TS_ASSERT_EQUALS(b.getX(), 20);
    TS_ASSERT_EQUALS(b.getY(), 30);
    TS_ASSERT_EQUALS(c.getX(), 10);
    TS_ASSERT_EQUALS(c.getY(), 20);
}

