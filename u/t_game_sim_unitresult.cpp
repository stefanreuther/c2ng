/**
  *  \file u/t_game_sim_unitresult.cpp
  *  \brief Test for game::sim::UnitResult
  */

#include "game/sim/unitresult.hpp"

#include "t_game_sim.hpp"
#include "game/sim/configuration.hpp"
#include "game/sim/planet.hpp"
#include "game/sim/ship.hpp"
#include "game/vcr/object.hpp"

/** Test ship handling, torpedo ship.
    A: create before/after ships
    E: expected values are captured */
void
TestGameSimUnitResult::testShip()
{
    // Environment
    game::sim::Ship oldShip;
    oldShip.setOwner(1);
    oldShip.setNumLaunchers(2);
    oldShip.setAmmo(20);
    oldShip.setDamage(3);
    oldShip.setShield(50);
    oldShip.setCrew(200);

    game::sim::Ship newShip;
    newShip.setOwner(1);             // 1 fight won
    newShip.setNumLaunchers(2);
    newShip.setAmmo(12);             // 8 torpedoes fired
    newShip.setDamage(5);            // 5 damage
    newShip.setShield(10);           // 10 shield
    newShip.setCrew(195);            // 195 crew left

    game::vcr::Object obj;

    game::vcr::Statistic stat;
    stat.init(obj, 1);
    for (int i = 0; i < 7; ++i) {
        stat.handleTorpedoHit();     // 7 torpedoes hit
    }

    game::sim::Result res;
    res.init(game::sim::Configuration(), 0);  // Index 0 is required to set the min/max values

    // Test
    game::sim::UnitResult testee;
    testee.addResult(oldShip, newShip, stat, res);

    // Validate
    TS_ASSERT_EQUALS(testee.getNumFightsWon(), 1);
    TS_ASSERT_EQUALS(testee.getNumFights(), 1);
    TS_ASSERT_EQUALS(testee.getNumCaptures(), 0);
    TS_ASSERT_EQUALS(testee.getNumTorpedoesFired().min, 8);
    TS_ASSERT_EQUALS(testee.getNumTorpedoesFired().max, 8);
    TS_ASSERT_EQUALS(testee.getNumTorpedoesFired().totalScaled, 8);
    TS_ASSERT_EQUALS(testee.getDamage().min, 5);
    TS_ASSERT_EQUALS(testee.getShield().min, 10);
    TS_ASSERT_EQUALS(testee.getCrewLeftOrDefenseLost().min, 195);
    TS_ASSERT_EQUALS(testee.getNumTorpedoHits().min, 7);
}

/** Test ship handling, carrier.
    A: create before/after ships
    E: expected values are captured */
void
TestGameSimUnitResult::testShip2()
{
    // Environment
    game::sim::Ship oldShip;
    oldShip.setOwner(1);
    oldShip.setNumLaunchers(0);
    oldShip.setNumBays(3);
    oldShip.setAmmo(50);
    oldShip.setDamage(3);
    oldShip.setShield(50);
    oldShip.setCrew(200);

    game::sim::Ship newShip;
    newShip.setOwner(3);             // 1 captured
    oldShip.setNumLaunchers(0);
    oldShip.setNumBays(3);
    newShip.setAmmo(20);             // 30 fighters lost
    newShip.setDamage(5);            // 5 damage
    newShip.setShield(0);            // 10 shield
    newShip.setCrew(0);              // 195 crew left

    game::vcr::Object obj;
    obj.setNumFighters(50);

    game::vcr::Statistic stat;
    stat.init(obj, 1);
    stat.handleFightersAboard(23);

    game::sim::Result res;
    res.init(game::sim::Configuration(), 0);  // Index 0 is required to set the min/max values

    // Test
    game::sim::UnitResult testee;
    testee.addResult(oldShip, newShip, stat, res);

    // Validate
    TS_ASSERT_EQUALS(testee.getNumFightsWon(), 0);
    TS_ASSERT_EQUALS(testee.getNumFights(), 1);
    TS_ASSERT_EQUALS(testee.getNumCaptures(), 1);
    TS_ASSERT_EQUALS(testee.getNumFightersLost().min, 30);
    TS_ASSERT_EQUALS(testee.getDamage().min, 5);
    TS_ASSERT_EQUALS(testee.getShield().min, 0);
    TS_ASSERT_EQUALS(testee.getCrewLeftOrDefenseLost().min, 0);
    TS_ASSERT_EQUALS(testee.getMinFightersAboard().min, 23);

    // Inversion
    TS_ASSERT_EQUALS(game::sim::UnitResult::Item(testee.getNumFightersLost(), 100, 1).max, 70);
}

/** Test planet handling.
    A: create before/after planets
    E: expected values are captured */
void
TestGameSimUnitResult::testPlanet()
{
    // Environment
    game::sim::Planet oldPlanet;
    oldPlanet.setOwner(1);
    oldPlanet.setNumBaseFighters(20);
    oldPlanet.setDamage(3);
    oldPlanet.setShield(70);
    oldPlanet.setDefense(61);

    game::sim::Planet newPlanet;
    newPlanet.setOwner(1);             // 1 fight won
    newPlanet.setNumBaseFighters(12);  // 8 fighters lost
    newPlanet.setDamage(12);           // 12 damage
    newPlanet.setShield(20);           // 10 shield
    newPlanet.setDefense(57);          // 4 defense lost

    game::vcr::Object obj;
    obj.setNumFighters(25);

    game::vcr::Statistic stat;
    stat.init(obj, 1);
    for (int i = 0; i < 5; ++i) {
        stat.handleTorpedoHit();       // 5 torpedoes hit
    }

    game::sim::Result res;
    res.init(game::sim::Configuration(), 0);  // Index 0 is required to set the min/max values

    // Test
    game::sim::UnitResult testee;
    testee.addResult(oldPlanet, newPlanet, stat, res);

    // Validate
    TS_ASSERT_EQUALS(testee.getNumFightsWon(), 1);
    TS_ASSERT_EQUALS(testee.getNumFights(), 1);
    TS_ASSERT_EQUALS(testee.getNumCaptures(), 0);
    // FIXME: planets do not yet track torpedoes fired
    // TS_ASSERT_EQUALS(testee.getNumTorpedoesFired().min, 5);
    // TS_ASSERT_EQUALS(testee.getNumTorpedoesFired().max, 5);
    // TS_ASSERT_EQUALS(testee.getNumTorpedoesFired().totalScaled, 5);
    TS_ASSERT_EQUALS(testee.getDamage().min, 12);
    TS_ASSERT_EQUALS(testee.getDamage().max, 12);
    TS_ASSERT_EQUALS(testee.getDamage().totalScaled, 12);
    TS_ASSERT_EQUALS(testee.getShield().min, 20);
    TS_ASSERT_EQUALS(testee.getNumFightersLost().min, 8);
    TS_ASSERT_EQUALS(testee.getCrewLeftOrDefenseLost().min, 4);
    TS_ASSERT_EQUALS(testee.getNumTorpedoHits().min, 5);
    TS_ASSERT_EQUALS(testee.getMinFightersAboard().min, 25);
}

/** Test handling of multiple results.
    A: capture multiple results.
    E: expected running totals are captured */
void
TestGameSimUnitResult::testMulti()
{
    game::sim::UnitResult testee;

    // Constant environment
    game::sim::Ship oldShip;
    oldShip.setDamage(3);

    game::vcr::Object obj;

    // First run
    {
        game::sim::Ship newShip;
        newShip.setDamage(30);

        game::vcr::Statistic stat;
        stat.init(obj, 1);

        game::sim::Result res;
        res.init(game::sim::Configuration(), 0);  // Index 0 is required to set the min/max values

        testee.addResult(oldShip, newShip, stat, res);
    }

    // Second run
    {
        game::sim::Ship newShip;
        newShip.setDamage(20);

        game::vcr::Statistic stat;
        stat.init(obj, 1);

        game::sim::Result res;
        res.init(game::sim::Configuration(), 1);

        testee.addResult(oldShip, newShip, stat, res);
    }

    // Third run
    {
        game::sim::Ship newShip;
        newShip.setDamage(40);

        game::vcr::Statistic stat;
        stat.init(obj, 1);

        game::sim::Result res;
        res.init(game::sim::Configuration(), 2);

        testee.addResult(oldShip, newShip, stat, res);
    }

    // Validate
    TS_ASSERT_EQUALS(testee.getDamage().min, 20);
    TS_ASSERT_EQUALS(testee.getDamage().max, 40);
    TS_ASSERT_EQUALS(testee.getDamage().totalScaled, 90);    // = 3*30

    // Inversion
    TS_ASSERT_EQUALS(game::sim::UnitResult::Item(testee.getDamage(), 100, 1).max, 80);
    TS_ASSERT_EQUALS(game::sim::UnitResult::Item(testee.getDamage(), 100, 1).min, 60);

    // Weight change
    testee.changeWeight(1, 4);
    TS_ASSERT_EQUALS(testee.getDamage().min, 20);
    TS_ASSERT_EQUALS(testee.getDamage().max, 40);
    TS_ASSERT_EQUALS(testee.getDamage().totalScaled, 360);   // = 3*30 * 4
}

