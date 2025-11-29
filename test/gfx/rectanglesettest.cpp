/**
  *  \file test/gfx/rectanglesettest.cpp
  *  \brief Test for gfx::RectangleSet
  */

#include "gfx/rectangleset.hpp"

#include "afl/test/testrunner.hpp"

// Test empty RectangleSet
AFL_TEST("gfx.RectangleSet:empty", a)
{
    gfx::RectangleSet testee;

    a.check("iterator", testee.begin() == testee.end());
    a.check("empty", testee.empty());

    a.check("contains", !testee.contains(gfx::Point(0, 0)));
}

// Test unit set
AFL_TEST("gfx.RectangleSet:unit", a)
{
    gfx::RectangleSet testee(gfx::Rectangle(10, 20, 5, 7));

    a.check("iterator", testee.begin() != testee.end());
    a.check("empty", !testee.empty());
    a.checkEqual("first", *testee.begin(), gfx::Rectangle(10, 20, 5, 7));

    a.check("contains 1", !testee.contains(gfx::Point(0, 0)));
    a.check("contains 2", testee.contains(gfx::Point(10, 20)));
}

// Test intersect()
AFL_TEST("gfx.RectangleSet:intersect", a)
{
    gfx::RectangleSet testee(gfx::Rectangle(10, 20, 50, 70));
    testee.intersect(gfx::Rectangle(20, 10, 100, 50));

    a.check("iterator", testee.begin() != testee.end());
    a.check("empty", !testee.empty());
    a.checkEqual("first", *testee.begin(), gfx::Rectangle(20, 20, 40, 40));
}

// Test intersect(), result is empty
AFL_TEST("gfx.RectangleSet:intersect:empty", a)
{
    gfx::RectangleSet testee(gfx::Rectangle(10, 20, 50, 70));
    testee.intersect(gfx::Rectangle(200, 10, 100, 50));

    a.check("iterator", testee.begin() == testee.end());
    a.check("empty", testee.empty());
}

// Test add(), disjoint case
AFL_TEST("gfx.RectangleSet:add:disjoint", a)
{
    // Two rectangles are disjoint
    gfx::RectangleSet testee(gfx::Rectangle(10, 20, 50, 70));
    testee.add(gfx::Rectangle(100, 10, 20, 30));

    a.check("iterator", testee.begin() != testee.end());
    a.check("empty", !testee.empty());

    gfx::RectangleSet::Iterator_t it = testee.begin();
    a.checkEqual("first",  *it++, gfx::Rectangle(10, 20, 50, 70));
    a.checkEqual("second", *it++, gfx::Rectangle(100, 10, 20, 30));
    a.check("end", it == testee.end());
}

// Test add(), rectangles overlap in a simple way
AFL_TEST("gfx.RectangleSet:add:simple-overlap", a)
{
    // Two rectangles overlap such that only a part of second rectangle is added
    gfx::RectangleSet testee(gfx::Rectangle(10, 20, 50, 70));
    testee.add(gfx::Rectangle(30, 40, 100, 10));

    a.check("iterator", testee.begin() != testee.end());
    a.check("empty", !testee.empty());

    gfx::RectangleSet::Iterator_t it = testee.begin();
    a.checkEqual("first",  *it++, gfx::Rectangle(10, 20, 50, 70));
    a.checkEqual("second", *it++, gfx::Rectangle(60, 40, 70, 10));
    a.check("end", it == testee.end());
}

// Test add(RectangleSet) rectangles overlap in a simple way
AFL_TEST("gfx.RectangleSet:add:simple-overlap:set", a)
{
    // Two rectangles overlap such that only a part of second rectangle is added
    gfx::RectangleSet testee(gfx::Rectangle(10, 20, 50, 70));
    testee.add(gfx::RectangleSet(gfx::Rectangle(30, 40, 100, 10)));

    a.check("iterator", testee.begin() != testee.end());
    a.check("empty", !testee.empty());

    gfx::RectangleSet::Iterator_t it = testee.begin();
    a.checkEqual("first",  *it++, gfx::Rectangle(10, 20, 50, 70));
    a.checkEqual("second", *it++, gfx::Rectangle(60, 40, 70, 10));
    a.check("end", it == testee.end());
}

// Test add(), rectangles overlap in a nontrivial way
AFL_TEST("gfx.RectangleSet:add:general-overlap", a)
{
    //  AAA
    //  AAAB
    //   BBB

    gfx::RectangleSet testee(gfx::Rectangle(0, 0, 3, 2));
    testee.add(gfx::Rectangle(1, 1, 3, 2));

    a.check("iterator", testee.begin() != testee.end());
    a.check("empty", !testee.empty());

    a.check("contains 0 0",  testee.contains(gfx::Point(0, 0)));
    a.check("contains 1 0",  testee.contains(gfx::Point(1, 0)));
    a.check("contains 2 0",  testee.contains(gfx::Point(2, 0)));
    a.check("contains 3 0", !testee.contains(gfx::Point(3, 0)));

    a.check("contains 0 1",  testee.contains(gfx::Point(0, 1)));
    a.check("contains 1 1",  testee.contains(gfx::Point(1, 1)));
    a.check("contains 2 1",  testee.contains(gfx::Point(2, 1)));
    a.check("contains 3 1",  testee.contains(gfx::Point(3, 1)));

    a.check("contains 0 2", !testee.contains(gfx::Point(0, 2)));
    a.check("contains 1 2",  testee.contains(gfx::Point(1, 2)));
    a.check("contains 2 2",  testee.contains(gfx::Point(2, 2)));
    a.check("contains 3 2",  testee.contains(gfx::Point(3, 2)));
}

// Test remove(), rectangles overlap in a nontrivial way
AFL_TEST("gfx.RectangleSet:remove:general-overlap", a)
{
    //  AAA
    //  ABBB
    //   BBB

    gfx::RectangleSet testee(gfx::Rectangle(0, 0, 3, 2));
    testee.remove(gfx::Rectangle(1, 1, 3, 2));

    a.check("iterator", testee.begin() != testee.end());
    a.check("empty", !testee.empty());

    a.check("contains 0 0",  testee.contains(gfx::Point(0, 0)));
    a.check("contains 1 0",  testee.contains(gfx::Point(1, 0)));
    a.check("contains 2 0",  testee.contains(gfx::Point(2, 0)));
    a.check("contains 3 0", !testee.contains(gfx::Point(3, 0)));

    a.check("contains 0 1",  testee.contains(gfx::Point(0, 1)));
    a.check("contains 1 1", !testee.contains(gfx::Point(1, 1)));
    a.check("contains 2 1", !testee.contains(gfx::Point(2, 1)));
    a.check("contains 3 1", !testee.contains(gfx::Point(3, 1)));

    a.check("contains 0 2", !testee.contains(gfx::Point(0, 2)));
    a.check("contains 1 2", !testee.contains(gfx::Point(1, 2)));
    a.check("contains 2 2", !testee.contains(gfx::Point(2, 2)));
    a.check("contains 3 2", !testee.contains(gfx::Point(3, 2)));
}
