/**
  *  \file test/game/map/pointtest.cpp
  *  \brief Test for game::map::Point
  */

#include "game/map/point.hpp"
#include "afl/test/testrunner.hpp"

using game::map::Point;

AFL_TEST("game.map.Point:basics", a)
{
    // ex GameCoordTestSuite::testBasics()
    Point pa(10, 20);
    Point pb(20, 30);
    Point pc(10, 20);

    a.check("01. ne", pa != pb);
    a.check("02. eq", pa == pc);
    a.check("03. ne", pb != pa);
    a.check("04. ne", pb != pc);
    a.check("05. eq", pc == pa);
    a.check("06. ne", pc != pb);

    a.checkEqual("11. getX", pa.getX(), 10);
    a.checkEqual("12. getY", pa.getY(), 20);
    a.checkEqual("13. getX", pb.getX(), 20);
    a.checkEqual("14. getY", pb.getY(), 30);
    a.checkEqual("15. getX", pc.getX(), 10);
    a.checkEqual("16. getY", pc.getY(), 20);

    a.checkEqual("21. toString", pa.toString(), "(10,20)");
}

/** Test modification functions. */
AFL_TEST("game.map.Point:modify", a)
{
    Point pa(10, 20);

    pa.setX(3);
    a.checkEqual("01. getX", pa.getX(), 3);
    a.checkEqual("02. get", pa.get(pa.X), 3);

    pa.setY(9);
    a.checkEqual("11. getY", pa.getY(), 9);
    a.checkEqual("12. get", pa.get(pa.Y), 9);

    pa.addX(5);
    a.checkEqual("21. getX", pa.getX(), 8);

    pa.addY(12);
    a.checkEqual("31. getY", pa.getY(), 21);

    pa.set(pa.X, 77);
    a.checkEqual("41. getX", pa.getX(), 77);

    pa.set(pa.Y, 66);
    a.checkEqual("51. getY", pa.getY(), 66);
}

/** Test operators. */
AFL_TEST("game.map.Point:operators", a)
{
    a.checkEqual("01. eq", Point(10, 20) == Point(10, 20), true);
    a.checkEqual("02. eq", Point(10, 20) == Point(10, 11), false);
    a.checkEqual("03. eq", Point(10, 20) == Point(11, 20), false);

    a.checkEqual("11. ne", Point(10, 20) != Point(10, 20), false);
    a.checkEqual("12. ne", Point(10, 20) != Point(10, 11), true);
    a.checkEqual("13. ne", Point(10, 20) != Point(11, 20), true);

    Point pa = Point(10, 20) + Point(3, 4);
    a.checkEqual("21. getX", pa.getX(), 13);
    a.checkEqual("22. getY", pa.getY(), 24);

    pa = Point(10, 20) - Point(3, 4);
    a.checkEqual("31. getX", pa.getX(), 7);
    a.checkEqual("32. getY", pa.getY(), 16);

    pa = Point(10, 20);
    a.checkEqual("41. op+=", &(pa += Point(5, 6)), &pa);
    a.checkEqual("42. getX", pa.getX(), 15);
    a.checkEqual("43. getY", pa.getY(), 26);

    pa = Point(10, 20);
    a.checkEqual("51. op-=", &(pa -= Point(5, 6)), &pa);
    a.checkEqual("52. getX", pa.getX(), 5);
    a.checkEqual("53. getY", pa.getY(), 14);
}

/** Test parseCoordinates(). */
AFL_TEST("game.map.Point:parseCoordinates:success", a)
{
    Point pa(1000, 2000);
    a.checkEqual("01. parseCoordinates", pa.parseCoordinates("500,600"), true);
    a.checkEqual("02. getX", pa.getX(), 500);
    a.checkEqual("03. getY", pa.getY(), 600);

    a.checkEqual("11. parseCoordinates", pa.parseCoordinates("(501,601)"), true);
    a.checkEqual("12. getX", pa.getX(), 501);
    a.checkEqual("13. getY", pa.getY(), 601);

    a.checkEqual("21. parseCoordinates", pa.parseCoordinates("  (  502\t ,  602  )  "), true);
    a.checkEqual("22. getX", pa.getX(), 502);
    a.checkEqual("23. getY", pa.getY(), 602);

    a.checkEqual("31. parseCoordinates", pa.parseCoordinates("  200 ,   300"), true);
    a.checkEqual("32. getX", pa.getX(), 200);
    a.checkEqual("33. getY", pa.getY(), 300);

    a.checkEqual("41. parseCoordinates", pa.parseCoordinates("-50,-100"), true);
    a.checkEqual("42. getX", pa.getX(), -50);
    a.checkEqual("43. getY", pa.getY(), -100);

    a.checkEqual("51. parseCoordinates", pa.parseCoordinates("-1,+3"), true);
    a.checkEqual("52. getX", pa.getX(), -1);
    a.checkEqual("53. getY", pa.getY(), +3);
}

/** Test parseCoordinates() failures. */
AFL_TEST("game.map.Point:parseCoordinates:fail", a)
{
    Point pa(333, 444);

    // Parse failure does not modify result
    a.checkEqual("01. parseCoordinates", pa.parseCoordinates(""), false);
    a.checkEqual("02. getX", pa.getX(), 333);
    a.checkEqual("03. getY", pa.getY(), 444);

    // Other failures
    a.checkEqual("11. parseCoordinates", pa.parseCoordinates("1"), false);
    a.checkEqual("12. parseCoordinates", pa.parseCoordinates("1,"), false);
    a.checkEqual("13. parseCoordinates", pa.parseCoordinates(",1"), false);
    a.checkEqual("14. parseCoordinates", pa.parseCoordinates("1,,1"), false);
    a.checkEqual("15. parseCoordinates", pa.parseCoordinates("1,2,"), false);
    a.checkEqual("16. parseCoordinates", pa.parseCoordinates("1a,2b"), false);
    a.checkEqual("17. parseCoordinates", pa.parseCoordinates("0x10,0x20"), false);
    a.checkEqual("18. parseCoordinates", pa.parseCoordinates("(500,600"), false);
    a.checkEqual("19. parseCoordinates", pa.parseCoordinates("500,600)"), false);
    a.checkEqual("20. parseCoordinates", pa.parseCoordinates("(500), 600"), false);
    a.checkEqual("21. parseCoordinates", pa.parseCoordinates("10 20"), false);

    // Still not modified
    a.checkEqual("31. getX", pa.getX(), 333);
    a.checkEqual("32. getY", pa.getY(), 444);
}

/** Test compare() function. */
AFL_TEST("game.map.Point:compare", a)
{
    a.checkEqual("01", Point(100, 100).compare(Point(100, 100)), 0);

    // Y difference
    a.checkEqual("11", Point(100, 100).compare(Point(100, 101)), -1);
    a.checkEqual("12", Point(100, 101).compare(Point(100, 100)), 1);

    // X difference
    a.checkEqual("21", Point(100, 100).compare(Point(101, 100)), -1);
    a.checkEqual("22", Point(101, 100).compare(Point(100, 100)), 1);

    // Y difference has precedence over X difference
    a.checkEqual("31", Point(101, 100).compare(Point(100, 101)), -1);
    a.checkEqual("32", Point(100, 101).compare(Point(101, 100)), 1);
}

/** Test distance functions. */
AFL_TEST("game.map.Point:distance", a)
{
    a.checkEqual("01. getSquaredRawDistance", Point(100, 200).getSquaredRawDistance(Point(103, 204)), 25);
    a.checkEqual("02. getSquaredRawDistance", Point(100, 200).getSquaredRawDistance(Point( 96, 197)), 25);
    a.checkEqual("03. getSquaredRawDistance", Point(100, 200).getSquaredRawDistance(Point(120, 200)), 400);

    a.checkEqual("11. isCloserThan", Point(100, 200).isCloserThan(Point(120, 200), 20), false);
    a.checkEqual("12. isCloserThan", Point(100, 200).isCloserThan(Point(120, 200), 21), true);
}
