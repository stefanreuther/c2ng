/**
  *  \file u/t_game_shipquery.cpp
  *  \brief Test for game::ShipQuery
  */

#include "game/shipquery.hpp"

#include "t_game.hpp"

/** Test initialisation, setters, getters.
    A: create ShipQuery. Use setters.
    E: expected initial state is set. Setters affect corresponding getters. */
void
TestGameShipQuery::testInit()
{
    game::ShipQuery a, b;
    TS_ASSERT_EQUALS(a == b, true);
    TS_ASSERT_EQUALS(a != b, false);

    // All scalars empty
    TS_ASSERT_EQUALS(a.getHullType(), 0);
    TS_ASSERT_EQUALS(a.getShipId(), 0);
    TS_ASSERT_EQUALS(a.getEngineType(), 0);
    TS_ASSERT_EQUALS(a.getCombatMass(), 0);
    TS_ASSERT_EQUALS(a.getUsedESBRate(), 0);
    TS_ASSERT_EQUALS(a.getCrew(), 0);
    TS_ASSERT_EQUALS(a.getOwner(), 0);

    // Level filter: defaults to all
    TS_ASSERT_EQUALS(a.getLevelFilterSet().contains(1), true);
    TS_ASSERT_EQUALS(a.getLevelFilterSet().contains(0), true);

    // Level display: defaults to level 0
    TS_ASSERT_EQUALS(a.getLevelDisplaySet().contains(1), false);
    TS_ASSERT_EQUALS(a.getLevelDisplaySet().contains(0), true);

    // Player filter: defaults to all
    TS_ASSERT_EQUALS(a.getPlayerFilterSet().contains(1), true);
    TS_ASSERT_EQUALS(a.getPlayerFilterSet().contains(0), true);

    // Player display: defaults to none
    TS_ASSERT_EQUALS(a.getPlayerDisplaySet().contains(1), false);
    TS_ASSERT_EQUALS(a.getPlayerDisplaySet().contains(0), false);

    // Modify and check success
    game::PlayerSet_t ps1 = game::PlayerSet_t::fromInteger(2);
    game::PlayerSet_t ps2 = game::PlayerSet_t::fromInteger(5);
    game::ExperienceLevelSet_t ls1 = game::ExperienceLevelSet_t::fromInteger(7);
    game::ExperienceLevelSet_t ls2 = game::ExperienceLevelSet_t::fromInteger(9);

    a.setHullType(42);
    a.setShipId(150);
    a.setEngineType(9);
    a.setCombatMass(500, 20);
    a.setCrew(92);
    a.setOwner(5);
    a.setPlayerFilterSet(ps1);
    a.setPlayerDisplaySet(ps2);
    a.setLevelFilterSet(ls1);
    a.setLevelDisplaySet(ls2);

    TS_ASSERT_EQUALS(a.getHullType(), 42);
    TS_ASSERT_EQUALS(a.getShipId(), 150);
    TS_ASSERT_EQUALS(a.getEngineType(), 9);
    TS_ASSERT_EQUALS(a.getCombatMass(), 500);
    TS_ASSERT_EQUALS(a.getUsedESBRate(), 20);
    TS_ASSERT_EQUALS(a.getCrew(), 92);
    TS_ASSERT_EQUALS(a.getOwner(), 5);
    TS_ASSERT_EQUALS(a.getPlayerFilterSet(), ps1);
    TS_ASSERT_EQUALS(a.getPlayerDisplaySet(), ps2);
    TS_ASSERT_EQUALS(a.getLevelFilterSet(), ls1);
    TS_ASSERT_EQUALS(a.getLevelDisplaySet(), ls2);
    TS_ASSERT_EQUALS(a == b, false);
    TS_ASSERT_EQUALS(a != b, true);
}

/** Test initForExistingShip().
    A: create universe, shiplist with a ship. Call initForExistingShip().
    E: all attributes of the ship are taken over */
void
TestGameShipQuery::testInitForExistingShip()
{
    const int SHIP_ID = 17;
    const int HULL_NR = 15;
    const int ENGINE_NR = 8;
    const int PLAYER_NR = 6;

    // Universe with a ship
    game::map::Universe univ;
    game::map::Ship* sh = univ.ships().create(SHIP_ID);

    game::map::ShipData sd;
    sd.x = 1000;
    sd.y = 2000;
    sd.hullType = HULL_NR;
    sd.engineType = ENGINE_NR;
    sd.owner = PLAYER_NR;
    sh->addCurrentShipData(sd, game::PlayerSet_t(PLAYER_NR));
    sh->internalCheck();
    sh->setPlayability(game::map::Object::Playable);

    // Ship list
    game::spec::ShipList shipList;
    game::spec::Hull* h = shipList.hulls().create(HULL_NR);
    h->setMass(500);
    h->setMaxCrew(99);
    shipList.engines().create(ENGINE_NR)->cost().set(game::spec::Cost::Money, 400);

    // Configuration
    game::config::HostConfiguration config;
    config[game::config::HostConfiguration::AllowEngineShieldBonus].set(1);
    config[game::config::HostConfiguration::EngineShieldBonusRate].set(25);

    // Score definitions
    game::UnitScoreDefinitionList scoreDefs;

    // Testee
    game::ShipQuery a;
    a.initForExistingShip(univ, SHIP_ID, shipList, config, scoreDefs);

    // Verify
    TS_ASSERT_EQUALS(a.getHullType(), HULL_NR);
    TS_ASSERT_EQUALS(a.getShipId(), SHIP_ID);
    TS_ASSERT_EQUALS(a.getEngineType(), ENGINE_NR);
    TS_ASSERT_EQUALS(a.getCombatMass(), 600);
    TS_ASSERT_EQUALS(a.getUsedESBRate(), 25);
    TS_ASSERT_EQUALS(a.getCrew(), 99);
    TS_ASSERT_EQUALS(a.getOwner(), PLAYER_NR);
    TS_ASSERT_EQUALS(a.getPlayerFilterSet(), game::ShipQuery().getPlayerFilterSet());  // unmodified default
    TS_ASSERT_EQUALS(a.getPlayerDisplaySet(), game::PlayerSet_t(PLAYER_NR));
    TS_ASSERT_EQUALS(a.getLevelFilterSet(), game::ShipQuery().getLevelFilterSet());    // unmodified default
    TS_ASSERT_EQUALS(a.getLevelDisplaySet(), game::ExperienceLevelSet_t(0));           // unmodified default
}

