/**
  *  \file test/game/shipquerytest.cpp
  *  \brief Test for game::ShipQuery
  */

#include "game/shipquery.hpp"
#include "afl/test/testrunner.hpp"

/** Test initialisation, setters, getters.
    A: create ShipQuery. Use setters.
    E: expected initial state is set. Setters affect corresponding getters. */
AFL_TEST("game.ShipQuery:init", a)
{
    game::ShipQuery qa, qb;
    a.checkEqual("01. eq", qa == qb, true);
    a.checkEqual("02. ne", qa != qb, false);

    // All scalars empty
    a.checkEqual("11. getHullType",    qa.getHullType(), 0);
    a.checkEqual("12. getShipId",      qa.getShipId(), 0);
    a.checkEqual("13. getEngineType",  qa.getEngineType(), 0);
    a.checkEqual("14. getCombatMass",  qa.getCombatMass(), 0);
    a.checkEqual("15. getUsedESBRate", qa.getUsedESBRate(), 0);
    a.checkEqual("16. getCrew",        qa.getCrew(), 0);
    a.checkEqual("17. getOwner",       qa.getOwner(), 0);
    a.checkEqual("18. getDamage",      qa.getDamage(), 0);

    // Level filter: defaults to all
    a.checkEqual("21. getLevelFilterSet", qa.getLevelFilterSet().contains(1), true);
    a.checkEqual("22. getLevelFilterSet", qa.getLevelFilterSet().contains(0), true);

    // Level display: defaults to level 0
    a.checkEqual("31. getLevelDisplaySet", qa.getLevelDisplaySet().contains(1), false);
    a.checkEqual("32. getLevelDisplaySet", qa.getLevelDisplaySet().contains(0), true);

    // Player filter: defaults to all
    a.checkEqual("41. getPlayerFilterSet", qa.getPlayerFilterSet().contains(1), true);
    a.checkEqual("42. getPlayerFilterSet", qa.getPlayerFilterSet().contains(0), true);

    // Player display: defaults to none
    a.checkEqual("51. getPlayerDisplaySet", qa.getPlayerDisplaySet().contains(1), false);
    a.checkEqual("52. getPlayerDisplaySet", qa.getPlayerDisplaySet().contains(0), false);

    // Modify and check success
    game::PlayerSet_t ps1 = game::PlayerSet_t::fromInteger(2);
    game::PlayerSet_t ps2 = game::PlayerSet_t::fromInteger(5);
    game::ExperienceLevelSet_t ls1 = game::ExperienceLevelSet_t::fromInteger(7);
    game::ExperienceLevelSet_t ls2 = game::ExperienceLevelSet_t::fromInteger(9);

    qa.setHullType(42);
    qa.setShipId(150);
    qa.setEngineType(9);
    qa.setCombatMass(500, 20);
    qa.setCrew(92);
    qa.setOwner(5);
    qa.setPlayerFilterSet(ps1);
    qa.setPlayerDisplaySet(ps2);
    qa.setLevelFilterSet(ls1);
    qa.setLevelDisplaySet(ls2);
    qa.setDamage(12);

    a.checkEqual("61. getHullType",         qa.getHullType(), 42);
    a.checkEqual("62. getShipId",           qa.getShipId(), 150);
    a.checkEqual("63. getEngineType",       qa.getEngineType(), 9);
    a.checkEqual("64. getCombatMass",       qa.getCombatMass(), 500);
    a.checkEqual("65. getUsedESBRate",      qa.getUsedESBRate(), 20);
    a.checkEqual("66. getCrew",             qa.getCrew(), 92);
    a.checkEqual("67. getOwner",            qa.getOwner(), 5);
    a.checkEqual("68. getPlayerFilterSet",  qa.getPlayerFilterSet(), ps1);
    a.checkEqual("69. getPlayerDisplaySet", qa.getPlayerDisplaySet(), ps2);
    a.checkEqual("70. getLevelFilterSet",   qa.getLevelFilterSet(), ls1);
    a.checkEqual("71. getLevelDisplaySet",  qa.getLevelDisplaySet(), ls2);
    a.checkEqual("72. getDamage",           qa.getDamage(), 12);
    a.checkEqual("73. eq",                  qa == qb, false);
    a.checkEqual("74. ne",                  qa != qb, true);
}

/** Test initForExistingShip().
    QA: create universe, shiplist with qa ship. Call initForExistingShip().
    E: all attributes of the ship are taken over */
AFL_TEST("game.ShipQuery:initForExistingShip", a)
{
    const int SHIP_ID = 17;
    const int HULL_NR = 15;
    const int ENGINE_NR = 8;
    const int PLAYER_NR = 6;

    // Universe with qa ship
    game::map::Universe univ;
    game::map::Ship* sh = univ.ships().create(SHIP_ID);

    game::map::ShipData sd;
    sd.x = 1000;
    sd.y = 2000;
    sd.hullType = HULL_NR;
    sd.engineType = ENGINE_NR;
    sd.owner = PLAYER_NR;
    sd.damage = 7;
    sh->addCurrentShipData(sd, game::PlayerSet_t(PLAYER_NR));
    sh->internalCheck(game::PlayerSet_t(PLAYER_NR), 15);
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
    game::ShipQuery qa;
    qa.initForExistingShip(univ, SHIP_ID, shipList, config, scoreDefs);

    // Verify
    a.checkEqual("01. getHullType",         qa.getHullType(), HULL_NR);
    a.checkEqual("02. getShipId",           qa.getShipId(), SHIP_ID);
    a.checkEqual("03. getEngineType",       qa.getEngineType(), ENGINE_NR);
    a.checkEqual("04. getCombatMass",       qa.getCombatMass(), 600);
    a.checkEqual("05. getUsedESBRate",      qa.getUsedESBRate(), 25);
    a.checkEqual("06. getCrew",             qa.getCrew(), 99);
    a.checkEqual("07. getOwner",            qa.getOwner(), PLAYER_NR);
    a.checkEqual("08. getPlayerFilterSet",  qa.getPlayerFilterSet(), game::ShipQuery().getPlayerFilterSet());  // unmodified default
    a.checkEqual("09. getPlayerDisplaySet", qa.getPlayerDisplaySet(), game::PlayerSet_t(PLAYER_NR));
    a.checkEqual("10. getLevelFilterSet",   qa.getLevelFilterSet(), game::ShipQuery().getLevelFilterSet());    // unmodified default
    a.checkEqual("11. getLevelDisplaySet",  qa.getLevelDisplaySet(), game::ExperienceLevelSet_t(0));           // unmodified default
    a.checkEqual("12. getDamage",           qa.getDamage(), 7);
}
