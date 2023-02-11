/**
  *  \file u/t_game_map_fleet.cpp
  *  \brief Test for game::map::Fleet
  */

#include "game/map/fleet.hpp"

#include "t_game_map.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/test/simpleturn.hpp"

using game::map::Fleet;
using game::map::Point;
using game::map::Ship;

void
TestGameMapFleet::testIt()
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
    TS_ASSERT_EQUALS(Fleet(t.universe(), s2).countFleetMembers(), 3);
    TS_ASSERT_EQUALS(Fleet(t.universe(), s5).countFleetMembers(), 2);

    Fleet(t.universe(), s2).synchronize(t.config(), t.shipList(), t.mapConfiguration());
    Fleet(t.universe(), s5).synchronize(t.config(), t.shipList(), t.mapConfiguration());

    // Ship 4 is member of fleet 1, with waypoint 1500,1000
    TS_ASSERT_EQUALS(s4.getWaypoint().orElse(Point()), Point(1500, 1100));

    // Ship 1 is member of fleet 5, intercepting #2
    TS_ASSERT_EQUALS(s1.getMission().orElse(-1), 17);
    TS_ASSERT_EQUALS(s1.getMissionParameter(game::InterceptParameter).orElse(-1), 2);

    // Titles
    TS_ASSERT_EQUALS(Fleet(t.universe(), s2).getTitle(tx), "Fleet 2: led by Two");
    TS_ASSERT_EQUALS(Fleet(t.universe(), s5).getTitle(tx), "Fleet 5: Fivers");
}

void
TestGameMapFleet::testHasSpecialFunction()
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
    TS_ASSERT_EQUALS(Fleet(t.universe(), s2).hasSpecialFunction(FUNC_ID, shipScores, t.shipList(), t.config()), true);

    // Add another ship that has no hull function
    t.setHull(20);
    Ship& s3 = t.addShip(3, 3, game::map::Object::Playable);
    s3.setFleetNumber(2);
    TS_ASSERT_EQUALS(Fleet(t.universe(), s2).hasSpecialFunction(FUNC_ID, shipScores, t.shipList(), t.config()), false);
}

