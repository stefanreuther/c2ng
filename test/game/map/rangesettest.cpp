/**
  *  \file test/game/map/rangesettest.cpp
  *  \brief Test for game::map::RangeSet
  */

#include "game/map/rangeset.hpp"

#include "afl/test/testrunner.hpp"
#include "game/map/explosion.hpp"
#include "game/map/explosiontype.hpp"

using game::map::Point;

/** Test initialisation.
    A: create RangeSet.
    E: check expected values */
AFL_TEST("game.map.RangeSet:init", a)
{
    game::map::RangeSet testee;
    a.checkEqual("01. isEmpty", testee.isEmpty(), true);
    a.checkEqual("02. getMin", testee.getMin(), Point());
    a.checkEqual("03. getMax", testee.getMax(), Point());
    a.check("04. empty interval", testee.begin() == testee.end());
}

/** Test adding a point.
    A: create RangeSet. Add a point.
    E: check expected values */
AFL_TEST("game.map.RangeSet:add", a)
{
    game::map::RangeSet testee;
    testee.add(Point(300, 400), 100);

    a.checkEqual("01. isEmpty", testee.isEmpty(), false);
    a.checkEqual("02. getMin", testee.getMin(), Point(200, 300));
    a.checkEqual("03. getMax", testee.getMax(), Point(400, 500));
    a.check("04. nonempty interval", testee.begin() !=testee.end());

    game::map::RangeSet::Iterator_t it = testee.begin();
    a.checkEqual("11. point", it->first, Point(300, 400));
    a.checkEqual("12. radius", it->second, 100);
    ++it;
    a.check("13. end", it == testee.end());
}

/** Test adding concentric points.
    A: create RangeSet. Add concentric points.
    E: check expected values */
AFL_TEST("game.map.RangeSet:add:concentric", a)
{
    game::map::RangeSet testee;
    testee.add(Point(300, 400), 100);
    testee.add(Point(300, 400),  50);
    testee.add(Point(300, 400), 200);
    testee.add(Point(300, 400), 150);

    a.checkEqual("01. isEmpty", testee.isEmpty(), false);
    a.checkEqual("02. getMin", testee.getMin(), Point(100, 200));
    a.checkEqual("03. getMax", testee.getMax(), Point(500, 600));
    a.check("04. nonempty interval", testee.begin() !=testee.end());

    game::map::RangeSet::Iterator_t it = testee.begin();
    a.checkEqual("11. point", it->first, Point(300, 400));
    a.checkEqual("12. radius", it->second, 200);
    ++it;
    a.check("13. end", it == testee.end());
}

/** Test adding multiple points.
    A: create RangeSet. Add multiple points.
    E: check expected values */
AFL_TEST("game.map.RangeSet:add:multiple", a)
{
    game::map::RangeSet testee;
    testee.add(Point(300, 400), 100);
    testee.add(Point(600, 500),  50);
    testee.add(Point(200, 300), 100);

    a.checkEqual("01. isEmpty", testee.isEmpty(), false);
    a.checkEqual("02. getMin", testee.getMin(), Point(100, 200));
    a.checkEqual("03. getMax", testee.getMax(), Point(650, 550));
    a.check("04. nonempty interval", testee.begin() != testee.end());
}

/** Test addObjectType().
    A: create RangeSet. Create an ObjectType with multiple objects.
    E: check expected values */
AFL_TEST("game.map.RangeSet:addObjectType", a)
{
    // Use ExplosionType because it is simplest
    using game::map::ExplosionType;
    using game::map::Explosion;
    ExplosionType ty;
    ty.add(Explosion(0, Point(500, 400)));
    ty.add(Explosion(0, Point(200, 800)));

    // We rely on Explosion returning owner 0, not unknown
    int owner = -1;
    a.checkEqual("01. getOwner", Explosion(0, Point(1,2)).getOwner().get(owner), true);
    a.checkEqual("02. owner", owner, 0);

    // Test goes here:
    game::map::RangeSet testee;
    testee.addObjectType(ty, game::PlayerSet_t(0), false, 30);

    // Verify
    a.checkEqual("11. isEmpty", testee.isEmpty(), false);
    a.checkEqual("12. getMin", testee.getMin(), Point(170, 370));
    a.checkEqual("13. getMax", testee.getMax(), Point(530, 830));
}

/** Test clear.
    A: create RangeSet. Add a point. Call clear().
    E: check expected values */
AFL_TEST("game.map.RangeSet:clear", a)
{
    game::map::RangeSet testee;
    testee.add(Point(100, 200), 30);
    testee.clear();

    a.checkEqual("01. isEmpty", testee.isEmpty(), true);
    a.checkEqual("02. getMin", testee.getMin(), Point());
    a.checkEqual("03. getMax", testee.getMax(), Point());
    a.check("04. empty interval", testee.begin() == testee.end());
}
