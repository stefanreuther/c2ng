/**
  *  \file u/t_game_map_explosiontype.cpp
  *  \brief Test for game::map::ExplosionType
  */

#include "game/map/explosiontype.hpp"

#include "t_game_map.hpp"

using game::map::Explosion;
using game::map::Point;

/** Test initial state (empty).
    A: create ExplosionType. Call iteration functions.
    E: must report no content */
void
TestGameMapExplosionType::testInit()
{
    game::map::ExplosionType testee;
    TS_ASSERT_EQUALS(testee.getNextIndex(0), 0);
    TS_ASSERT_EQUALS(testee.getPreviousIndex(0), 0);
    TS_ASSERT(testee.getObjectByIndex(1) == 0);
    TS_ASSERT(testee.getObjectByIndex(-1) == 0);
}

/** Test iteration.
    A: create ExplosionType and add some explosions. Call iteration functions.
    E: must report correct content */
void
TestGameMapExplosionType::testIteration()
{
    game::map::ExplosionType testee;
    testee.add(Explosion(10, Point(200, 300)));
    testee.add(Explosion(20, Point(400, 500)));

    // Forward iteration
    game::Id_t firstIndex = testee.getNextIndex(0);
    TS_ASSERT_DIFFERS(firstIndex, 0);
    const Explosion* e = testee.getObjectByIndex(firstIndex);
    TS_ASSERT(e);
    TS_ASSERT_EQUALS(e->getId(), 10);

    game::Id_t secondIndex = testee.getNextIndex(firstIndex);
    TS_ASSERT_DIFFERS(secondIndex, 0);
    e = testee.getObjectByIndex(secondIndex);
    TS_ASSERT(e);
    TS_ASSERT_EQUALS(e->getId(), 20);

    TS_ASSERT_EQUALS(testee.getNextIndex(secondIndex), 0);

    // Backward iteration must produce same indexes
    TS_ASSERT_EQUALS(testee.getPreviousIndex(0), secondIndex);
    TS_ASSERT_EQUALS(testee.getPreviousIndex(secondIndex), firstIndex);
    TS_ASSERT_EQUALS(testee.getPreviousIndex(firstIndex), 0);
}

/** Test addMessageInformation().
    A: create ExplosionType. Call addMessageInformation() with some explosion.
    E: must report correct content */
void
TestGameMapExplosionType::testAddMessageInformation()
{
    namespace gp = game::parser;

    game::map::ExplosionType testee;

    // Add message
    gp::MessageInformation info(gp::MessageInformation::Explosion, 15, 1);
    info.addValue(gp::mi_X, 333);
    info.addValue(gp::mi_Y, 444);
    info.addValue(gp::ms_Name, "Boomer");
    info.addValue(gp::mi_ExplodedShipId, 80);
    testee.addMessageInformation(info);

    // Verify
    game::Id_t firstIndex = testee.getNextIndex(0);
    TS_ASSERT_DIFFERS(firstIndex, 0);
    const Explosion* e = testee.getObjectByIndex(firstIndex);

    TS_ASSERT_EQUALS(e->getId(), 15);
    TS_ASSERT_EQUALS(e->getShipId(), 80);
    TS_ASSERT_EQUALS(e->getShipName(), "Boomer");

    Point pt;
    TS_ASSERT_EQUALS(e->getPosition(pt), true);
    TS_ASSERT_EQUALS(pt, Point(333, 444));
}

