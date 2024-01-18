/**
  *  \file test/game/map/fleettest.cpp
  *  \brief Test for game::map::Fleet
  */

#include "game/map/fleet.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/test/simpleturn.hpp"

using game::map::Fleet;
using game::map::Point;
using game::map::Ship;

/** General test. */
AFL_TEST("game.map.Fleet:basics", a)
{
    afl::string::NullTranslator tx;
    game::test::SimpleTurn t;
    t.setPosition(Point(1000, 1000));
    t.shipList().missions().addMission(game::spec::Mission(17, "!is*,Intercept"));

    Ship& s1 = t.addShip(1, 3, game::map::Object::Playable);
    Ship& s2 = t.addShip(2, 3, game::map::Object::Playable);
    Ship& s3 = t.addShip(3, 3, game::map::Object::Playable);
    Ship& s4 = t.addShip(4, 3, game::map::Object::Playable);
    Ship& s5 = t.addShip(5, 3, game::map::Object::Playable);
    // Fleet #2
    s2.setFleetNumber(2);
    s3.setFleetNumber(2);
    s4.setFleetNumber(2);
    s2.setWaypoint(Point(1500, 1100));
    s2.setName("Two");
    // Fleet #3
    s1.setFleetNumber(5);
    s5.setFleetNumber(5);
    s5.setName("Five");
    s5.setFleetName("Fivers");
    s5.setMission(17, 2, 0);

    // Verify
    a.checkEqual("01. countFleetMembers", Fleet(t.universe(), s2).countFleetMembers(), 3);
    a.checkEqual("02. countFleetMembers", Fleet(t.universe(), s5).countFleetMembers(), 2);

    Fleet(t.universe(), s2).synchronize(t.config(), t.shipList(), t.mapConfiguration());
    Fleet(t.universe(), s5).synchronize(t.config(), t.shipList(), t.mapConfiguration());

    // Ship 4 is member of fleet 1, with waypoint 1500,1000
    a.checkEqual("11. getWaypoint", s4.getWaypoint().orElse(Point()), Point(1500, 1100));

    // Ship 1 is member of fleet 5, intercepting #2
    a.checkEqual("21. getMission", s1.getMission().orElse(-1), 17);
    a.checkEqual("22. getMissionParameter", s1.getMissionParameter(game::InterceptParameter).orElse(-1), 2);

    // Titles
    a.checkEqual("31. getTitle", Fleet(t.universe(), s2).getTitle(tx), "Fleet 2: led by Two");
    a.checkEqual("32. getTitle", Fleet(t.universe(), s5).getTitle(tx), "Fleet 5: Fivers");
}

/** Test Fleet::hasSpecialFunction(). */
AFL_TEST("game.map.Fleet:hasSpecialFunction", a)
{
    const int FUNC_ID = 17;
    game::UnitScoreDefinitionList shipScores;

    // Define ships with a special function
    game::test::SimpleTurn t;
    t.setHull(10);
    t.hull().changeHullFunction(t.shipList().modifiedHullFunctions().getFunctionIdFromHostId(FUNC_ID), game::PlayerSet_t::allUpTo(30), game::PlayerSet_t(), true);
    Ship& s1 = t.addShip(1, 3, game::map::Object::Playable);
    Ship& s2 = t.addShip(2, 3, game::map::Object::Playable);

    s1.setFleetNumber(2);
    s2.setFleetNumber(2);

    // Fleet has function
    a.checkEqual("01. hasSpecialFunction", Fleet(t.universe(), s2).hasSpecialFunction(FUNC_ID, shipScores, t.shipList(), t.config()), true);

    // Add another ship that has no hull function
    t.setHull(20);
    Ship& s3 = t.addShip(3, 3, game::map::Object::Playable);
    s3.setFleetNumber(2);
    a.checkEqual("11. hasSpecialFunction", Fleet(t.universe(), s2).hasSpecialFunction(FUNC_ID, shipScores, t.shipList(), t.config()), false);
}

/** Test Fleet::getMaxEfficientWarp(). */

// Single ship, normal case
AFL_TEST("game.map.Fleet:getMaxEfficientWarp:single", a)
{
    afl::string::NullTranslator tx;
    game::test::SimpleTurn t;
    Ship& s = t.addShip(1, 3, game::map::Object::Playable);
    s.setEngineType(7);
    t.shipList().engines().create(7)->setMaxEfficientWarp(6);

    a.checkEqual("", Fleet(t.universe(), s).getMaxEfficientWarp(t.shipList()), 6);
}

// Single ship, undefined engine
AFL_TEST("game.map.Fleet:getMaxEfficientWarp:undefined-engine", a)
{
    afl::string::NullTranslator tx;
    game::test::SimpleTurn t;
    Ship& s = t.addShip(1, 3, game::map::Object::Playable);
    s.setEngineType(7);

    a.checkEqual("", Fleet(t.universe(), s).getMaxEfficientWarp(t.shipList()), 9);
}

// Fleet
AFL_TEST("game.map.Fleet:getMaxEfficientWarp:fleet", a)
{
    afl::string::NullTranslator tx;
    game::test::SimpleTurn t;
    Ship& s1 = t.addShip(1, 3, game::map::Object::Playable);
    s1.setEngineType(7);
    s1.setFleetNumber(9);
    t.shipList().engines().create(7)->setMaxEfficientWarp(5);

    Ship& s2 = t.addShip(9, 3, game::map::Object::Playable);
    s2.setEngineType(8);
    s2.setFleetNumber(9);
    t.shipList().engines().create(8)->setMaxEfficientWarp(9);

    a.checkEqual("", Fleet(t.universe(), s2).getMaxEfficientWarp(t.shipList()), 5);
}

/** Test synchronisation when Intercept is being used to intercept a member. */
AFL_TEST("game.map.Fleet:synchronize:intercept", a)
{
    afl::string::NullTranslator tx;
    game::test::SimpleTurn t;
    t.setPosition(Point(1000, 1000));
    t.shipList().missions().addMission(game::spec::Mission(17, "!is*,Intercept"));

    Ship& s1 = t.addShip(1, 3, game::map::Object::Playable);
    Ship& s2 = t.addShip(2, 3, game::map::Object::Playable);
    Ship& s3 = t.addShip(3, 3, game::map::Object::Playable);

    s1.setEngineType(7);
    s2.setEngineType(7);
    s3.setEngineType(7);
    t.shipList().engines().create(7)->setMaxEfficientWarp(6);

    // Fleet #2
    s1.setFleetNumber(2);
    s2.setFleetNumber(2);
    s3.setFleetNumber(2);
    s2.setMission(17, 3, 0);
    s2.setWarpFactor(3);

    // Synchronize
    Fleet(t.universe(), s2).synchronize(t.config(), t.shipList(), t.mapConfiguration());

    // Verify missions
    a.checkEqual("01. getMission", s1.getMission().orElse(0), 17);
    a.checkEqual("02. getMission", s2.getMission().orElse(0), 17);
    a.checkEqual("03. getMission", s3.getMission().orElse(0), 1);
    a.checkEqual("04. getMissionParameter", s1.getMissionParameter(game::InterceptParameter).orElse(0), 3);
    a.checkEqual("05. getMissionParameter", s2.getMissionParameter(game::InterceptParameter).orElse(0), 3);

    // Verify speeds
    a.checkEqual("11. getWarpFactor", s1.getWarpFactor().orElse(0), 3);
    a.checkEqual("12. getWarpFactor", s2.getWarpFactor().orElse(0), 3);
    a.checkEqual("13. getWarpFactor", s3.getWarpFactor().orElse(0), 6);
}

