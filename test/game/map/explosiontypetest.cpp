/**
  *  \file test/game/map/explosiontypetest.cpp
  *  \brief Test for game::map::ExplosionType
  */

#include "game/map/explosiontype.hpp"
#include "afl/test/testrunner.hpp"

using game::map::Explosion;
using game::map::Point;

/** Test initial state (empty).
    A: create ExplosionType. Call iteration functions.
    E: must report no content */
AFL_TEST("game.map.ExplosionType:init", a)
{
    game::map::ExplosionType testee;
    a.checkEqual("01. getNextIndex", testee.getNextIndex(0), 0);
    a.checkEqual("02. getPreviousIndex", testee.getPreviousIndex(0), 0);
    a.checkNull("03. getObjectByIndex", testee.getObjectByIndex(1));
    a.checkNull("04. getObjectByIndex", testee.getObjectByIndex(-1));
}

/** Test iteration.
    A: create ExplosionType and add some explosions. Call iteration functions.
    E: must report correct content */
AFL_TEST("game.map.ExplosionType:iteration", a)
{
    game::map::ExplosionType testee;
    testee.add(Explosion(10, Point(200, 300)));
    testee.add(Explosion(20, Point(400, 500)));

    // Forward iteration
    game::Id_t firstIndex = testee.getNextIndex(0);
    a.checkDifferent("01. firstIndex", firstIndex, 0);
    const Explosion* e = testee.getObjectByIndex(firstIndex);
    a.check("02. getObjectByIndex", e);
    a.checkEqual("03. getId", e->getId(), 10);

    game::Id_t secondIndex = testee.getNextIndex(firstIndex);
    a.checkDifferent("11. getNextIndex", secondIndex, 0);
    e = testee.getObjectByIndex(secondIndex);
    a.check("12. getObjectByIndex", e);
    a.checkEqual("13. getId", e->getId(), 20);

    a.checkEqual("21. getNextIndex", testee.getNextIndex(secondIndex), 0);

    // Backward iteration must produce same indexes
    a.checkEqual("31. getPreviousIndex", testee.getPreviousIndex(0), secondIndex);
    a.checkEqual("32. getPreviousIndex", testee.getPreviousIndex(secondIndex), firstIndex);
    a.checkEqual("33. getPreviousIndex", testee.getPreviousIndex(firstIndex), 0);
}

/** Test addMessageInformation().
    A: create ExplosionType. Call addMessageInformation() with some explosion.
    E: must report correct content */
AFL_TEST("game.map.ExplosionType:addMessageInformation", a)
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
    a.checkDifferent("01. firstIndex", firstIndex, 0);
    const Explosion* e = testee.getObjectByIndex(firstIndex);

    a.checkEqual("11. getId", e->getId(), 15);
    a.checkEqual("12. getShipId", e->getShipId(), 80);
    a.checkEqual("13. getShipName", e->getShipName(), "Boomer");

    Point pt;
    a.checkEqual("21. getPosition", e->getPosition().get(pt), true);
    a.checkEqual("22. pt", pt, Point(333, 444));
}
