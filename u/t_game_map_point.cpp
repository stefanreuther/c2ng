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

    TS_ASSERT_EQUALS(a.toString(), "(10,20)");
}

/** Test modification functions. */
void
TestGameMapPoint::testModify()
{
    game::map::Point a(10, 20);

    a.setX(3);
    TS_ASSERT_EQUALS(a.getX(), 3);
    TS_ASSERT_EQUALS(a.get(a.X), 3);

    a.setY(9);
    TS_ASSERT_EQUALS(a.getY(), 9);
    TS_ASSERT_EQUALS(a.get(a.Y), 9);

    a.addX(5);
    TS_ASSERT_EQUALS(a.getX(), 8);

    a.addY(12);
    TS_ASSERT_EQUALS(a.getY(), 21);

    a.set(a.X, 77);
    TS_ASSERT_EQUALS(a.getX(), 77);

    a.set(a.Y, 66);
    TS_ASSERT_EQUALS(a.getY(), 66);
}

/** Test operators. */
void
TestGameMapPoint::testOperators()
{
    using game::map::Point;
    TS_ASSERT_EQUALS(Point(10, 20) == Point(10, 20), true);
    TS_ASSERT_EQUALS(Point(10, 20) == Point(10, 11), false);
    TS_ASSERT_EQUALS(Point(10, 20) == Point(11, 20), false);

    TS_ASSERT_EQUALS(Point(10, 20) != Point(10, 20), false);
    TS_ASSERT_EQUALS(Point(10, 20) != Point(10, 11), true);
    TS_ASSERT_EQUALS(Point(10, 20) != Point(11, 20), true);

    Point a = Point(10, 20) + Point(3, 4);
    TS_ASSERT_EQUALS(a.getX(), 13);
    TS_ASSERT_EQUALS(a.getY(), 24);

    a = Point(10, 20) - Point(3, 4);
    TS_ASSERT_EQUALS(a.getX(), 7);
    TS_ASSERT_EQUALS(a.getY(), 16);

    a = Point(10, 20);
    TS_ASSERT_EQUALS(&(a += Point(5, 6)), &a);
    TS_ASSERT_EQUALS(a.getX(), 15);
    TS_ASSERT_EQUALS(a.getY(), 26);

    a = Point(10, 20);
    TS_ASSERT_EQUALS(&(a -= Point(5, 6)), &a);
    TS_ASSERT_EQUALS(a.getX(), 5);
    TS_ASSERT_EQUALS(a.getY(), 14);
}

/** Test parseCoordinates(). */
void
TestGameMapPoint::testParse()
{
    game::map::Point a(1000, 2000);
    TS_ASSERT_EQUALS(a.parseCoordinates("500,600"), true);
    TS_ASSERT_EQUALS(a.getX(), 500);
    TS_ASSERT_EQUALS(a.getY(), 600);

    TS_ASSERT_EQUALS(a.parseCoordinates("(501,601)"), true);
    TS_ASSERT_EQUALS(a.getX(), 501);
    TS_ASSERT_EQUALS(a.getY(), 601);

    TS_ASSERT_EQUALS(a.parseCoordinates("  (  502\t ,  602  )  "), true);
    TS_ASSERT_EQUALS(a.getX(), 502);
    TS_ASSERT_EQUALS(a.getY(), 602);

    TS_ASSERT_EQUALS(a.parseCoordinates("  200 ,   300"), true);
    TS_ASSERT_EQUALS(a.getX(), 200);
    TS_ASSERT_EQUALS(a.getY(), 300);

    TS_ASSERT_EQUALS(a.parseCoordinates("-50,-100"), true);
    TS_ASSERT_EQUALS(a.getX(), -50);
    TS_ASSERT_EQUALS(a.getY(), -100);

    TS_ASSERT_EQUALS(a.parseCoordinates("-1,+3"), true);
    TS_ASSERT_EQUALS(a.getX(), -1);
    TS_ASSERT_EQUALS(a.getY(), +3);
}

/** Test parseCoordinates() failures. */
void
TestGameMapPoint::testParseFail()
{
    game::map::Point a(333, 444);

    // Parse failure does not modify result
    TS_ASSERT_EQUALS(a.parseCoordinates(""), false);
    TS_ASSERT_EQUALS(a.getX(), 333);
    TS_ASSERT_EQUALS(a.getY(), 444);

    // Other failures
    TS_ASSERT_EQUALS(a.parseCoordinates("1"), false);
    TS_ASSERT_EQUALS(a.parseCoordinates("1,"), false);
    TS_ASSERT_EQUALS(a.parseCoordinates(",1"), false);
    TS_ASSERT_EQUALS(a.parseCoordinates("1,,1"), false);
    TS_ASSERT_EQUALS(a.parseCoordinates("1,2,"), false);
    TS_ASSERT_EQUALS(a.parseCoordinates("1a,2b"), false);
    TS_ASSERT_EQUALS(a.parseCoordinates("0x10,0x20"), false);
    TS_ASSERT_EQUALS(a.parseCoordinates("(500,600"), false);
    TS_ASSERT_EQUALS(a.parseCoordinates("500,600)"), false);
    TS_ASSERT_EQUALS(a.parseCoordinates("(500), 600"), false);
    TS_ASSERT_EQUALS(a.parseCoordinates("10 20"), false);

    // Still not modified
    TS_ASSERT_EQUALS(a.getX(), 333);
    TS_ASSERT_EQUALS(a.getY(), 444);
}

/** Test compare() function. */
void
TestGameMapPoint::testCompare()
{
    using game::map::Point;
    TS_ASSERT_EQUALS(Point(100, 100).compare(Point(100, 100)), 0);

    // Y difference
    TS_ASSERT_EQUALS(Point(100, 100).compare(Point(100, 101)), -1);
    TS_ASSERT_EQUALS(Point(100, 101).compare(Point(100, 100)), 1);

    // X difference
    TS_ASSERT_EQUALS(Point(100, 100).compare(Point(101, 100)), -1);
    TS_ASSERT_EQUALS(Point(101, 100).compare(Point(100, 100)), 1);

    // Y difference has precedence over X difference
    TS_ASSERT_EQUALS(Point(101, 100).compare(Point(100, 101)), -1);
    TS_ASSERT_EQUALS(Point(100, 101).compare(Point(101, 100)), 1);
}

/** Test distance functions. */
void
TestGameMapPoint::testDistance()
{
    using game::map::Point;
    TS_ASSERT_EQUALS(Point(100, 200).getSquaredRawDistance(Point(103, 204)), 25);
    TS_ASSERT_EQUALS(Point(100, 200).getSquaredRawDistance(Point( 96, 197)), 25);
    TS_ASSERT_EQUALS(Point(100, 200).getSquaredRawDistance(Point(120, 200)), 400);

    TS_ASSERT_EQUALS(Point(100, 200).isCloserThan(Point(120, 200), 20), false);
    TS_ASSERT_EQUALS(Point(100, 200).isCloserThan(Point(120, 200), 21), true);
}
