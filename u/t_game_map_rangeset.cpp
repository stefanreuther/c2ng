/**
  *  \file u/t_game_map_rangeset.cpp
  *  \brief Test for game::map::RangeSet
  */

#include "game/map/rangeset.hpp"

#include "t_game_map.hpp"
#include "game/map/explosion.hpp"
#include "game/map/explosiontype.hpp"

using game::map::Point;

/** Test initialisation.
    A: create RangeSet.
    E: check expected values */
void
TestGameMapRangeSet::testInit()
{
    game::map::RangeSet testee;
    TS_ASSERT_EQUALS(testee.isEmpty(), true);
    TS_ASSERT_EQUALS(testee.getMin(), Point());
    TS_ASSERT_EQUALS(testee.getMax(), Point());
    TS_ASSERT_EQUALS(testee.begin(), testee.end());
}

/** Test adding a point.
    A: create RangeSet. Add a point.
    E: check expected values */
void
TestGameMapRangeSet::testAdd()
{
    game::map::RangeSet testee;
    testee.add(Point(300, 400), 100);

    TS_ASSERT_EQUALS(testee.isEmpty(), false);
    TS_ASSERT_EQUALS(testee.getMin(), Point(200, 300));
    TS_ASSERT_EQUALS(testee.getMax(), Point(400, 500));
    TS_ASSERT_DIFFERS(testee.begin(), testee.end());

    game::map::RangeSet::Iterator_t it = testee.begin();
    TS_ASSERT_EQUALS(it->first, Point(300, 400));
    TS_ASSERT_EQUALS(it->second, 100);
    ++it;
    TS_ASSERT_EQUALS(it, testee.end());
}

/** Test adding concentric points.
    A: create RangeSet. Add concentric points.
    E: check expected values */
void
TestGameMapRangeSet::testAddConcentric()
{
    game::map::RangeSet testee;
    testee.add(Point(300, 400), 100);
    testee.add(Point(300, 400),  50);
    testee.add(Point(300, 400), 200);
    testee.add(Point(300, 400), 150);

    TS_ASSERT_EQUALS(testee.isEmpty(), false);
    TS_ASSERT_EQUALS(testee.getMin(), Point(100, 200));
    TS_ASSERT_EQUALS(testee.getMax(), Point(500, 600));
    TS_ASSERT_DIFFERS(testee.begin(), testee.end());

    game::map::RangeSet::Iterator_t it = testee.begin();
    TS_ASSERT_EQUALS(it->first, Point(300, 400));
    TS_ASSERT_EQUALS(it->second, 200);
    ++it;
    TS_ASSERT_EQUALS(it, testee.end());
}

/** Test adding multiple points.
    A: create RangeSet. Add multiple points.
    E: check expected values */
void
TestGameMapRangeSet::testAddMultiple()
{
    game::map::RangeSet testee;
    testee.add(Point(300, 400), 100);
    testee.add(Point(600, 500),  50);
    testee.add(Point(200, 300), 100);

    TS_ASSERT_EQUALS(testee.isEmpty(), false);
    TS_ASSERT_EQUALS(testee.getMin(), Point(100, 200));
    TS_ASSERT_EQUALS(testee.getMax(), Point(650, 550));
    TS_ASSERT_DIFFERS(testee.begin(), testee.end());
}

/** Test addObjectType().
    A: create RangeSet. Create an ObjectType with multiple objects.
    E: check expected values */
void
TestGameMapRangeSet::testAddObjectType()
{
    // Use ExplosionType because it is simplest
    using game::map::ExplosionType;
    using game::map::Explosion;
    ExplosionType ty;
    ty.add(Explosion(0, Point(500, 400)));
    ty.add(Explosion(0, Point(200, 800)));

    // We rely on Explosion returning owner 0, not unknown
    int owner = -1;
    TS_ASSERT_EQUALS(Explosion(0, Point(1,2)).getOwner(owner), true);
    TS_ASSERT_EQUALS(owner, 0);

    // Test goes here:
    game::map::RangeSet testee;
    testee.addObjectType(ty, game::PlayerSet_t(0), false, 30);

    // Verify
    TS_ASSERT_EQUALS(testee.isEmpty(), false);
    TS_ASSERT_EQUALS(testee.getMin(), Point(170, 370));
    TS_ASSERT_EQUALS(testee.getMax(), Point(530, 830));
}

/** Test clear.
    A: create RangeSet. Add a point. Call clear().
    E: check expected values */
void
TestGameMapRangeSet::testClear()
{
    game::map::RangeSet testee;
    testee.add(Point(100, 200), 30);
    testee.clear();

    TS_ASSERT_EQUALS(testee.isEmpty(), true);
    TS_ASSERT_EQUALS(testee.getMin(), Point());
    TS_ASSERT_EQUALS(testee.getMax(), Point());
    TS_ASSERT_EQUALS(testee.begin(), testee.end());
}

