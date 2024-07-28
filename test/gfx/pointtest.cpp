/**
  *  \file test/gfx/pointtest.cpp
  *  \brief Test for gfx::Point
  */

#include "gfx/point.hpp"

#include "afl/test/testrunner.hpp"

using gfx::Point;

/** Simple test. */
AFL_TEST("gfx.Point:basics", a)
{
    const Point pa(1,2);
    const Point pb(3,4);

    // Construction, equality, inequality
    a.checkEqual("01. getX", pa.getX(), 1);
    a.checkEqual("02. getY", pa.getY(), 2);
    a.checkEqual("03. getX", pb.getX(), 3);
    a.checkEqual("04. getY", pb.getY(), 4);
    a.check("05. eq", pa == pa);
    a.check("06. ne", !(pa != pa));
    a.check("07. eq", pa == Point(1, 2));
    a.check("08. ne", pa != pb);
    a.check("09. eq", !(pa == pb));
    a.check("10. ne", pa != Point(1, 3));
    a.check("11. ne", pa != Point(2, 1));
    a.check("12. ne", pa != Point(2, 2));
    a.check("13. eq", !(pa == Point(1, 3)));
    a.check("14. eq", !(pa == Point(2, 1)));
    a.check("15. eq", !(pa == Point(2, 2)));

    // movedBy
    a.check("21. plus", pa + Point(2, 2) == pb);
    a.check("22. plus", pb + Point(-2, -2) == pa);

    // scaledBy
    a.check("31. scaledBy", pa.scaledBy(5,6) == Point(5, 12));
    a.check("32. scaledBy", pb.scaledBy(7,8) == Point(21, 32));
    a.check("33. scaledBy", pa.scaledBy(pb)   == Point(3, 8));

    // modify
    Point p = pa;
    p.setX(9);
    p.setY(10);
    p.addX(11);
    p.addY(12);
    a.checkEqual("41. getX", p.getX(), 20);
    a.checkEqual("42. getY", p.getY(), 22);

    // +, -
    a.checkEqual("51. plus", p + pb, Point(23, 26));
    a.checkEqual("52. minus", p - pb, Point(17, 18));

    a.checkEqual("61. inc", &(p += pa), &p);
    a.checkEqual("62. inc", p, Point(21, 24));

    a.checkEqual("71. dec", &(p -= pb), &p);
    a.checkEqual("72. dec", p, Point(18, 20));
}

/** Test extendRight, extendBelow. */
AFL_TEST("gfx.Point:extend", a)
{
    a.checkEqual("01. extendRight", Point(10, 5).extendRight(Point(20, 4)),  Point(30, 5));
    a.checkEqual("02. extendRight", Point(10, 5).extendRight(Point(20, 12)), Point(30, 12));

    a.checkEqual("11. extendBelow", Point(10, 5).extendBelow(Point(20, 4)),  Point(20, 9));
    a.checkEqual("12. extendBelow", Point(10, 5).extendBelow(Point( 5, 12)), Point(10, 17));
}
