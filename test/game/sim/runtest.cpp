/**
  *  \file test/game/sim/runtest.cpp
  *  \brief Test for game::sim::Run
  */

#include "game/sim/run.hpp"

#include "afl/test/testrunner.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/sim/configuration.hpp"
#include "game/sim/planet.hpp"
#include "game/sim/result.hpp"
#include "game/sim/setup.hpp"
#include "game/sim/ship.hpp"
#include "game/spec/engine.hpp"
#include "game/spec/hull.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/shiplist.hpp"
#include "game/vcr/battle.hpp"
#include "game/vcr/object.hpp"
#include "game/vcr/statistic.hpp"
#include "util/randomnumbergenerator.hpp"

using game::sim::Ship;
using game::sim::Planet;

namespace {
    void initShipList(game::spec::ShipList& list)
    {
        game::test::initStandardBeams(list);
        game::test::initStandardTorpedoes(list);
        game::test::addOutrider(list);
        game::test::addAnnihilation(list);
        game::test::addGorbie(list);
        game::test::addNovaDrive(list);
        game::test::addTranswarp(list);
    }

    void setDeterministicConfig(game::sim::Configuration& opts, const game::config::HostConfiguration& config, game::sim::Configuration::VcrMode mode, game::sim::Configuration::BalancingMode balance)
    {
        opts.setMode(mode, 0, config);
        opts.setEngineShieldBonus(0);
        opts.setScottyBonus(true);
        opts.setRandomLeftRight(false);
        opts.setHonorAlliances(true);
        opts.setOnlyOneSimulation(true);
        opts.setSeedControl(true);
        opts.setRandomizeFCodesOnEveryFight(false);
        opts.setBalancingMode(balance);
    }

    Ship* addShip(game::sim::Setup& setup, int hullNr, int id, int owner, const game::spec::ShipList& list)
    {
        Ship* ship = setup.addShip();
        ship->setId(id);
        ship->setFriendlyCode("???");
        ship->setDamage(0);
        ship->setShield(100);
        ship->setOwner(owner);
        ship->setExperienceLevel(0);
        ship->setFlags(0);
        ship->setHullType(hullNr, list);     // sets crew, mass, hullType, numBeams, beamType, numLaunchers, torpedoType, numBays, ammo
        ship->setEngineType(9);
        ship->setAggressiveness(Ship::agg_Kill);
        ship->setInterceptId(0);
        return ship;
    }

    Ship* addOutrider(afl::test::Assert a, game::sim::Setup& setup, int id, int owner, const game::spec::ShipList& list)
    {
        Ship* ship = addShip(setup, 1, id, owner, list);
        a.checkEqual("addOutrider > getCrew", ship->getCrew(), 180);  // verify that setHullType worked as planned
        return ship;
    }

    Ship* addGorbie(afl::test::Assert a, game::sim::Setup& setup, int id, int owner, const game::spec::ShipList& list)
    {
        Ship* ship = addShip(setup, 70, id, owner, list);
        a.checkEqual("addGorbie > getCrew", ship->getCrew(), 2287);
        return ship;
    }

    Ship* addAnnihilation(afl::test::Assert a, game::sim::Setup& setup, int id, int owner, const game::spec::ShipList& list)
    {
        Ship* ship = addShip(setup, 53, id, owner, list);
        a.checkEqual("addAnnihilation > getCrew", ship->getCrew(), 2910);
        return ship;
    }

    Planet* addPlanet(game::sim::Setup& setup, int id, int owner)
    {
        Planet* p = setup.addPlanet();
        p->setId(id);
        p->setFriendlyCode("???");
        p->setDamage(0);
        p->setShield(100);
        p->setOwner(owner);
        p->setExperienceLevel(0);
        p->setFlags(0);
        p->setDefense(61);
        p->setBaseDefense(30);
        p->setBaseBeamTech(5);
        p->setBaseTorpedoTech(1);
        p->setNumBaseFighters(12);
        return p;
    }

    void addPlanetSetup(game::sim::Setup& setup)
    {
        // Add ship setup for order tests.
        // Reference: test.sim from bug #428
        Ship* a = setup.addShip();
        a->setId(450);
        a->setFriendlyCode("010");
        a->setDamage(0);
        a->setShield(100);
        a->setOwner(8);
        a->setHullTypeOnly(0);
        a->setCrew(2287);
        a->setMass(980);
        a->setBeamType(6);
        a->setNumBeams(10);
        a->setNumLaunchers(0);
        a->setNumBays(10);
        a->setAmmo(250);
        a->setEngineType(9);
        a->setAggressiveness(Ship::agg_Kill);

        Ship* b = setup.addShip();
        b->setId(455);
        b->setFriendlyCode("020");
        b->setDamage(0);
        b->setShield(100);
        b->setOwner(9);
        b->setHullTypeOnly(0);
        b->setCrew(1958);
        b->setMass(850);
        b->setBeamType(10);
        b->setNumBeams(6);
        b->setNumLaunchers(0);
        b->setNumBays(10);
        b->setAmmo(10);
        b->setEngineType(9);
        b->setAggressiveness(Ship::agg_Kill);

        Planet* p = setup.addPlanet();
        p->setId(230);
        p->setFriendlyCode("000");
        p->setOwner(8);
        p->setDefense(351);
        p->setBaseBeamTech(10);
        p->setNumBaseFighters(50);
        p->setBaseDefense(200);
        p->setBaseTorpedoTech(10);
    }

    void addShipSetup(game::sim::Setup& setup)
    {
        // Add ship setup for order tests.
        // Reference: test2.sim from bug #428
        Ship* a = setup.addShip();
        a->setId(384);
        a->setFriendlyCode("&'K");
        a->setDamage(0);
        a->setShield(100);
        a->setOwner(8);
        a->setHullTypeOnly(0);
        a->setCrew(1);
        a->setMass(24);
        a->setBeamType(6);
        a->setNumBeams(1);
        a->setNumLaunchers(0);
        a->setNumBays(0);
        a->setAmmo(0);
        a->setEngineType(9);
        a->setAggressiveness(Ship::agg_Kill);

        Ship* b = setup.addShip();
        b->setId(489);
        b->setFriendlyCode("'d;");
        b->setDamage(81);
        b->setShield(19);
        b->setOwner(10);
        b->setHullTypeOnly(0);
        b->setCrew(430);
        b->setMass(160);
        b->setBeamType(9);
        b->setNumBeams(4);
        b->setNumLaunchers(3);
        b->setTorpedoType(10);
        b->setNumBays(0);
        b->setAmmo(0);
        b->setEngineType(9);
        b->setAggressiveness(Ship::agg_Kill);

        Ship* c = setup.addShip();
        c->setId(320);
        c->setFriendlyCode("001");
        c->setDamage(0);
        c->setShield(100);
        c->setOwner(9);
        c->setHullTypeOnly(0);
        c->setCrew(102);
        c->setMass(130);
        c->setBeamType(0);
        c->setNumBeams(0);
        c->setNumLaunchers(0);
        c->setTorpedoType(0);
        c->setNumBays(0);
        c->setAmmo(0);
        c->setEngineType(9);
        c->setAggressiveness(Ship::agg_Passive);
    }

    /* Common environment */
    struct TestHarness {
        game::spec::ShipList list;
        game::config::HostConfiguration config;
        game::vcr::flak::Configuration flakConfiguration;
        util::RandomNumberGenerator rng;
        std::vector<game::vcr::Statistic> stats;
        game::sim::Configuration opts;
        game::sim::Setup setup;
        game::sim::Result result;

        TestHarness()
            : list(), config(), flakConfiguration(), rng(42)
            { initShipList(list); }
    };
}

/** Test basic Host simulation.
    A: prepare two ships, Host simulation.
    E: expected results and metadata produced (verified against PCC2 playvcr). */
AFL_TEST("game.sim.Run:VcrHost", a)
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    Ship* s1 = addOutrider(a, h.setup, 1, 12, h.list);
    Ship* s2 = addOutrider(a, h.setup, 2, 11, h.list);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result
    // - rng has not been touched because we use seed control
    a.checkEqual("01. getSeed", h.rng.getSeed(), 42U);

    // - a battle has been created
    a.checkNonNull("11. battles",           h.result.battles.get());
    a.checkEqual("12. getNumBattles",       h.result.battles->getNumBattles(), 1U);
    a.checkEqual("13. this_battle_weight",  h.result.this_battle_weight, 1);
    a.checkEqual("14. total_battle_weight", h.result.total_battle_weight, 1);
    a.checkEqual("15. series_length",       h.result.series_length, 110);
    a.checkEqual("16. this_battle_index",   h.result.this_battle_index, 0);

    // - statistics
    a.checkEqual("21. stats", h.stats.size(), 2U);

    // - ship 1
    a.checkEqual("31. getDamage", s1->getDamage(), 107);
    a.checkEqual("32. getShield", s1->getShield(), 0);
    a.checkEqual("33. getCrew",   s1->getCrew(), 103);
    a.checkEqual("34. getOwner",  s1->getOwner(), 0);

    // - ship 2
    a.checkEqual("41. getDamage", s2->getDamage(), 82);
    a.checkEqual("42. getShield", s2->getShield(), 0);
    a.checkEqual("43. getCrew",   s2->getCrew(), 121);
    a.checkEqual("44. getOwner",  s2->getOwner(), 11);
}

/** Test basic Host simulation, big ships.
    A: prepare two ships, Host simulation.
    E: expected results and metadata produced (verified against PCC2 playvcr). */
AFL_TEST("game.sim.Run:VcrHost:big", a)
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    Ship* s1 = addGorbie(a, h.setup, 1, 8, h.list);
    Ship* s2 = addAnnihilation(a, h.setup, 2, 6, h.list);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result
    // - rng has not been touched because we use seed control
    a.checkEqual("01. getSeed", h.rng.getSeed(), 42U);

    // - a battle has been created
    a.checkNonNull("11. battles",           h.result.battles.get());
    a.checkEqual("12. getNumBattles",       h.result.battles->getNumBattles(), 1U);
    a.checkEqual("13. this_battle_weight",  h.result.this_battle_weight, 1);
    a.checkEqual("14. total_battle_weight", h.result.total_battle_weight, 1);
    a.checkEqual("15. series_length",       h.result.series_length, 110);
    a.checkEqual("16. this_battle_index",   h.result.this_battle_index, 0);

    // - statistics
    a.checkEqual("21. stats",                h.stats.size(), 2U);
    a.checkEqual("22. getMinFightersAboard", h.stats[0].getMinFightersAboard(), 201);
    a.checkEqual("23. getNumTorpedoHits",    h.stats[0].getNumTorpedoHits(), 0);
    a.checkEqual("24. getMinFightersAboard", h.stats[1].getMinFightersAboard(), 0);
    a.checkEqual("25. getNumTorpedoHits",    h.stats[1].getNumTorpedoHits(), 29);

    // - ship 1
    a.checkEqual("31. getDamage", s1->getDamage(), 38);
    a.checkEqual("32. getShield", s1->getShield(), 0);
    a.checkEqual("33. getCrew",   s1->getCrew(), 2173);
    a.checkEqual("34. getOwner",  s1->getOwner(), 8);

    // - ship 2
    a.checkEqual("41. getDamage", s2->getDamage(), 102);
    a.checkEqual("42. getShield", s2->getShield(), 0);
    a.checkEqual("43. getCrew",   s2->getCrew(), 2880);
    a.checkEqual("44. getOwner",  s2->getOwner(), 0);
}

/** Test basic Host simulation, NTP.
    A: prepare two ships, Host simulation, one with NTP.
    E: expected results and metadata produced (verified against PCC2 playvcr). */
AFL_TEST("game.sim.Run:VcrHost:NTP", a)
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    Ship* s1 = addAnnihilation(a, h.setup, 1, 6, h.list);
    Ship* s2 = addAnnihilation(a, h.setup, 2, 2, h.list);
    s2->setFriendlyCode("NTP");
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result
    // - rng has not been touched because we use seed control
    a.checkEqual("01. getSeed", h.rng.getSeed(), 42U);

    // - a battle has been created
    a.checkNonNull("11. battles", h.result.battles.get());
    a.checkEqual("12. getNumBattles",       h.result.battles->getNumBattles(), 1U);
    a.checkEqual("13. getId",               h.result.battles->getBattle(0)->getObject(0, false)->getId(), 2);
    a.checkEqual("14. getNumBattles",       h.result.battles->getBattle(0)->getObject(0, false)->getNumTorpedoes(), 0);
    a.checkEqual("15. getId",               h.result.battles->getBattle(0)->getObject(1, false)->getId(), 1);
    a.checkEqual("16. getNumTorpedoes",     h.result.battles->getBattle(0)->getObject(1, false)->getNumTorpedoes(), 320);
    a.checkEqual("17. this_battle_weight",  h.result.this_battle_weight, 1);
    a.checkEqual("18. total_battle_weight", h.result.total_battle_weight, 1);
    a.checkEqual("19. series_length",       h.result.series_length, 110);
    a.checkEqual("20. this_battle_index",   h.result.this_battle_index, 0);

    // - statistics
    a.checkEqual("21. stats",                h.stats.size(), 2U);
    a.checkEqual("22. getMinFightersAboard", h.stats[0].getMinFightersAboard(), 0);
    a.checkEqual("23. getNumTorpedoHits",    h.stats[0].getNumTorpedoHits(), 72);
    a.checkEqual("24. getMinFightersAboard", h.stats[1].getMinFightersAboard(), 0);
    a.checkEqual("25. getNumTorpedoHits",    h.stats[1].getNumTorpedoHits(), 0);

    // - ship 1
    a.checkEqual("31. getDamage", s1->getDamage(), 2);
    a.checkEqual("32. getShield", s1->getShield(), 0);
    a.checkEqual("33. getCrew",   s1->getCrew(), 2907);
    a.checkEqual("34. getOwner",  s1->getOwner(), 6);

    // - ship 2
    a.checkEqual("41. getDamage", s2->getDamage(), 153);
    a.checkEqual("42. getShield", s2->getShield(), 0);
    a.checkEqual("43. getCrew",   s2->getCrew(), 2483);
    a.checkEqual("44. getOwner",  s2->getOwner(), 0);
}

/** Test Host simulation, balancing mode "360 kt".
    A: prepare two ships, Host simulation.
    E: expected results and metadata produced (verified against PCC2 playvcr). */
AFL_TEST("game.sim.Run:VcrHost:Balance360k", a)
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::Balance360k);

    // Setup
    Ship* s1 = addOutrider(a, h.setup, 1, 12, h.list);
    Ship* s2 = addOutrider(a, h.setup, 2, 11, h.list);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result
    // - rng has not been touched because we use seed control
    a.checkEqual("01. getSeed", h.rng.getSeed(), 42U);

    // - a battle has been created - increased weight due to balancing
    a.checkNonNull("11. battles",           h.result.battles.get());
    a.checkEqual("12. getNumBattles",       h.result.battles->getNumBattles(), 1U);
    a.checkEqual("13. this_battle_weight",  h.result.this_battle_weight, 50);
    a.checkEqual("14. total_battle_weight", h.result.total_battle_weight, 100);
    a.checkEqual("15. series_length",       h.result.series_length, 220);             // doubled by Balance360k
    a.checkEqual("16. this_battle_index",   h.result.this_battle_index, 0);

    // - statistics
    a.checkEqual("21. stats", h.stats.size(), 2U);

    // - ship 1
    a.checkEqual("31. getDamage", s1->getDamage(), 107);
    a.checkEqual("32. getShield", s1->getShield(), 0);
    a.checkEqual("33. getCrew",   s1->getCrew(), 103);
    a.checkEqual("34. getOwner",  s1->getOwner(), 0);

    // - ship 2
    a.checkEqual("41. getDamage", s2->getDamage(), 82);
    a.checkEqual("42. getShield", s2->getShield(), 0);
    a.checkEqual("43. getCrew",   s2->getCrew(), 121);
    a.checkEqual("44. getOwner",  s2->getOwner(), 11);
}

/** Test Host simulation, balancing mode "Master at Arms".
    A: prepare two ships, Host simulation.
    E: expected results and metadata produced (verified against PCC2 playvcr). */
AFL_TEST("game.sim.Run:VcrHost:BalanceMasterAtArms", a)
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceMasterAtArms);

    // Setup
    Ship* s1 = addGorbie(a, h.setup, 1, 8, h.list);
    Ship* s2 = addGorbie(a, h.setup, 2, 6, h.list);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result
    // - rng has not been touched because we use seed control
    a.checkEqual("01. getSeed", h.rng.getSeed(), 42U);

    // - a battle has been created - increased weight due to balancing
    a.checkNonNull("11. battles",           h.result.battles.get());
    a.checkEqual("12. getNumBattles",       h.result.battles->getNumBattles(), 1U);
    a.checkEqual("13. this_battle_weight",  h.result.this_battle_weight, 28);
    a.checkEqual("14. total_battle_weight", h.result.total_battle_weight, 1000);
    a.checkEqual("15. series_length",       h.result.series_length, 440);             // doubled by bonus bays and by bonus fighters
    a.checkEqual("16. this_battle_index",   h.result.this_battle_index, 0);

    // - statistics
    a.checkEqual("21. stats",                h.stats.size(), 2U);
    a.checkEqual("22. getMinFightersAboard", h.stats[0].getMinFightersAboard(), 146);
    a.checkEqual("23. getNumTorpedoHits",    h.stats[0].getNumTorpedoHits(), 0);
    a.checkEqual("24. getMinFightersAboard", h.stats[1].getMinFightersAboard(), 167);
    a.checkEqual("25. getNumTorpedoHits",    h.stats[1].getNumTorpedoHits(), 0);

    // - ship 1
    a.checkEqual("31. getDamage", s1->getDamage(), 102);
    a.checkEqual("32. getShield", s1->getShield(), 0);
    a.checkEqual("33. getCrew",   s1->getCrew(), 2287);
    a.checkEqual("34. getOwner",  s1->getOwner(), 0);
    a.checkEqual("35. getAmmo",              s1->getAmmo(), 151);

    // - ship 2
    a.checkEqual("41. getDamage", s2->getDamage(), 0);
    a.checkEqual("42. getShield", s2->getShield(), 50);
    a.checkEqual("43. getCrew",   s2->getCrew(), 2287);
    a.checkEqual("44. getOwner",  s2->getOwner(), 6);
    a.checkEqual("45. getAmmo",              s2->getAmmo(), 175);
}

/** Test Host simulation, planet.
    A: prepare ships and planet, Host simulation.
    E: expected results and metadata produced (verified against PCC2 playvcr). */
AFL_TEST("game.sim.Run:VcrHost:planet", a)
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    Ship* s = addOutrider(a, h.setup, 1, 5, h.list);
    Planet* p = addPlanet(h.setup, 1, 4);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result
    // - rng has not been touched because we use seed control
    a.checkEqual("01. getSeed", h.rng.getSeed(), 42U);

    // - a battle has been created
    a.checkNonNull("11. battles",           h.result.battles.get());
    a.checkEqual("12. getNumBattles",       h.result.battles->getNumBattles(), 1U);
    a.checkEqual("13. this_battle_weight",  h.result.this_battle_weight, 1);
    a.checkEqual("14. total_battle_weight", h.result.total_battle_weight, 1);
    a.checkEqual("15. series_length",       h.result.series_length, 110);
    a.checkEqual("16. this_battle_index",   h.result.this_battle_index, 0);

    // - statistics
    a.checkEqual("21. stats", h.stats.size(), 2U);

    // - ship 1
    a.checkEqual("31. getDamage", s->getDamage(), 103);
    a.checkEqual("32. getShield", s->getShield(), 0);
    a.checkEqual("33. getCrew",   s->getCrew(), 128);
    a.checkEqual("34. getOwner",  s->getOwner(), 0);

    // - ship 2
    a.checkEqual("41. getDamage", p->getDamage(), 0);
    a.checkEqual("42. getShield", p->getShield(), 100);
    a.checkEqual("43. getOwner",  p->getOwner(), 4);
}

/** Test Host simulation, intercept-attack.
    A: prepare four ships, with two of them intercepting one, Host simulation.
    E: expected results and metadata produced (verified against PCC2 playvcr). */
AFL_TEST("game.sim.Run:VcrHost:intercept", a)
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    /*Ship* s1 =*/ addOutrider(a, h.setup, 1, 1, h.list);
    Ship* s2 = addOutrider(a, h.setup, 2, 2, h.list);
    Ship* s3 = addOutrider(a, h.setup, 3, 3, h.list);
    Ship* s4 = addOutrider(a, h.setup, 4, 4, h.list);
    s3->setAggressiveness(2);
    s3->setInterceptId(2);
    s3->setFriendlyCode("200");
    s4->setAggressiveness(2);
    s4->setInterceptId(2);
    s4->setFriendlyCode("100");

    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result
    // - rng has not been touched because we use seed control
    a.checkEqual("01. getSeed", h.rng.getSeed(), 42U);

    // - a battle has been created
    a.checkNonNull("11. battles",           h.result.battles.get());
    a.checkEqual("12. getId",               h.result.battles->getBattle(0)->getObject(0, false)->getId(), 2);
    a.checkEqual("13. getId",               h.result.battles->getBattle(0)->getObject(1, false)->getId(), 4);
    a.checkEqual("14. getNumBattles",       h.result.battles->getNumBattles(), 1U);
    a.checkEqual("15. this_battle_weight",  h.result.this_battle_weight, 1);
    a.checkEqual("16. total_battle_weight", h.result.total_battle_weight, 1);
    a.checkEqual("17. series_length",       h.result.series_length, 110);
    a.checkEqual("18. this_battle_index",   h.result.this_battle_index, 0);

    // - statistics
    a.checkEqual("21. stats", h.stats.size(), 4U);

    // - ship 2
    a.checkEqual("31. getDamage", s2->getDamage(), 82);
    a.checkEqual("32. getShield", s2->getShield(), 0);
    a.checkEqual("33. getCrew",   s2->getCrew(), 121);
    a.checkEqual("34. getOwner",  s2->getOwner(), 2);

    // - ship 4
    a.checkEqual("41. getDamage", s4->getDamage(), 107);
    a.checkEqual("42. getShield", s4->getShield(), 0);
    a.checkEqual("43. getCrew",   s4->getCrew(), 103);
    a.checkEqual("44. getOwner",  s4->getOwner(), 0);
}

/** Test multi-ship Host simulation.
    A: prepare multiple ships, Host simulation.
    E: expected results and metadata produced. Expected battle order produced. This is a regression test to ensure constant behaviour. */
AFL_TEST("game.sim.Run:VcrHost:multi-ship", a)
{
    // Environment
    TestHarness h;
    h.opts.setMode(game::sim::Configuration::VcrHost, 0, h.config);

    // Setup
    Ship* s1 = addOutrider(a, h.setup, 1, 1, h.list);
    Ship* s2 = addOutrider(a, h.setup, 2, 2, h.list);
    Ship* s3 = addOutrider(a, h.setup, 3, 2, h.list);
    Ship* s4 = addOutrider(a, h.setup, 4, 2, h.list);
    Planet* p = addPlanet(h.setup, 17, 1);
    s1->setFriendlyCode("-20");
    s2->setFriendlyCode("100");
    s3->setFriendlyCode("300");
    s4->setFriendlyCode("200");
    p->setFriendlyCode("ATT");

    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result
    // - rng has been used
    a.checkEqual("01. getSeed", h.rng.getSeed(), 673767206U);

    // - battles have been created; series length unchanged
    a.checkNonNull("11. battles",           h.result.battles.get());
    a.checkEqual("12. getNumBattles",       h.result.battles->getNumBattles(), 4U);
    a.checkEqual("13. this_battle_weight",  h.result.this_battle_weight, 1);
    a.checkEqual("14. total_battle_weight", h.result.total_battle_weight, 1);
    a.checkEqual("15. series_length",       h.result.series_length, 110);
    a.checkEqual("16. this_battle_index",   h.result.this_battle_index, 0);

    // - first battle (#2 is aggressor, #1 wins)
    a.checkEqual("21. getId", h.result.battles->getBattle(0)->getObject(0, false)->getId(), 1);
    a.checkEqual("22. getId", h.result.battles->getBattle(0)->getObject(1, false)->getId(), 2);

    // - second battle (#4 is aggressor, #4 wins)
    a.checkEqual("31. getId", h.result.battles->getBattle(1)->getObject(0, false)->getId(), 1);
    a.checkEqual("32. getId", h.result.battles->getBattle(1)->getObject(1, false)->getId(), 4);

    // - third battle (#4 is aggressor, #17 wins)
    a.checkEqual("41. getId", h.result.battles->getBattle(2)->getObject(0, false)->getId(), 4);
    a.checkEqual("42. getId", h.result.battles->getBattle(2)->getObject(1, false)->getId(), 17);

    // - fourth battle (#3 is aggressor, #17 wins)
    a.checkEqual("51. getId", h.result.battles->getBattle(3)->getObject(0, false)->getId(), 3);
    a.checkEqual("52. getId", h.result.battles->getBattle(3)->getObject(1, false)->getId(), 17);

    // - statistics
    a.checkEqual("61. stats", h.stats.size(), 5U);

    // - ship 1
    a.checkEqual("71. getDamage", s1->getDamage(), 110);
    a.checkEqual("72. getShield", s1->getShield(), 0);
    a.checkEqual("73. getCrew",   s1->getCrew(), 47);
    a.checkEqual("74. getOwner",  s1->getOwner(), 0);

    // - ship 2
    a.checkEqual("81. getDamage", s2->getDamage(), 162);
    a.checkEqual("82. getShield", s2->getShield(), 0);
    a.checkEqual("83. getCrew",   s2->getCrew(), 65);
    a.checkEqual("84. getOwner",  s2->getOwner(), 0);

    // - ship 3
    a.checkEqual("91. getDamage", s3->getDamage(), 159);
    a.checkEqual("92. getShield", s3->getShield(), 0);
    a.checkEqual("93. getCrew",   s3->getCrew(), 100);
    a.checkEqual("94. getOwner",  s3->getOwner(), 0);

    // - ship 4
    a.checkEqual("101. getDamage", s4->getDamage(), 168);
    a.checkEqual("102. getShield", s4->getShield(), 0);
    a.checkEqual("103. getCrew",   s4->getCrew(), 73);
    a.checkEqual("104. getOwner",  s4->getOwner(), 0);

    // - planet
    a.checkEqual("111. getDamage", p->getDamage(), 0);
    a.checkEqual("112. getShield", p->getShield(), 100);
    a.checkEqual("113. getOwner",  p->getOwner(), 1);
}

/** Test Host simulation with Engine/Shield bonus.
    A: prepare two ships with different engines, Host simulation, ESB 20%.
    E: expected results and metadata produced (verified against PCC2 playvcr). */
AFL_TEST("game.sim.Run:VcrHost:esb", a)
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);
    h.opts.setEngineShieldBonus(20);

    // Setup
    Ship* s1 = addOutrider(a, h.setup, 1, 6, h.list);
    Ship* s2 = addOutrider(a, h.setup, 2, 9, h.list);
    h.result.init(h.opts, 0);
    s1->setEngineType(5);          // Nova Drive 5, 5 kt bonus
    s2->setEngineType(9);          // Transwarp Drive, 60 kt bonus

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result
    // - rng has not been touched because we use seed control
    a.checkEqual("01. getSeed", h.rng.getSeed(), 42U);

    // - a battle has been created
    a.checkNonNull("11. battles",           h.result.battles.get());
    a.checkEqual("12. getNumBattles",       h.result.battles->getNumBattles(), 1U);
    a.checkEqual("13. getOwner",            h.result.battles->getBattle(0)->getObject(0, false)->getOwner(), 9);
    a.checkEqual("14. getMass",             h.result.battles->getBattle(0)->getObject(0, false)->getMass(), 135);
    a.checkEqual("15. getOwner",            h.result.battles->getBattle(0)->getObject(1, false)->getOwner(), 6);
    a.checkEqual("16. getMass",             h.result.battles->getBattle(0)->getObject(1, false)->getMass(), 80);
    a.checkEqual("17. this_battle_weight",  h.result.this_battle_weight, 1);
    a.checkEqual("18. total_battle_weight", h.result.total_battle_weight, 1);
    a.checkEqual("19. series_length",       h.result.series_length, 110);
    a.checkEqual("20. this_battle_index",   h.result.this_battle_index, 0);

    // - statistics
    a.checkEqual("21. stats", h.stats.size(), 2U);

    // - ship 1
    a.checkEqual("31. getDamage", s1->getDamage(), 119);
    a.checkEqual("32. getShield", s1->getShield(), 0);
    a.checkEqual("33. getCrew",   s1->getCrew(), 89);
    a.checkEqual("34. getOwner",  s1->getOwner(), 0);

    // - ship 2
    a.checkEqual("41. getDamage", s2->getDamage(), 12);
    a.checkEqual("42. getShield", s2->getShield(), 0);
    a.checkEqual("43. getCrew",   s2->getCrew(), 158);
    a.checkEqual("44. getOwner",  s2->getOwner(), 9);
}

/** Test basic PHost simulation.
    A: prepare two ships, PHost simulation.
    E: expected results and metadata produced (verified against PCC2 playvcr). */
AFL_TEST("game.sim.Run:VcrPHost4", a)
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrPHost4, game::sim::Configuration::BalanceNone);
    h.opts.setRandomLeftRight(true);

    // Setup
    Ship* s1 = addOutrider(a, h.setup, 1, 12, h.list);
    Ship* s2 = addOutrider(a, h.setup, 2, 11, h.list);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result
    // - rng has not been touched because we use seed control
    a.checkEqual("01. getSeed", h.rng.getSeed(), 42U);

    // - a battle has been created
    a.checkNonNull("11. battles",     h.result.battles.get());
    a.checkEqual("12. getNumBattles", h.result.battles->getNumBattles(), 1U);
    a.checkEqual("13. getOwner",      h.result.battles->getBattle(0)->getObject(0, false)->getOwner(), 12);
    a.checkEqual("14. getOwner",      h.result.battles->getBattle(0)->getObject(1, false)->getOwner(), 11);

    a.checkEqual("21. this_battle_weight",  h.result.this_battle_weight, 1);
    a.checkEqual("22. total_battle_weight", h.result.total_battle_weight, 1);
    a.checkEqual("23. series_length",       h.result.series_length, 220);          // doubled by random left/right
    a.checkEqual("24. this_battle_index",   h.result.this_battle_index, 0);

    // - statistics
    a.checkEqual("31. stats", h.stats.size(), 2U);

    // - ship 1
    a.checkEqual("41. getDamage", s1->getDamage(), 100);
    a.checkEqual("42. getShield", s1->getShield(), 0);
    a.checkEqual("43. getCrew",   s1->getCrew(), 132);
    a.checkEqual("44. getOwner",  s1->getOwner(), 0);

    // - ship 2
    a.checkEqual("51. getDamage", s2->getDamage(), 70);
    a.checkEqual("52. getShield", s2->getShield(), 0);
    a.checkEqual("53. getCrew",   s2->getCrew(), 132);
    a.checkEqual("54. getOwner",  s2->getOwner(), 11);
}

/** Test basic Host simulation, big ships.
    A: prepare two ships, PHost simulation.
    E: expected results and metadata produced (verified against PCC2 playvcr). */
AFL_TEST("game.sim.Run:VcrPHost3:big", a)
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrPHost3, game::sim::Configuration::BalanceNone);

    // Setup
    Ship* s1 = addGorbie(a, h.setup, 1, 8, h.list);
    Ship* s2 = addAnnihilation(a, h.setup, 2, 6, h.list);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result
    // - rng has not been touched because we use seed control
    a.checkEqual("01. getSeed", h.rng.getSeed(), 42U);

    // - a battle has been created
    a.checkNonNull("11. battles",           h.result.battles.get());
    a.checkEqual("12. getNumBattles",       h.result.battles->getNumBattles(), 1U);
    a.checkEqual("13. this_battle_weight",  h.result.this_battle_weight, 1);
    a.checkEqual("14. total_battle_weight", h.result.total_battle_weight, 1);
    a.checkEqual("15. series_length",       h.result.series_length, 110);
    a.checkEqual("16. this_battle_index",   h.result.this_battle_index, 0);

    // - statistics
    a.checkEqual("21. stats",                h.stats.size(), 2U);
    a.checkEqual("22. getMinFightersAboard", h.stats[0].getMinFightersAboard(), 210);
    a.checkEqual("23. getNumTorpedoHits",    h.stats[0].getNumTorpedoHits(), 0);
    a.checkEqual("24. getMinFightersAboard", h.stats[1].getMinFightersAboard(), 0);
    a.checkEqual("25. getNumTorpedoHits",    h.stats[1].getNumTorpedoHits(), 29);

    // - ship 1
    a.checkEqual("31. getDamage", s1->getDamage(), 38);
    a.checkEqual("32. getShield", s1->getShield(), 0);
    a.checkEqual("33. getCrew",   s1->getCrew(), 2173);
    a.checkEqual("34. getOwner",  s1->getOwner(), 8);

    // - ship 2
    a.checkEqual("41. getDamage", s2->getDamage(), 100);
    a.checkEqual("42. getShield", s2->getShield(), 0);
    a.checkEqual("43. getCrew",   s2->getCrew(), 2902);
    a.checkEqual("44. getOwner",  s2->getOwner(), 0);
}


/** Test PHost simulation, planet.
    A: prepare ships and planet, PHost simulation.
    E: expected results and metadata produced (verified against PCC2 playvcr). */
AFL_TEST("game.sim.Run:VcrPHost4:planet", a)
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrPHost4, game::sim::Configuration::BalanceNone);

    // Setup
    Ship* s = addOutrider(a, h.setup, 1, 5, h.list);
    Planet* p = addPlanet(h.setup, 1, 4);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result
    // - rng has not been touched because we use seed control
    a.checkEqual("01. getSeed", h.rng.getSeed(), 42U);

    // - a battle has been created
    a.checkNonNull("11. battles",           h.result.battles.get());
    a.checkEqual("12. getNumBattles",       h.result.battles->getNumBattles(), 1U);
    a.checkEqual("13. this_battle_weight",  h.result.this_battle_weight, 1);
    a.checkEqual("14. total_battle_weight", h.result.total_battle_weight, 1);
    a.checkEqual("15. series_length",       h.result.series_length, 110);
    a.checkEqual("16. this_battle_index",   h.result.this_battle_index, 0);

    // - statistics
    a.checkEqual("21. stats",                h.stats.size(), 2U);
    a.checkEqual("22. getMinFightersAboard", h.stats[0].getMinFightersAboard(), 0);
    a.checkEqual("23. getNumTorpedoHits",    h.stats[0].getNumTorpedoHits(), 0);
    a.checkEqual("24. getMinFightersAboard", h.stats[1].getMinFightersAboard(), 0);
    a.checkEqual("25. getNumTorpedoHits",    h.stats[1].getNumTorpedoHits(), 0);

    // - ship 1
    a.checkEqual("31. getDamage", s->getDamage(), 100);
    a.checkEqual("32. getShield", s->getShield(), 0);
    a.checkEqual("33. getCrew",   s->getCrew(), 131);
    a.checkEqual("34. getOwner",  s->getOwner(), 0);

    // - ship 2
    a.checkEqual("41. getDamage", p->getDamage(), 0);
    a.checkEqual("42. getShield", p->getShield(), 100);
    a.checkEqual("43. getOwner",  p->getOwner(), 4);
}

/** Test PHost simulation, planet with torpedo tubes.
    A: prepare ships and planet, set PlanetsHaveTubes=Yes, PHost simulation.
    E: expected results and metadata produced (verified against PCC2 playvcr). */
AFL_TEST("game.sim.Run:VcrPHost4:PlanetsHaveTubes", a)
{
    // Environment
    TestHarness h;
    h.config[game::config::HostConfiguration::PlanetsHaveTubes].set(true);
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrPHost4, game::sim::Configuration::BalanceNone);

    // Setup
    Ship* s = addAnnihilation(a, h.setup, 1, 6, h.list);
    Planet* p = addPlanet(h.setup, 1, 9);
    p->setDefense(61);
    p->setBaseDefense(200);
    p->setBaseBeamTech(7);
    p->setBaseTorpedoTech(4);
    p->setNumBaseFighters(40);
    p->setNumBaseTorpedoes(5, 20);
    p->setNumBaseTorpedoes(6, 30);

    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result
    // - rng has not been touched because we use seed control
    a.checkEqual("01. getSeed", h.rng.getSeed(), 42U);

    // - a battle has been created
    a.checkNonNull("11. battles",           h.result.battles.get());
    a.checkEqual("12. getNumTorpedoes",     h.result.battles->getBattle(0)->getObject(0, false)->getNumTorpedoes(), 320);
    a.checkEqual("13. getNumFighters",      h.result.battles->getBattle(0)->getObject(0, false)->getNumFighters(), 0);
    a.checkEqual("14. getNumTorpedoes",     h.result.battles->getBattle(0)->getObject(1, false)->getNumTorpedoes(), 72);
    a.checkEqual("15. getNumFighters",      h.result.battles->getBattle(0)->getObject(1, false)->getNumFighters(), 48);
    a.checkEqual("16. getNumBattles",       h.result.battles->getNumBattles(), 1U);
    a.checkEqual("17. this_battle_weight",  h.result.this_battle_weight, 1);
    a.checkEqual("18. total_battle_weight", h.result.total_battle_weight, 1);
    a.checkEqual("19. series_length",       h.result.series_length, 110);
    a.checkEqual("20. this_battle_index",   h.result.this_battle_index, 0);

    // - statistics
    a.checkEqual("21. stats",                h.stats.size(), 2U);
    a.checkEqual("22. getMinFightersAboard", h.stats[0].getMinFightersAboard(), 0);
    a.checkEqual("23. getNumTorpedoHits",    h.stats[0].getNumTorpedoHits(), 16);
    a.checkEqual("24. getMinFightersAboard", h.stats[1].getMinFightersAboard(), 0);
    // FIXME: missing -> a.checkEqual("25. getNumTorpedoHits", h.stats[1].getNumTorpedoHits(), 0);

    // - ship 1
    a.checkEqual("31. getDamage", s->getDamage(), 100);
    a.checkEqual("32. getShield", s->getShield(), 0);
    a.checkEqual("33. getCrew",   s->getCrew(), 2884);
    a.checkEqual("34. getOwner",  s->getOwner(), 0);
    a.checkEqual("35. getAmmo",   s->getAmmo(), 290);

    // - ship 2
    a.checkEqual("41. getDamage", p->getDamage(), 84);
    a.checkEqual("42. getShield", p->getShield(), 0);
    a.checkEqual("43. getOwner",  p->getOwner(), 9);

    /* Existing torpedoes are worth 20*12 + 30*13 = 630 mc = 48 torpedoes effectively,
       plus 3*8 = 24 from PlanetaryTorpsPerTube = 78 total.
       We fire 24 torpedoes = 312 mc worth,
       and thus remove ceil(312 / (12+13)) = 13 of each. */
    a.checkEqual("51. getNumBaseTorpedoes", p->getNumBaseTorpedoes(5), 7);
    a.checkEqual("52. getNumBaseTorpedoes", p->getNumBaseTorpedoes(6), 17);
}

/** Test PHost simulation, intercept-attack.
    A: prepare four ships, with two of them intercepting one, PHost simulation.
    E: expected results and metadata produced (verified against PCC2 playvcr). */
AFL_TEST("game.sim.Run:VcrPHost4:intercept", a)
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrPHost4, game::sim::Configuration::BalanceNone);

    // Setup
    /*Ship* s1 =*/ addOutrider(a, h.setup, 1, 1, h.list);
    Ship* s2 = addOutrider(a, h.setup, 2, 2, h.list);
    Ship* s3 = addOutrider(a, h.setup, 3, 3, h.list);
    Ship* s4 = addOutrider(a, h.setup, 4, 4, h.list);
    s3->setAggressiveness(2);
    s3->setInterceptId(2);
    s3->setFriendlyCode("200");
    s4->setAggressiveness(2);
    s4->setInterceptId(2);
    s4->setFriendlyCode("100");

    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result
    // - rng has not been touched because we use seed control
    a.checkEqual("01. getSeed", h.rng.getSeed(), 42U);

    // - a battle has been created
    // Note that as of 20200923, this result is DIFFERENT from PCC2 2.0.9:
    // PCC2 places the interceptor on the left side, whereas we place them on the right (same as in THost and c2web).
    // This is not a difference from actual host behaviour because PHost always randomizes sides;
    // this test only disabled random left/right for determinism of test results.
    a.checkNonNull("11. battles",           h.result.battles.get());
    a.checkEqual("12. getId",               h.result.battles->getBattle(0)->getObject(0, false)->getId(), 2);
    a.checkEqual("13. getId",               h.result.battles->getBattle(0)->getObject(1, false)->getId(), 4);
    a.checkEqual("14. getNumBattles",       h.result.battles->getNumBattles(), 1U);
    a.checkEqual("15. this_battle_weight",  h.result.this_battle_weight, 1);
    a.checkEqual("16. total_battle_weight", h.result.total_battle_weight, 1);
    a.checkEqual("17. series_length",       h.result.series_length, 110);
    a.checkEqual("18. this_battle_index",   h.result.this_battle_index, 0);

    // - statistics
    a.checkEqual("21. stats", h.stats.size(), 4U);

    // - ship 2
    a.checkEqual("31. getDamage", s2->getDamage(), 100);
    a.checkEqual("32. getShield", s2->getShield(), 0);
    a.checkEqual("33. getCrew",   s2->getCrew(), 110);
    a.checkEqual("34. getOwner",  s2->getOwner(), 0);

    // - ship 4
    a.checkEqual("41. getDamage", s4->getDamage(), 100);
    a.checkEqual("42. getShield", s4->getShield(), 0);
    a.checkEqual("43. getCrew",   s4->getCrew(), 132);
    a.checkEqual("44. getOwner",  s4->getOwner(), 0);
}

/** Test multi-ship PHost simulation.
    A: prepare multiple ships, PHost simulation.
    E: expected results and metadata produced. Expected battle order produced. This is a regression test to ensure constant behaviour. */
AFL_TEST("game.sim.Run:VcrPHost2:multi-ship", a)
{
    // Environment
    TestHarness h;
    h.opts.setMode(game::sim::Configuration::VcrPHost2, 0, h.config);

    // Setup
    Ship* s1 = addOutrider(a, h.setup, 1, 1, h.list);
    Ship* s2 = addOutrider(a, h.setup, 2, 2, h.list);
    Ship* s3 = addOutrider(a, h.setup, 3, 2, h.list);
    Ship* s4 = addOutrider(a, h.setup, 4, 2, h.list);
    Planet* p = addPlanet(h.setup, 17, 1);
    s1->setFriendlyCode("-20");
    s2->setFriendlyCode("100");
    s3->setFriendlyCode("300");
    s4->setFriendlyCode("200");
    p->setFriendlyCode("ATT");

    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result
    // - rng has been used
    a.checkEqual("01. getSeed", h.rng.getSeed(), 3638705852U);

    // - battles have been created; series length unchanged
    a.checkNonNull("11. battles",           h.result.battles.get());
    a.checkEqual("12. getNumBattles",       h.result.battles->getNumBattles(), 4U);
    a.checkEqual("13. this_battle_weight",  h.result.this_battle_weight, 1);
    a.checkEqual("14. total_battle_weight", h.result.total_battle_weight, 1);
    a.checkEqual("15. series_length",       h.result.series_length, 110);
    a.checkEqual("16. this_battle_index",   h.result.this_battle_index, 0);

    // - first battle (#1 is aggressor, #1 wins)
    a.checkEqual("21. getId", h.result.battles->getBattle(0)->getObject(0, false)->getId(), 1);
    a.checkEqual("22. getId", h.result.battles->getBattle(0)->getObject(1, false)->getId(), 2);

    // - second battle (#4 is aggressor, #4 wins)
    a.checkEqual("31. getId", h.result.battles->getBattle(1)->getObject(0, false)->getId(), 1);
    a.checkEqual("32. getId", h.result.battles->getBattle(1)->getObject(1, false)->getId(), 4);

    // - third battle (#4 is aggressor, #17 wins)
    a.checkEqual("41. getId", h.result.battles->getBattle(2)->getObject(0, false)->getId(), 4);
    a.checkEqual("42. getId", h.result.battles->getBattle(2)->getObject(1, false)->getId(), 17);

    // - fourth battle (#3 is aggressor, #17 wins)
    a.checkEqual("51. getId", h.result.battles->getBattle(3)->getObject(0, false)->getId(), 3);
    a.checkEqual("52. getId", h.result.battles->getBattle(3)->getObject(1, false)->getId(), 17);

    // - statistics
    a.checkEqual("61. stats", h.stats.size(), 5U);

    // - ship 1
    a.checkEqual("71. getDamage", s1->getDamage(), 100);
    a.checkEqual("72. getShield", s1->getShield(), 0);
    a.checkEqual("73. getCrew",   s1->getCrew(), 84);
    a.checkEqual("74. getOwner",  s1->getOwner(), 0);

    // - ship 2
    a.checkEqual("81. getDamage", s2->getDamage(), 100);
    a.checkEqual("82. getShield", s2->getShield(), 0);
    a.checkEqual("83. getCrew",   s2->getCrew(), 88);
    a.checkEqual("84. getOwner",  s2->getOwner(), 0);

    // - ship 3
    a.checkEqual("91. getDamage", s3->getDamage(), 100);
    a.checkEqual("92. getShield", s3->getShield(), 0);
    a.checkEqual("93. getCrew",   s3->getCrew(), 107);
    a.checkEqual("94. getOwner",  s3->getOwner(), 0);

    // - ship 4
    a.checkEqual("101. getDamage", s4->getDamage(), 100);
    a.checkEqual("102. getShield", s4->getShield(), 0);
    a.checkEqual("103. getCrew",   s4->getCrew(), 94);
    a.checkEqual("104. getOwner",  s4->getOwner(), 0);

    // - planet
    a.checkEqual("111. getDamage", p->getDamage(), 0);
    a.checkEqual("112. getShield", p->getShield(), 100);
    a.checkEqual("113. getOwner",  p->getOwner(), 1);
}

/** Test PHost simulation, with commanders.
    A: prepare multiple ships including a Commander, PHost simulation.
    E: expected results and metadata produced (verified against PCC2 playvcr). */
AFL_TEST("game.sim.Run:VcrPHost4:Commander", a)
{
    // Environment
    TestHarness h;
    h.config[game::config::HostConfiguration::NumExperienceLevels].set(4);
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrPHost4, game::sim::Configuration::BalanceNone);

    // Setup
    Ship* s1 = addOutrider(a, h.setup, 1, 1, h.list);
    Ship* s2 = addOutrider(a, h.setup, 2, 2, h.list);
    Ship* s3 = addOutrider(a, h.setup, 3, 2, h.list);
    s1->setAggressiveness(Ship::agg_Passive);
    s2->setAggressiveness(Ship::agg_Kill);
    s3->setAggressiveness(Ship::agg_Passive);
    s3->setExperienceLevel(3);
    s3->setFlags(Ship::fl_Commander | Ship::fl_CommanderSet);

    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result
    // - rng has not been used
    a.checkEqual("01. getSeed", h.rng.getSeed(), 42U);

    // - battles have been created
    a.checkNonNull("11. battles",           h.result.battles.get());
    a.checkEqual("12. getNumBattles",       h.result.battles->getNumBattles(), 1U);
    a.checkEqual("13. getId",               h.result.battles->getBattle(0)->getObject(0, false)->getId(), 1);
    a.checkEqual("14. getExperienceLevel",  h.result.battles->getBattle(0)->getObject(0, false)->getExperienceLevel(), 0);
    a.checkEqual("15. getId",               h.result.battles->getBattle(0)->getObject(1, false)->getId(), 2);
    a.checkEqual("16. getExperienceLevel",  h.result.battles->getBattle(0)->getObject(1, false)->getExperienceLevel(), 1);
    a.checkEqual("17. this_battle_weight",  h.result.this_battle_weight, 1);
    a.checkEqual("18. total_battle_weight", h.result.total_battle_weight, 1);
    a.checkEqual("19. series_length",       h.result.series_length, 110);
    a.checkEqual("20. this_battle_index",   h.result.this_battle_index, 0);

    // - statistics
    a.checkEqual("21. stats", h.stats.size(), 3U);
    a.checkEqual("22. getNumFights", h.stats[0].getNumFights(), 1);
    a.checkEqual("23. getNumFights", h.stats[1].getNumFights(), 1);
    a.checkEqual("24. getNumFights", h.stats[2].getNumFights(), 0);

    // - ship 1
    a.checkEqual("31. getDamage", s1->getDamage(), 37);
    a.checkEqual("32. getShield", s1->getShield(), 0);
    a.checkEqual("33. getCrew",   s1->getCrew(), 140);
    a.checkEqual("34. getOwner",  s1->getOwner(), 1);

    // - ship 2
    a.checkEqual("41. getDamage", s2->getDamage(), 100);
    a.checkEqual("42. getShield", s2->getShield(), 0);
    a.checkEqual("43. getCrew",   s2->getCrew(), 92);
    a.checkEqual("44. getOwner",  s2->getOwner(), 0);
}

/** Test deactivated ship.
    A: prepare two ships, one deactivated.
    E: no fight happens. */
AFL_TEST("game.sim.Run:ship:deactivated", a)
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    // As of 20200920, setting an Intercept Id will try to match the ships even though #1 is not part of battle order due to being disabled.
    addOutrider(a, h.setup, 1, 12, h.list)->setFlags(Ship::fl_Deactivated);
    addOutrider(a, h.setup, 2, 11, h.list)->setInterceptId(1);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result: no fight
    a.checkNonNull("01. battles", h.result.battles.get());
    a.checkEqual("02. getNumBattles", h.result.battles->getNumBattles(), 0U);
}

/** Test allied ships.
    A: prepare two ships, bidirectional alliance.
    E: no fight happens. */
AFL_TEST("game.sim.Run:ship:allied", a)
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);
    h.opts.allianceSettings().set(11, 12, true);
    h.opts.allianceSettings().set(12, 11, true);

    // Setup
    addOutrider(a, h.setup, 1, 12, h.list);
    addOutrider(a, h.setup, 2, 11, h.list);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result: no fight
    a.checkNonNull("01. battles", h.result.battles.get());
    a.checkEqual("02. getNumBattles", h.result.battles->getNumBattles(), 0U);
}

/** Test passive ships.
    A: prepare two ships, passive.
    E: no fight happens. */
AFL_TEST("game.sim.Run:ship:passive", a)
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    addOutrider(a, h.setup, 1, 12, h.list)->setAggressiveness(Ship::agg_Passive);
    addOutrider(a, h.setup, 2, 11, h.list)->setAggressiveness(Ship::agg_Passive);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result: no fight
    a.checkNonNull("01. battles", h.result.battles.get());
    a.checkEqual("02. getNumBattles", h.result.battles->getNumBattles(), 0U);
}

/** Test non-hostile ships.
    A: prepare two ships, mismatching primary enemy.
    E: no fight happens. */
AFL_TEST("game.sim.Run:ship:not-enemy", a)
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    addOutrider(a, h.setup, 1, 12, h.list)->setAggressiveness(7);
    addOutrider(a, h.setup, 2, 11, h.list)->setAggressiveness(2);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result: no fight
    a.checkNonNull("01. battles", h.result.battles.get());
    a.checkEqual("02. getNumBattles", h.result.battles->getNumBattles(), 0U);
}

/** Test hostile ships.
    A: prepare two ships, one passive, one with primary enemy.
    E: fight happens. */
AFL_TEST("game.sim.Run:ship:enemy", a)
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    addOutrider(a, h.setup, 1, 12, h.list)->setAggressiveness(11);
    addOutrider(a, h.setup, 2, 11, h.list)->setAggressiveness(2);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result: no fight
    a.checkNonNull("01. battles", h.result.battles.get());
    a.checkEqual("02. getNumBattles", h.result.battles->getNumBattles(), 1U);
}

/** Test hostile ships, via persistent enemies.
    A: prepare two ships, one passive, one with mismatching primary enemy but persistent enemy setting
    E: fight happens. */
AFL_TEST("game.sim.Run:ship:persistent-enemy", a)
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);
    h.opts.enemySettings().set(11, 12, true);

    // Setup
    addOutrider(a, h.setup, 1, 12, h.list)->setAggressiveness(5);
    addOutrider(a, h.setup, 2, 11, h.list)->setAggressiveness(2);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result: no fight
    a.checkNonNull("01. battles", h.result.battles.get());
    a.checkEqual("02. getNumBattles", h.result.battles->getNumBattles(), 1U);
}

/** Test cloaked ships.
    A: prepare two ships, one cloaked.
    E: no fight happens. */
AFL_TEST("game.sim.Run:ship:cloaked", a)
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);
    h.config[game::config::HostConfiguration::AllowCloakedShipsAttack].set(0);

    // Setup
    addOutrider(a, h.setup, 1, 12, h.list)->setFlags(Ship::fl_Cloaked);
    addOutrider(a, h.setup, 2, 11, h.list);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result: no fight
    a.checkNonNull("01. battles", h.result.battles.get());
    a.checkEqual("02. getNumBattles", h.result.battles->getNumBattles(), 0U);
}

/** Test ships, matching friendly codes.
    A: prepare two ships with matching friendly codes.
    E: no fight happens. */
AFL_TEST("game.sim.Run:ship:fcode-match", a)
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    addOutrider(a, h.setup, 1, 12, h.list)->setFriendlyCode("abc");
    addOutrider(a, h.setup, 2, 11, h.list)->setFriendlyCode("abc");
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result: no fight
    a.checkNonNull("01. battles", h.result.battles.get());
    a.checkEqual("02. getNumBattles", h.result.battles->getNumBattles(), 0U);
}

/** Test ships, no fuel.
    A: prepare two ships, one with no fuel.
    E: no fight happens. */
AFL_TEST("game.sim.Run:ship:no-fuel", a)
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    addOutrider(a, h.setup, 1, 12, h.list)->setAggressiveness(Ship::agg_NoFuel);
    addOutrider(a, h.setup, 2, 11, h.list);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result: no fight
    a.checkNonNull("01. battles", h.result.battles.get());
    a.checkEqual("02. getNumBattles", h.result.battles->getNumBattles(), 0U);
}

/** Test ships, Cloaked Fighter Bays ability.
    A: prepare three ships; one passive with Cloaked Fighter Bays ability.
    E: expected results and metadata produced (verified against PCC2 playvcr). */
AFL_TEST("game.sim.Run:ship:CloakedFighterBays", a)
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrNuHost, game::sim::Configuration::BalanceNone);

    // Setup
    Ship* s1 = addGorbie(a, h.setup, 1, 8, h.list);
    Ship* s2 = addGorbie(a, h.setup, 2, 4, h.list);
    Ship* s3 = addGorbie(a, h.setup, 3, 8, h.list);
    h.result.init(h.opts, 0);
    s3->setAggressiveness(Ship::agg_Passive);
    s3->setFlags(Ship::fl_Cloaked | Ship::fl_CloakedBays | Ship::fl_CloakedBaysSet);
    // This line is not needed if Klingon ships automatically have DoubleBeamChargeAbility in NuHost:
    // s2->setFlags(Ship::fl_DoubleBeamCharge | Ship::fl_DoubleBeamChargeSet);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result
    // - rng has not been touched because we use seed control
    a.checkEqual("01. getSeed", h.rng.getSeed(), 42U);

    // - a battle has been created
    a.checkNonNull("11. battles",           h.result.battles.get());
    a.checkEqual("12. getNumBattles",       h.result.battles->getNumBattles(), 1U);
    a.checkEqual("13. getId",               h.result.battles->getBattle(0)->getObject(0, false)->getId(), 2);
    a.checkEqual("14. getNumBays",          h.result.battles->getBattle(0)->getObject(0, false)->getNumBays(), 10);
    a.checkEqual("15. getNumFights",        h.result.battles->getBattle(0)->getObject(0, false)->getNumFighters(), 250);
    a.checkEqual("16. getId",               h.result.battles->getBattle(0)->getObject(1, false)->getId(), 1);
    a.checkEqual("17. getNumBays",          h.result.battles->getBattle(0)->getObject(1, false)->getNumBays(), 20);
    a.checkEqual("18. getNumFighters",      h.result.battles->getBattle(0)->getObject(1, false)->getNumFighters(), 500);
    a.checkEqual("19. this_battle_weight",  h.result.this_battle_weight, 1);
    a.checkEqual("20. total_battle_weight", h.result.total_battle_weight, 1);
    a.checkEqual("21. series_length",       h.result.series_length, 118);
    a.checkEqual("22. this_battle_index",   h.result.this_battle_index, 0);

    // - statistics
    a.checkEqual("31. stats", h.stats.size(), 3U);
    a.checkEqual("32. getNumFights", h.stats[0].getNumFights(), 1);
    a.checkEqual("33. getNumFights", h.stats[1].getNumFights(), 1);
    a.checkEqual("34. getNumFights", h.stats[2].getNumFights(), 0);

    // - ship 1
    a.checkEqual("41. getDamage", s1->getDamage(), 9);
    a.checkEqual("42. getShield", s1->getShield(), 0);
    a.checkEqual("43. getCrew",   s1->getCrew(), 2287);
    a.checkEqual("44. getOwner",  s1->getOwner(), 8);
    a.checkEqual("45. getAmmo",   s1->getAmmo(), 183);

    // - ship 2
    a.checkEqual("51. getDamage", s2->getDamage(), 102);
    a.checkEqual("52. getShield", s2->getShield(), 0);
    a.checkEqual("53. getCrew",   s2->getCrew(), 2287);
    a.checkEqual("54. getOwner",  s2->getOwner(), 0);
    a.checkEqual("55. getAmmo",   s2->getAmmo(), 150);

    // - ship 3
    a.checkEqual("61. getDamage", s3->getDamage(), 0);
    a.checkEqual("62. getShield", s3->getShield(), 100);
    a.checkEqual("63. getCrew",   s3->getCrew(), 2287);
    a.checkEqual("64. getOwner",  s3->getOwner(), 8);
    a.checkEqual("65. getAmmo",   s3->getAmmo(), 183);
}

/** Test ships, Cloaked Fighter Bays ability, ammo limit (bug #416).
    A: prepare three ships; one passive with Cloaked Fighter Bays ability, one aggressive with ammo limit.
    E: expected results and metadata produced (verified against PCC2 playvcr). In particular, correct fighter amounts lost. */
AFL_TEST("game.sim.Run:ship:CloakedFighterBays:ammo-limit", a)
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrNuHost, game::sim::Configuration::BalanceNone);

    // Setup
    Ship* s1 = addOutrider(a, h.setup, 1, 12, h.list);
    Ship* s2 = addGorbie(a, h.setup, 2, 8, h.list);
    Ship* s3 = addGorbie(a, h.setup, 3, 8, h.list);
    h.result.init(h.opts, 0);
    s2->setFriendlyCode("NT1");
    s3->setAggressiveness(Ship::agg_Passive);
    s3->setFlags(Ship::fl_Cloaked | Ship::fl_CloakedBays | Ship::fl_CloakedBaysSet);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result
    // - rng has not been touched because we use seed control
    a.checkEqual("01. getSeed", h.rng.getSeed(), 42U);

    // - a battle has been created
    a.checkNonNull("11. battles",           h.result.battles.get());
    a.checkEqual("12. getNumBattles",       h.result.battles->getNumBattles(), 1U);
    a.checkEqual("13. getId",               h.result.battles->getBattle(0)->getObject(0, false)->getId(), 2);
    a.checkEqual("14. getNumBays",          h.result.battles->getBattle(0)->getObject(0, false)->getNumBays(), 20);
    a.checkEqual("15. getNumFighters",      h.result.battles->getBattle(0)->getObject(0, false)->getNumFighters(), 10); /* limit applied */
    a.checkEqual("16. getId",               h.result.battles->getBattle(0)->getObject(1, false)->getId(), 1);
    a.checkEqual("17. this_battle_weight",  h.result.this_battle_weight, 1);
    a.checkEqual("18. total_battle_weight", h.result.total_battle_weight, 1);
    a.checkEqual("19. series_length",       h.result.series_length, 118);
    a.checkEqual("20. this_battle_index",   h.result.this_battle_index, 0);

    // - statistics
    a.checkEqual("21. stats", h.stats.size(), 3U);
    a.checkEqual("22. getNumFights", h.stats[0].getNumFights(), 1);
    a.checkEqual("23. getNumFights", h.stats[1].getNumFights(), 1);
    a.checkEqual("24. getNumFights", h.stats[2].getNumFights(), 0);

    // - ship 1
    a.checkEqual("31. getDamage", s1->getDamage(), 187);
    a.checkEqual("32. getShield", s1->getShield(), 0);
    a.checkEqual("33. getCrew",   s1->getCrew(), 64);
    a.checkEqual("34. getOwner",  s1->getOwner(), 0);
    a.checkEqual("35. getAmmo",   s1->getAmmo(), 0);

    // - ship 2
    a.checkEqual("41. getDamage", s2->getDamage(), 0);
    a.checkEqual("42. getShield", s2->getShield(), 100);
    a.checkEqual("43. getCrew",   s2->getCrew(), 2287);
    a.checkEqual("44. getOwner",  s2->getOwner(), 8);
    a.checkEqual("45. getAmmo",   s2->getAmmo(), 248);

    // - ship 3
    a.checkEqual("51. getDamage", s3->getDamage(), 0);
    a.checkEqual("52. getShield", s3->getShield(), 100);
    a.checkEqual("53. getCrew",   s3->getCrew(), 2287);
    a.checkEqual("54. getOwner",  s3->getOwner(), 8);
    a.checkEqual("55. getAmmo",   s3->getAmmo(), 249);
}

/** Test ships, Squadron ability.
    A: prepare two ships; a small Squadron one with three beams, and a big one.
    E: expected results and metadata produced (verified against PCC2 playvcr). */
AFL_TEST("game.sim.Run:ship:Squadron", a)
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrNuHost, game::sim::Configuration::BalanceNone);

    // Setup
    Ship* s1 = addGorbie(a, h.setup, 1, 8, h.list);
    Ship* s2 = addGorbie(a, h.setup, 2, 4, h.list);
    h.result.init(h.opts, 0);
    s1->setHullType(0, h.list);
    s1->setMass(200);
    s1->setNumBeams(3);
    s1->setNumBays(0);
    s1->setFlags(Ship::fl_Squadron | Ship::fl_SquadronSet);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result
    // - rng has not been touched because we use seed control
    a.checkEqual("01. getSeed", h.rng.getSeed(), 42U);

    // - a battle has been created
    a.checkNonNull("11. battles",           h.result.battles.get());
    a.checkEqual("12. getNumBattles",       h.result.battles->getNumBattles(), 1U);
    a.checkEqual("13. getId",               h.result.battles->getBattle(0)->getObject(0, false)->getId(), 2);
    a.checkEqual("14. getNumBeams",         h.result.battles->getBattle(0)->getObject(0, false)->getNumBeams(), 10);
    a.checkEqual("15. getId",               h.result.battles->getBattle(0)->getObject(1, false)->getId(), 1);
    a.checkEqual("16. getNumBeams",         h.result.battles->getBattle(0)->getObject(1, false)->getNumBeams(), 3);
    a.checkEqual("17. this_battle_weight",  h.result.this_battle_weight, 1);
    a.checkEqual("18. total_battle_weight", h.result.total_battle_weight, 1);
    a.checkEqual("19. series_length",       h.result.series_length, 118);
    a.checkEqual("20. this_battle_index",   h.result.this_battle_index, 0);

    // - statistics
    a.checkEqual("21. stats", h.stats.size(), 2U);

    // - ship 1
    a.checkEqual("31. getDamage",   s1->getDamage(), 0);
    a.checkEqual("32. getShield",   s1->getShield(), 100);
    a.checkEqual("33. getCrew",     s1->getCrew(), 2287);
    a.checkEqual("34. getOwner",    s1->getOwner(), 8);
    a.checkEqual("35. getNumBeams", s1->getNumBeams(), 2);       // <- changed

    // - ship 2
    a.checkEqual("41. getDamage", s2->getDamage(), 0);
    a.checkEqual("42. getShield", s2->getShield(), 100);
    a.checkEqual("43. getCrew",   s2->getCrew(), 2287);
    a.checkEqual("44. getOwner",  s2->getOwner(), 4);
    a.checkEqual("45. getAmmo",   s2->getAmmo(), 244);
}

/** Test deactivated planet.
    A: prepare ship and planet, planet deactivated.
    E: no fight happens. */
AFL_TEST("game.sim.Run:planet:deactivated", a)
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    addOutrider(a, h.setup, 1, 5, h.list);
    addPlanet(h.setup, 1, 4)->setFlags(Planet::fl_Deactivated);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result: no fight
    a.checkNonNull("01. battles", h.result.battles.get());
    a.checkEqual("02. getNumBattles", h.result.battles->getNumBattles(), 0U);
}

/** Test cloaked ship at planet.
    A: prepare ship and planet, ship cloaked.
    E: no fight happens. */
AFL_TEST("game.sim.Run:planet:cloaked-ship", a)
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);
    h.config[game::config::HostConfiguration::AllowCloakedShipsAttack].set(0);

    // Setup
    addOutrider(a, h.setup, 1, 5, h.list)->setFlags(Ship::fl_Cloaked);
    addPlanet(h.setup, 1, 4)->setFriendlyCode("ATT");
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result: no fight
    a.checkNonNull("01. battles", h.result.battles.get());
    a.checkEqual("02. getNumBattles", h.result.battles->getNumBattles(), 0U);
}

/** Test ship and planet with matching friendly codes.
    A: prepare ship and planet with matching friendly codes.
    E: no fight happens. */
AFL_TEST("game.sim.Run:planet:fcode-match", a)
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    addOutrider(a, h.setup, 1, 5, h.list)->setFriendlyCode("xyz");
    addPlanet(h.setup, 1, 4)->setFriendlyCode("xyz");
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result: no fight
    a.checkNonNull("01. battles", h.result.battles.get());
    a.checkEqual("02. getNumBattles", h.result.battles->getNumBattles(), 0U);
}

/** Test ship and planet, allied.
    A: prepare ship and planet, set up alliance.
    E: no fight happens. */
AFL_TEST("game.sim.Run:planet:allied", a)
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);
    h.opts.allianceSettings().set(4, 5, true);
    h.opts.allianceSettings().set(5, 4, true);

    // Setup
    addOutrider(a, h.setup, 1, 5, h.list);
    addPlanet(h.setup, 1, 4);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result: no fight
    a.checkNonNull("01. battles", h.result.battles.get());
    a.checkEqual("02. getNumBattles", h.result.battles->getNumBattles(), 0U);
}

/** Test ship and planet, not aggressive.
    A: prepare ship and planet, none is aggressive.
    E: no fight happens. */
AFL_TEST("game.sim.Run:planet:not-aggressive", a)
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    addOutrider(a, h.setup, 1, 5, h.list)->setAggressiveness(Ship::agg_Passive);
    addPlanet(h.setup, 1, 4)->setFriendlyCode("123");
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result: no fight
    a.checkNonNull("01. battles", h.result.battles.get());
    a.checkEqual("02. getNumBattles", h.result.battles->getNumBattles(), 0U);
}

/** Test ship and planet, mismatching primary enemy.
    A: prepare ship and planet, planet not aggressive, ship with mismatching enemy.
    E: no fight happens. */
AFL_TEST("game.sim.Run:planet:not-enemy", a)
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    addOutrider(a, h.setup, 1, 5, h.list)->setAggressiveness(7);
    addPlanet(h.setup, 1, 4)->setFriendlyCode("123");
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result: no fight
    a.checkNonNull("01. battles", h.result.battles.get());
    a.checkEqual("02. getNumBattles", h.result.battles->getNumBattles(), 0U);
}

/** Test ship and planet, ship is immune (by being Klingon).
    A: prepare ship and planet, ship is of an immune race, planet is aggressive.
    E: no fight happens. */
AFL_TEST("game.sim.Run:planet:immune-race", a)
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    addOutrider(a, h.setup, 1, 4, h.list)->setAggressiveness(7);
    addPlanet(h.setup, 1, 2)->setFriendlyCode("ATT");
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result: no fight
    a.checkNonNull("01. battles", h.result.battles.get());
    a.checkEqual("02. getNumBattles", h.result.battles->getNumBattles(), 0U);
}

/** Test ship and planet, ship is immune (by being Bird without fuel).
    A: prepare ship and planet, ship is Bird and fuelless, planet is aggressive.
    E: no fight happens. */
AFL_TEST("game.sim.Run:planet:immune-bird", a)
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    addOutrider(a, h.setup, 1, 3, h.list)->setAggressiveness(Ship::agg_NoFuel);
    addPlanet(h.setup, 1, 2)->setFriendlyCode("NUK");
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result: no fight
    a.checkNonNull("01. battles", h.result.battles.get());
    a.checkEqual("02. getNumBattles", h.result.battles->getNumBattles(), 0U);
}

/** Test ship and planet, primary enemy.
    A: prepare ship and planet, ship has PE.
    E: fight happens. */
AFL_TEST("game.sim.Run:planet:matching-enemy", a)
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    addOutrider(a, h.setup, 1, 9, h.list)->setAggressiveness(2);
    addPlanet(h.setup, 1, 2)->setFriendlyCode("qqq");
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result: no fight
    a.checkNonNull("01. battles", h.result.battles.get());
    a.checkEqual("02. getNumBattles", h.result.battles->getNumBattles(), 1U);
}

/** Test ship and planet, planet has NUK.
    A: prepare ship and planet, ship has no fuel, planet has NUK.
    E: fight happens. */
AFL_TEST("game.sim.Run:planet:NUK", a)
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    addOutrider(a, h.setup, 1, 9, h.list)->setAggressiveness(Ship::agg_NoFuel);
    addPlanet(h.setup, 1, 2)->setFriendlyCode("NUK");
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result: no fight
    a.checkNonNull("01. battles", h.result.battles.get());
    a.checkEqual("02. getNumBattles", h.result.battles->getNumBattles(), 1U);
}

/** Test basic FLAK simulation.
    A: prepare two ships, FLAK simulation.
    E: expected results and metadata produced. This is a regression test to ensure constant behaviour. */
AFL_TEST("game.sim.Run:VcrFLAK", a)
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrFLAK, game::sim::Configuration::BalanceNone);

    // Setup
    Ship* s1 = addOutrider(a, h.setup, 1, 12, h.list);
    Ship* s2 = addOutrider(a, h.setup, 2, 11, h.list);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result
    // Note that FLAK does not support seed control and will touch the RNG.

    // - a battle has been created
    a.checkNonNull("01. battles",           h.result.battles.get());
    a.checkEqual("02. getNumBattles",       h.result.battles->getNumBattles(), 1U);
    a.checkEqual("03. this_battle_weight",  h.result.this_battle_weight, 1);
    a.checkEqual("04. total_battle_weight", h.result.total_battle_weight, 1);
    a.checkEqual("05. series_length",       h.result.series_length, 110);
    a.checkEqual("06. this_battle_index",   h.result.this_battle_index, 0);

    // - statistics
    a.checkEqual("11. stats", h.stats.size(), 2U);

    // - battle content
    a.checkNonNull("21. getBattle",     h.result.battles->getBattle(0));
    a.checkEqual  ("22. getNumObjects", h.result.battles->getBattle(0)->getNumObjects(), 2U);
    a.checkNonNull("23. getObject",     h.result.battles->getBattle(0)->getObject(0, false));
    a.checkEqual  ("24. getMass",       h.result.battles->getBattle(0)->getObject(0, false)->getMass(), 75);
    a.checkNonNull("25. getObject",     h.result.battles->getBattle(0)->getObject(1, false));
    a.checkEqual  ("26. getMass",       h.result.battles->getBattle(0)->getObject(1, false)->getMass(), 75);

    // - ship 1
    a.checkEqual("31. getDamage", s1->getDamage(), 71);
    a.checkEqual("32. getShield", s1->getShield(), 0);
    a.checkEqual("33. getCrew",   s1->getCrew(), 131);
    a.checkEqual("34. getOwner",  s1->getOwner(), 12);

    // - ship 2
    a.checkEqual("41. getDamage", s2->getDamage(), 103);
    a.checkEqual("42. getShield", s2->getShield(), 0);
    a.checkEqual("43. getCrew",   s2->getCrew(), 109);
    a.checkEqual("44. getOwner",  s2->getOwner(), 0);
}

/** Test basic FLAK simulation, with ESB.
    A: prepare two ships, FLAK simulation.
    E: expected results and metadata produced. This is a regression test to ensure constant behaviour. */
AFL_TEST("game.sim.Run:VcrFLAK:esb", a)
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrFLAK, game::sim::Configuration::BalanceNone);
    h.opts.setEngineShieldBonus(20);

    // Setup
    Ship* s1 = addOutrider(a, h.setup, 1, 12, h.list);
    Ship* s2 = addOutrider(a, h.setup, 2, 11, h.list);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result
    // Note that FLAK does not support seed control and will touch the RNG.

    // - a battle has been created
    a.checkNonNull("01. battles",           h.result.battles.get());
    a.checkEqual("02. getNumBattles",       h.result.battles->getNumBattles(), 1U);
    a.checkEqual("03. this_battle_weight",  h.result.this_battle_weight, 1);
    a.checkEqual("04. total_battle_weight", h.result.total_battle_weight, 1);
    a.checkEqual("05. series_length",       h.result.series_length, 110);
    a.checkEqual("06. this_battle_index",   h.result.this_battle_index, 0);

    // - battle content
    a.checkNonNull("11. getBattle",     h.result.battles->getBattle(0));
    a.checkEqual  ("12. getNumObjects", h.result.battles->getBattle(0)->getNumObjects(), 2U);
    a.checkNonNull("13. getObject",     h.result.battles->getBattle(0)->getObject(0, false));
    a.checkEqual  ("14. getMass",       h.result.battles->getBattle(0)->getObject(0, false)->getMass(), 135);  // 75 kt + 300 mc * 20%
    a.checkNonNull("15. getObject",     h.result.battles->getBattle(0)->getObject(1, false));
    a.checkEqual  ("16. getMass",       h.result.battles->getBattle(0)->getObject(1, false)->getMass(), 135);

    // - statistics
    a.checkEqual("21. stats", h.stats.size(), 2U);

    // - ship 1
    a.checkEqual("31. getDamage", s1->getDamage(), 96);
    a.checkEqual("32. getShield", s1->getShield(), 0);
    a.checkEqual("33. getCrew",   s1->getCrew(), 76);
    a.checkEqual("34. getOwner",  s1->getOwner(), 12);

    // - ship 2
    a.checkEqual("41. getDamage", s2->getDamage(), 107);
    a.checkEqual("42. getShield", s2->getShield(), 0);
    a.checkEqual("43. getCrew",   s2->getCrew(), 64);
    a.checkEqual("44. getOwner",  s2->getOwner(), 0);
}

/** Test multi-ship FLAK simulation.
    A: prepare multiple ships, FLAK simulation.
    E: expected results and metadata produced. This is a regression test to ensure constant behaviour. */
AFL_TEST("game.sim.Run:VcrFLAK:multi-ship", a)
{
    // Environment
    TestHarness h;
    h.opts.setMode(game::sim::Configuration::VcrFLAK, 0, h.config);

    // Setup
    Ship* s1 = addOutrider(a, h.setup, 1, 1, h.list);
    Ship* s2 = addOutrider(a, h.setup, 2, 2, h.list);
    Ship* s3 = addOutrider(a, h.setup, 3, 2, h.list);
    Ship* s4 = addOutrider(a, h.setup, 4, 2, h.list);
    Planet* p = addPlanet(h.setup, 17, 1);
    s1->setFriendlyCode("-20");
    s2->setFriendlyCode("100");
    s3->setFriendlyCode("300");
    s4->setFriendlyCode("200");
    p->setFriendlyCode("ATT");
    p->setNumBaseFighters(60);

    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result
    // - battles have been created; series length unchanged
    a.checkNonNull("01. battles",           h.result.battles.get());
    a.checkEqual("02. getNumBattles",       h.result.battles->getNumBattles(), 1U);
    a.checkEqual("03. this_battle_weight",  h.result.this_battle_weight, 1);
    a.checkEqual("04. total_battle_weight", h.result.total_battle_weight, 1);
    a.checkEqual("05. series_length",       h.result.series_length, 110);
    a.checkEqual("06. this_battle_index",   h.result.this_battle_index, 0);

    // - statistics
    a.checkEqual("11. stats", h.stats.size(), 5U);
    a.checkEqual("12. getMinFightersAboard", h.stats.at(4).getMinFightersAboard(), 39);

    // - ship 1
    a.checkEqual("21. getDamage", s1->getDamage(), 0);
    a.checkEqual("22. getShield", s1->getShield(), 52);
    a.checkEqual("23. getCrew",   s1->getCrew(), 180);
    a.checkEqual("24. getOwner",  s1->getOwner(), 1);

    // - ship 2
    a.checkEqual("31. getDamage", s2->getDamage(), 159);
    a.checkEqual("32. getShield", s2->getShield(), 0);
    a.checkEqual("33. getCrew",   s2->getCrew(), 101);
    a.checkEqual("34. getOwner",  s2->getOwner(), 0);

    // - ship 3
    a.checkEqual("41. getDamage", s3->getDamage(), 151);
    a.checkEqual("42. getShield", s3->getShield(), 0);
    a.checkEqual("43. getCrew",   s3->getCrew(), 105);
    a.checkEqual("44. getOwner",  s3->getOwner(), 0);

    // - ship 4
    a.checkEqual("51. getDamage", s4->getDamage(), 155);
    a.checkEqual("52. getShield", s4->getShield(), 0);
    a.checkEqual("53. getCrew",   s4->getCrew(), 97);
    a.checkEqual("54. getOwner",  s4->getOwner(), 0);

    // - planet
    a.checkEqual("61. getDamage", p->getDamage(), 0);
    a.checkEqual("62. getShield", p->getShield(), 100);
    a.checkEqual("63. getOwner",  p->getOwner(), 1);
}

/** Test host order for ship fights, Tim-Host.
    Test case 'test2.sim' for bug #428, corresponds to c2hosttest/combat/04_order/b.
    A: set up 3 ships, lowest is passive but has low battle order
    E: verify sequence of fights is the same as generated by Host. */
AFL_TEST("game.sim.Run:VcrHost:battle-order", a)
{
    // Environment
    TestHarness h;
    h.opts.setMode(game::sim::Configuration::VcrHost, 0, h.config);
    h.opts.setRandomLeftRight(false);

    // Setup
    addShipSetup(h.setup);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result
    a.checkNonNull("01. battles", h.result.battles.get());
    a.checkEqual("02. getNumBattles", h.result.battles->getNumBattles(), 3U);

    // 320 (lowest FCBO) vs 384 (lowest other Id)
    a.checkEqual("11. getId", h.result.battles->getBattle(0)->getObject(0, false)->getId(), 384);
    a.checkEqual("12. getId", h.result.battles->getBattle(0)->getObject(1, false)->getId(), 320);

    // 320 (lowest FCBO) vs 489 (second-lowest other Id)
    a.checkEqual("21. getId", h.result.battles->getBattle(1)->getObject(0, false)->getId(), 489);
    a.checkEqual("22. getId", h.result.battles->getBattle(1)->getObject(1, false)->getId(), 320);

    // 384 (lowest other Id) vs 489 (second-lowest other Id)
    a.checkEqual("31. getId", h.result.battles->getBattle(2)->getObject(0, false)->getId(), 489);
    a.checkEqual("32. getId", h.result.battles->getBattle(2)->getObject(1, false)->getId(), 384);
}

/** Test host order for planet fights, Tim-Host.
    Test case 'test.sim' for bug #428, corresponds to c2hosttest/combat/04_order/a.
    A: set up 2 ships and a planet, planet is passive but has low battle order (not relevant in Tim-Host).
    E: verify sequence of fights is the same as generated by Host. */
AFL_TEST("game.sim.Run:VcrHost:battle-order:planet", a)
{
    // Environment
    TestHarness h;
    h.opts.setMode(game::sim::Configuration::VcrHost, 0, h.config);
    h.opts.setRandomLeftRight(false);

    // Setup
    addPlanetSetup(h.setup);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result
    a.checkNonNull("01. battles", h.result.battles.get());
    a.checkEqual("02. getNumBattles", h.result.battles->getNumBattles(), 1U);

    // 455 vs 450
    a.checkEqual("11. getId", h.result.battles->getBattle(0)->getObject(0, false)->getId(), 455);
    a.checkEqual("12. getId", h.result.battles->getBattle(0)->getObject(1, false)->getId(), 450);
}

/** Test host order for ship fights, PHost.
    Test case 'test2.sim' for bug #428, corresponds to c2hosttest/combat/04_order/b.
    A: set up 3 ships, lowest is passive but has low battle order
    E: verify sequence of fights is the same as generated by Host. */
AFL_TEST("game.sim.Run:VcrPHost4:battle-order", a)
{
    // Environment
    TestHarness h;
    h.opts.setMode(game::sim::Configuration::VcrPHost4, 0, h.config);
    h.opts.setRandomLeftRight(false);

    // Setup
    addShipSetup(h.setup);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result
    a.checkNonNull("01. battles", h.result.battles.get());
    a.checkEqual("02. getNumBattles", h.result.battles->getNumBattles(), 3U);

    // 384 (lowest other Id, as aggressor) vs 320 (lowest FCBO, as opponent)
    a.checkEqual("11. getId",   h.result.battles->getBattle(0)->getObject(0, false)->getId(), 320);
    a.checkEqual("12. getId",   h.result.battles->getBattle(0)->getObject(1, false)->getId(), 384);
    a.checkEqual("13. getRole", h.result.battles->getBattle(0)->getObject(0, false)->getRole(), game::vcr::Object::OpponentRole);
    a.checkEqual("14. getRole", h.result.battles->getBattle(0)->getObject(1, false)->getRole(), game::vcr::Object::AggressorRole);

    // 384 (lowest other Id, as aggressor) vs 489 (second-lowest other Id, as opponent)
    a.checkEqual("21. getId",   h.result.battles->getBattle(1)->getObject(0, false)->getId(), 489);
    a.checkEqual("22. getId",   h.result.battles->getBattle(1)->getObject(1, false)->getId(), 384);
    a.checkEqual("23. getRole", h.result.battles->getBattle(1)->getObject(0, false)->getRole(), game::vcr::Object::OpponentRole);
    a.checkEqual("24. getRole", h.result.battles->getBattle(1)->getObject(1, false)->getRole(), game::vcr::Object::AggressorRole);

    // 489 (second-lowest other Id, as aggressor) vs 320 (as opponent)
    a.checkEqual("31. getId",   h.result.battles->getBattle(2)->getObject(0, false)->getId(), 320);
    a.checkEqual("32. getId",   h.result.battles->getBattle(2)->getObject(1, false)->getId(), 489);
    a.checkEqual("33. getRole", h.result.battles->getBattle(2)->getObject(0, false)->getRole(), game::vcr::Object::OpponentRole);
    a.checkEqual("34. getRole", h.result.battles->getBattle(2)->getObject(1, false)->getRole(), game::vcr::Object::AggressorRole);
}

/** Test host order for planet fights, PHost.
    Test case 'test.sim' for bug #428, corresponds to c2hosttest/combat/04_order/a.
    A: set up 2 ships and a planet, planet is passive but has low battle order.
    E: verify sequence of fights is the same as generated by Host. */
AFL_TEST("game.sim.Run:VcrPHost4:battle-order:planet", a)
{
    // Environment
    TestHarness h;
    h.opts.setMode(game::sim::Configuration::VcrPHost4, 0, h.config);
    h.opts.setRandomLeftRight(false);

    // Setup
    addPlanetSetup(h.setup);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result
    a.checkNonNull("01. battles", h.result.battles.get());
    a.checkEqual("02. getNumBattles", h.result.battles->getNumBattles(), 1U);

    // 455 vs 450
    a.checkEqual("11. getId", h.result.battles->getBattle(0)->getObject(0, false)->getId(), 455);
    a.checkEqual("12. getId", h.result.battles->getBattle(0)->getObject(1, false)->getId(), 450);
}

/** Test ShieldGenerator.
    A: set up two capital ships of player 1, the second has a shield generator. Add multiple freighters.
    E: observe that shields of first ship increase. */
AFL_TEST("game.sim.Run:ship:ShieldGenerator", a)
{
    // Environment
    TestHarness h;
    h.opts.setMode(game::sim::Configuration::VcrHost, 0, h.config);
    h.opts.setRandomLeftRight(false);

    // Setup
    // - attackers
    Ship* a1 = addAnnihilation(a, h.setup, 1, 6, h.list);
    a1->setShield(10);
    a1->setBeamType(10);
    a1->setNumBeams(10);
    Ship* a2 = addAnnihilation(a, h.setup, 2, 6, h.list);
    a2->setFlags(Ship::fl_ShieldGenerator | Ship::fl_ShieldGeneratorSet);

    // - defenders
    for (int i = 0; i < 5; ++i) {
        addOutrider(a, h.setup, 10+i, 7, h.list);
    }

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.flakConfiguration, h.rng);

    // Verify result
    // THost places aggressor to the right, thus, freighters always on the left.
    a.checkNonNull("01. battles", h.result.battles.get());
    a.checkEqual("02. getNumBattles", h.result.battles->getNumBattles(), 5U);

    a.checkEqual("11. getId",     h.result.battles->getBattle(0)->getObject(1, false)->getId(), 1);
    a.checkEqual("12. getShield", h.result.battles->getBattle(0)->getObject(1, false)->getShield(), 35);
    a.checkEqual("13. getId",     h.result.battles->getBattle(0)->getObject(0, false)->getId(), 10);

    a.checkEqual("21. getId",     h.result.battles->getBattle(1)->getObject(1, false)->getId(), 1);
    a.checkEqual("22. getShield", h.result.battles->getBattle(1)->getObject(1, false)->getShield(), 60);
    a.checkEqual("23. getId",     h.result.battles->getBattle(1)->getObject(0, false)->getId(), 11);

    a.checkEqual("31. getId",     h.result.battles->getBattle(2)->getObject(1, false)->getId(), 1);
    a.checkEqual("32. getShield", h.result.battles->getBattle(2)->getObject(1, false)->getShield(), 85);
    a.checkEqual("33. getId",     h.result.battles->getBattle(2)->getObject(0, false)->getId(), 12);

    a.checkEqual("41. getId",     h.result.battles->getBattle(3)->getObject(1, false)->getId(), 1);
    a.checkEqual("42. getShield", h.result.battles->getBattle(3)->getObject(1, false)->getShield(), 110);
    a.checkEqual("43. getId",     h.result.battles->getBattle(3)->getObject(0, false)->getId(), 13);

    a.checkEqual("51. getId",     h.result.battles->getBattle(4)->getObject(1, false)->getId(), 1);
    a.checkEqual("52. getShield", h.result.battles->getBattle(4)->getObject(1, false)->getShield(), 125);  // Maximum reached
    a.checkEqual("53. getId",     h.result.battles->getBattle(4)->getObject(0, false)->getId(), 14);
}
