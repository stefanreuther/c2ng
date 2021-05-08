/**
  *  \file u/t_game_sim_run.cpp
  *  \brief Test for game::sim::Run
  */

#include "game/sim/run.hpp"

#include "t_game_sim.hpp"
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
        game::TeamSettings team;
        opts.setMode(mode, team, config);
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

    Ship* addOutrider(game::sim::Setup& setup, int id, int owner, const game::spec::ShipList& list)
    {
        Ship* ship = addShip(setup, 1, id, owner, list);
        TS_ASSERT_EQUALS(ship->getCrew(), 180);  // verify that setHullType worked as planned
        return ship;
    }

    Ship* addGorbie(game::sim::Setup& setup, int id, int owner, const game::spec::ShipList& list)
    {
        Ship* ship = addShip(setup, 70, id, owner, list);
        TS_ASSERT_EQUALS(ship->getCrew(), 2287);
        return ship;
    }

    Ship* addAnnihilation(game::sim::Setup& setup, int id, int owner, const game::spec::ShipList& list)
    {
        Ship* ship = addShip(setup, 53, id, owner, list);
        TS_ASSERT_EQUALS(ship->getCrew(), 2910);
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

    /* Common environment */
    struct TestHarness {
        game::spec::ShipList list;
        game::config::HostConfiguration config;
        util::RandomNumberGenerator rng;
        std::vector<game::vcr::Statistic> stats;
        game::sim::Configuration opts;
        game::sim::Setup setup;
        game::sim::Result result;

        TestHarness()
            : list(), config(), rng(42)
            { initShipList(list); }
    };
}

/** Test basic Host simulation.
    A: prepare two ships, Host simulation.
    E: expected results and metadata produced (verified against PCC2 playvcr). */
void
TestGameSimRun::testHost()
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    Ship* s1 = addOutrider(h.setup, 1, 12, h.list);
    Ship* s2 = addOutrider(h.setup, 2, 11, h.list);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result
    // - rng has not been touched because we use seed control
    TS_ASSERT_EQUALS(h.rng.getSeed(), 42U);

    // - a battle has been created
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 1U);
    TS_ASSERT_EQUALS(h.result.this_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.total_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.series_length, 110);
    TS_ASSERT_EQUALS(h.result.this_battle_index, 0);

    // - statistics
    TS_ASSERT_EQUALS(h.stats.size(), 2U);

    // - ship 1
    TS_ASSERT_EQUALS(s1->getDamage(), 107);
    TS_ASSERT_EQUALS(s1->getShield(), 0);
    TS_ASSERT_EQUALS(s1->getCrew(), 103);
    TS_ASSERT_EQUALS(s1->getOwner(), 0);

    // - ship 2
    TS_ASSERT_EQUALS(s2->getDamage(), 82);
    TS_ASSERT_EQUALS(s2->getShield(), 0);
    TS_ASSERT_EQUALS(s2->getCrew(), 121);
    TS_ASSERT_EQUALS(s2->getOwner(), 11);
}

/** Test basic Host simulation, big ships.
    A: prepare two ships, Host simulation.
    E: expected results and metadata produced (verified against PCC2 playvcr). */
void
TestGameSimRun::testHostBig()
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    Ship* s1 = addGorbie(h.setup, 1, 8, h.list);
    Ship* s2 = addAnnihilation(h.setup, 2, 6, h.list);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result
    // - rng has not been touched because we use seed control
    TS_ASSERT_EQUALS(h.rng.getSeed(), 42U);

    // - a battle has been created
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 1U);
    TS_ASSERT_EQUALS(h.result.this_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.total_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.series_length, 110);
    TS_ASSERT_EQUALS(h.result.this_battle_index, 0);

    // - statistics
    TS_ASSERT_EQUALS(h.stats.size(), 2U);
    TS_ASSERT_EQUALS(h.stats[0].getMinFightersAboard(), 201);
    TS_ASSERT_EQUALS(h.stats[0].getNumTorpedoHits(), 0);
    TS_ASSERT_EQUALS(h.stats[1].getMinFightersAboard(), 0);
    TS_ASSERT_EQUALS(h.stats[1].getNumTorpedoHits(), 29);

    // - ship 1
    TS_ASSERT_EQUALS(s1->getDamage(), 38);
    TS_ASSERT_EQUALS(s1->getShield(), 0);
    TS_ASSERT_EQUALS(s1->getCrew(), 2173);
    TS_ASSERT_EQUALS(s1->getOwner(), 8);

    // - ship 2
    TS_ASSERT_EQUALS(s2->getDamage(), 102);
    TS_ASSERT_EQUALS(s2->getShield(), 0);
    TS_ASSERT_EQUALS(s2->getCrew(), 2880);
    TS_ASSERT_EQUALS(s2->getOwner(), 0);
}

/** Test basic Host simulation, NTP.
    A: prepare two ships, Host simulation, one with NTP.
    E: expected results and metadata produced (verified against PCC2 playvcr). */
void
TestGameSimRun::testHostNoTorps()
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    Ship* s1 = addAnnihilation(h.setup, 1, 6, h.list);
    Ship* s2 = addAnnihilation(h.setup, 2, 2, h.list);
    s2->setFriendlyCode("NTP");
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result
    // - rng has not been touched because we use seed control
    TS_ASSERT_EQUALS(h.rng.getSeed(), 42U);

    // - a battle has been created
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 1U);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(0, false)->getId(), 2);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(0, false)->getNumTorpedoes(), 0);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(1, false)->getId(), 1);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(1, false)->getNumTorpedoes(), 320);
    TS_ASSERT_EQUALS(h.result.this_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.total_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.series_length, 110);
    TS_ASSERT_EQUALS(h.result.this_battle_index, 0);

    // - statistics
    TS_ASSERT_EQUALS(h.stats.size(), 2U);
    TS_ASSERT_EQUALS(h.stats[0].getMinFightersAboard(), 0);
    TS_ASSERT_EQUALS(h.stats[0].getNumTorpedoHits(), 72);
    TS_ASSERT_EQUALS(h.stats[1].getMinFightersAboard(), 0);
    TS_ASSERT_EQUALS(h.stats[1].getNumTorpedoHits(), 0);

    // - ship 1
    TS_ASSERT_EQUALS(s1->getDamage(), 2);
    TS_ASSERT_EQUALS(s1->getShield(), 0);
    TS_ASSERT_EQUALS(s1->getCrew(), 2907);
    TS_ASSERT_EQUALS(s1->getOwner(), 6);

    // - ship 2
    TS_ASSERT_EQUALS(s2->getDamage(), 153);
    TS_ASSERT_EQUALS(s2->getShield(), 0);
    TS_ASSERT_EQUALS(s2->getCrew(), 2483);
    TS_ASSERT_EQUALS(s2->getOwner(), 0);
}

/** Test Host simulation, balancing mode "360 kt".
    A: prepare two ships, Host simulation.
    E: expected results and metadata produced (verified against PCC2 playvcr). */
void
TestGameSimRun::testHostBalance()
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::Balance360k);

    // Setup
    Ship* s1 = addOutrider(h.setup, 1, 12, h.list);
    Ship* s2 = addOutrider(h.setup, 2, 11, h.list);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result
    // - rng has not been touched because we use seed control
    TS_ASSERT_EQUALS(h.rng.getSeed(), 42U);

    // - a battle has been created - increased weight due to balancing
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 1U);
    TS_ASSERT_EQUALS(h.result.this_battle_weight, 50);
    TS_ASSERT_EQUALS(h.result.total_battle_weight, 100);
    TS_ASSERT_EQUALS(h.result.series_length, 220);             // doubled by Balance360k
    TS_ASSERT_EQUALS(h.result.this_battle_index, 0);

    // - statistics
    TS_ASSERT_EQUALS(h.stats.size(), 2U);

    // - ship 1
    TS_ASSERT_EQUALS(s1->getDamage(), 107);
    TS_ASSERT_EQUALS(s1->getShield(), 0);
    TS_ASSERT_EQUALS(s1->getCrew(), 103);
    TS_ASSERT_EQUALS(s1->getOwner(), 0);

    // - ship 2
    TS_ASSERT_EQUALS(s2->getDamage(), 82);
    TS_ASSERT_EQUALS(s2->getShield(), 0);
    TS_ASSERT_EQUALS(s2->getCrew(), 121);
    TS_ASSERT_EQUALS(s2->getOwner(), 11);
}

/** Test Host simulation, balancing mode "Master at Arms".
    A: prepare two ships, Host simulation.
    E: expected results and metadata produced (verified against PCC2 playvcr). */
void
TestGameSimRun::testHostMaster()
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceMasterAtArms);

    // Setup
    Ship* s1 = addGorbie(h.setup, 1, 8, h.list);
    Ship* s2 = addGorbie(h.setup, 2, 6, h.list);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result
    // - rng has not been touched because we use seed control
    TS_ASSERT_EQUALS(h.rng.getSeed(), 42U);

    // - a battle has been created - increased weight due to balancing
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 1U);
    TS_ASSERT_EQUALS(h.result.this_battle_weight, 28);
    TS_ASSERT_EQUALS(h.result.total_battle_weight, 1000);
    TS_ASSERT_EQUALS(h.result.series_length, 440);             // doubled by bonus bays and by bonus fighters
    TS_ASSERT_EQUALS(h.result.this_battle_index, 0);

    // - statistics
    TS_ASSERT_EQUALS(h.stats.size(), 2U);
    TS_ASSERT_EQUALS(h.stats[0].getMinFightersAboard(), 146);
    TS_ASSERT_EQUALS(h.stats[0].getNumTorpedoHits(), 0);
    TS_ASSERT_EQUALS(h.stats[1].getMinFightersAboard(), 167);
    TS_ASSERT_EQUALS(h.stats[1].getNumTorpedoHits(), 0);

    // - ship 1
    TS_ASSERT_EQUALS(s1->getDamage(), 102);
    TS_ASSERT_EQUALS(s1->getShield(), 0);
    TS_ASSERT_EQUALS(s1->getCrew(), 2287);
    TS_ASSERT_EQUALS(s1->getOwner(), 0);
    TS_ASSERT_EQUALS(s1->getAmmo(), 151);

    // - ship 2
    TS_ASSERT_EQUALS(s2->getDamage(), 0);
    TS_ASSERT_EQUALS(s2->getShield(), 50);
    TS_ASSERT_EQUALS(s2->getCrew(), 2287);
    TS_ASSERT_EQUALS(s2->getOwner(), 6);
    TS_ASSERT_EQUALS(s2->getAmmo(), 175);
}

/** Test Host simulation, planet.
    A: prepare ships and planet, Host simulation.
    E: expected results and metadata produced (verified against PCC2 playvcr). */
void
TestGameSimRun::testHostPlanet()
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    Ship* s = addOutrider(h.setup, 1, 5, h.list);
    Planet* p = addPlanet(h.setup, 1, 4);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result
    // - rng has not been touched because we use seed control
    TS_ASSERT_EQUALS(h.rng.getSeed(), 42U);

    // - a battle has been created
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 1U);
    TS_ASSERT_EQUALS(h.result.this_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.total_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.series_length, 110);
    TS_ASSERT_EQUALS(h.result.this_battle_index, 0);

    // - statistics
    TS_ASSERT_EQUALS(h.stats.size(), 2U);

    // - ship 1
    TS_ASSERT_EQUALS(s->getDamage(), 103);
    TS_ASSERT_EQUALS(s->getShield(), 0);
    TS_ASSERT_EQUALS(s->getCrew(), 128);
    TS_ASSERT_EQUALS(s->getOwner(), 0);

    // - ship 2
    TS_ASSERT_EQUALS(p->getDamage(), 0);
    TS_ASSERT_EQUALS(p->getShield(), 100);
    TS_ASSERT_EQUALS(p->getOwner(), 4);
}

/** Test Host simulation, intercept-attack.
    A: prepare four ships, with two of them intercepting one, Host simulation.
    E: expected results and metadata produced (verified against PCC2 playvcr). */
void
TestGameSimRun::testHostIntercept()
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    /*Ship* s1 =*/ addOutrider(h.setup, 1, 1, h.list);
    Ship* s2 = addOutrider(h.setup, 2, 2, h.list);
    Ship* s3 = addOutrider(h.setup, 3, 3, h.list);
    Ship* s4 = addOutrider(h.setup, 4, 4, h.list);
    s3->setAggressiveness(2);
    s3->setInterceptId(2);
    s3->setFriendlyCode("200");
    s4->setAggressiveness(2);
    s4->setInterceptId(2);
    s4->setFriendlyCode("100");

    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result
    // - rng has not been touched because we use seed control
    TS_ASSERT_EQUALS(h.rng.getSeed(), 42U);

    // - a battle has been created
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(0, false)->getId(), 2);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(1, false)->getId(), 4);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 1U);
    TS_ASSERT_EQUALS(h.result.this_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.total_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.series_length, 110);
    TS_ASSERT_EQUALS(h.result.this_battle_index, 0);

    // - statistics
    TS_ASSERT_EQUALS(h.stats.size(), 4U);

    // - ship 2
    TS_ASSERT_EQUALS(s2->getDamage(), 82);
    TS_ASSERT_EQUALS(s2->getShield(), 0);
    TS_ASSERT_EQUALS(s2->getCrew(), 121);
    TS_ASSERT_EQUALS(s2->getOwner(), 2);

    // - ship 4
    TS_ASSERT_EQUALS(s4->getDamage(), 107);
    TS_ASSERT_EQUALS(s4->getShield(), 0);
    TS_ASSERT_EQUALS(s4->getCrew(), 103);
    TS_ASSERT_EQUALS(s4->getOwner(), 0);
}

/** Test multi-ship Host simulation.
    A: prepare multiple ships, Host simulation.
    E: expected results and metadata produced. Expected battle order produced. This is a regression test to ensure constant behaviour. */
void
TestGameSimRun::testHostMulti()
{
    // Environment
    TestHarness h;
    game::TeamSettings team;
    h.opts.setMode(game::sim::Configuration::VcrHost, team, h.config);

    // Setup
    Ship* s1 = addOutrider(h.setup, 1, 1, h.list);
    Ship* s2 = addOutrider(h.setup, 2, 2, h.list);
    Ship* s3 = addOutrider(h.setup, 3, 2, h.list);
    Ship* s4 = addOutrider(h.setup, 4, 2, h.list);
    Planet* p = addPlanet(h.setup, 17, 1);
    s1->setFriendlyCode("-20");
    s2->setFriendlyCode("100");
    s3->setFriendlyCode("300");
    s4->setFriendlyCode("200");
    p->setFriendlyCode("ATT");

    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result
    // - rng has been used
    TS_ASSERT_EQUALS(h.rng.getSeed(), 673767206U);

    // - battles have been created; series length unchanged
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 4U);
    TS_ASSERT_EQUALS(h.result.this_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.total_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.series_length, 110);
    TS_ASSERT_EQUALS(h.result.this_battle_index, 0);
    
    // - first battle (#2 is aggressor, #1 wins)
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(0, false)->getId(), 1);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(1, false)->getId(), 2);

    // - second battle (#4 is aggressor, #4 wins)
    TS_ASSERT_EQUALS(h.result.battles->getBattle(1)->getObject(0, false)->getId(), 1);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(1)->getObject(1, false)->getId(), 4);

    // - third battle (#4 is aggressor, #17 wins)
    TS_ASSERT_EQUALS(h.result.battles->getBattle(2)->getObject(0, false)->getId(), 4);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(2)->getObject(1, false)->getId(), 17);

    // - fourth battle (#3 is aggressor, #17 wins)
    TS_ASSERT_EQUALS(h.result.battles->getBattle(3)->getObject(0, false)->getId(), 3);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(3)->getObject(1, false)->getId(), 17);

    // - statistics
    TS_ASSERT_EQUALS(h.stats.size(), 5U);

    // - ship 1
    TS_ASSERT_EQUALS(s1->getDamage(), 110);
    TS_ASSERT_EQUALS(s1->getShield(), 0);
    TS_ASSERT_EQUALS(s1->getCrew(), 47);
    TS_ASSERT_EQUALS(s1->getOwner(), 0);

    // - ship 2
    TS_ASSERT_EQUALS(s2->getDamage(), 162);
    TS_ASSERT_EQUALS(s2->getShield(), 0);
    TS_ASSERT_EQUALS(s2->getCrew(), 65);
    TS_ASSERT_EQUALS(s2->getOwner(), 0);

    // - ship 3
    TS_ASSERT_EQUALS(s3->getDamage(), 159);
    TS_ASSERT_EQUALS(s3->getShield(), 0);
    TS_ASSERT_EQUALS(s3->getCrew(), 100);
    TS_ASSERT_EQUALS(s3->getOwner(), 0);

    // - ship 4
    TS_ASSERT_EQUALS(s4->getDamage(), 168);
    TS_ASSERT_EQUALS(s4->getShield(), 0);
    TS_ASSERT_EQUALS(s4->getCrew(), 73);
    TS_ASSERT_EQUALS(s4->getOwner(), 0);

    // - planet
    TS_ASSERT_EQUALS(p->getDamage(), 0);
    TS_ASSERT_EQUALS(p->getShield(), 100);
    TS_ASSERT_EQUALS(p->getOwner(), 1);
}

/** Test Host simulation with Engine/Shield bonus.
    A: prepare two ships with different engines, Host simulation, ESB 20%.
    E: expected results and metadata produced (verified against PCC2 playvcr). */
void
TestGameSimRun::testHostESB()
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);
    h.opts.setEngineShieldBonus(20);

    // Setup
    Ship* s1 = addOutrider(h.setup, 1, 6, h.list);
    Ship* s2 = addOutrider(h.setup, 2, 9, h.list);
    h.result.init(h.opts, 0);
    s1->setEngineType(5);          // Nova Drive 5, 5 kt bonus
    s2->setEngineType(9);          // Transwarp Drive, 60 kt bonus

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result
    // - rng has not been touched because we use seed control
    TS_ASSERT_EQUALS(h.rng.getSeed(), 42U);

    // - a battle has been created
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 1U);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(0, false)->getOwner(), 9);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(0, false)->getMass(), 135);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(1, false)->getOwner(), 6);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(1, false)->getMass(), 80);
    TS_ASSERT_EQUALS(h.result.this_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.total_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.series_length, 110);
    TS_ASSERT_EQUALS(h.result.this_battle_index, 0);

    // - statistics
    TS_ASSERT_EQUALS(h.stats.size(), 2U);

    // - ship 1
    TS_ASSERT_EQUALS(s1->getDamage(), 119);
    TS_ASSERT_EQUALS(s1->getShield(), 0);
    TS_ASSERT_EQUALS(s1->getCrew(), 89);
    TS_ASSERT_EQUALS(s1->getOwner(), 0);

    // - ship 2
    TS_ASSERT_EQUALS(s2->getDamage(), 12);
    TS_ASSERT_EQUALS(s2->getShield(), 0);
    TS_ASSERT_EQUALS(s2->getCrew(), 158);
    TS_ASSERT_EQUALS(s2->getOwner(), 9);
}

/** Test basic PHost simulation.
    A: prepare two ships, PHost simulation.
    E: expected results and metadata produced (verified against PCC2 playvcr). */
void
TestGameSimRun::testPHost()
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrPHost4, game::sim::Configuration::BalanceNone);
    h.opts.setRandomLeftRight(true);

    // Setup
    Ship* s1 = addOutrider(h.setup, 1, 12, h.list);
    Ship* s2 = addOutrider(h.setup, 2, 11, h.list);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result
    // - rng has not been touched because we use seed control
    TS_ASSERT_EQUALS(h.rng.getSeed(), 42U);

    // - a battle has been created
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 1U);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(0, false)->getOwner(), 12);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(1, false)->getOwner(), 11);

    TS_ASSERT_EQUALS(h.result.this_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.total_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.series_length, 220);          // doubled by random left/right
    TS_ASSERT_EQUALS(h.result.this_battle_index, 0);

    // - statistics
    TS_ASSERT_EQUALS(h.stats.size(), 2U);

    // - ship 1
    TS_ASSERT_EQUALS(s1->getDamage(), 100);
    TS_ASSERT_EQUALS(s1->getShield(), 0);
    TS_ASSERT_EQUALS(s1->getCrew(), 132);
    TS_ASSERT_EQUALS(s1->getOwner(), 0);

    // - ship 2
    TS_ASSERT_EQUALS(s2->getDamage(), 70);
    TS_ASSERT_EQUALS(s2->getShield(), 0);
    TS_ASSERT_EQUALS(s2->getCrew(), 132);
    TS_ASSERT_EQUALS(s2->getOwner(), 11);
}

/** Test basic Host simulation, big ships.
    A: prepare two ships, PHost simulation.
    E: expected results and metadata produced (verified against PCC2 playvcr). */
void
TestGameSimRun::testPHostBig()
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrPHost3, game::sim::Configuration::BalanceNone);

    // Setup
    Ship* s1 = addGorbie(h.setup, 1, 8, h.list);
    Ship* s2 = addAnnihilation(h.setup, 2, 6, h.list);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result
    // - rng has not been touched because we use seed control
    TS_ASSERT_EQUALS(h.rng.getSeed(), 42U);

    // - a battle has been created
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 1U);
    TS_ASSERT_EQUALS(h.result.this_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.total_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.series_length, 110);
    TS_ASSERT_EQUALS(h.result.this_battle_index, 0);

    // - statistics
    TS_ASSERT_EQUALS(h.stats.size(), 2U);
    TS_ASSERT_EQUALS(h.stats[0].getMinFightersAboard(), 210);
    TS_ASSERT_EQUALS(h.stats[0].getNumTorpedoHits(), 0);
    TS_ASSERT_EQUALS(h.stats[1].getMinFightersAboard(), 0);
    TS_ASSERT_EQUALS(h.stats[1].getNumTorpedoHits(), 29);

    // - ship 1
    TS_ASSERT_EQUALS(s1->getDamage(), 38);
    TS_ASSERT_EQUALS(s1->getShield(), 0);
    TS_ASSERT_EQUALS(s1->getCrew(), 2173);
    TS_ASSERT_EQUALS(s1->getOwner(), 8);

    // - ship 2
    TS_ASSERT_EQUALS(s2->getDamage(), 100);
    TS_ASSERT_EQUALS(s2->getShield(), 0);
    TS_ASSERT_EQUALS(s2->getCrew(), 2902);
    TS_ASSERT_EQUALS(s2->getOwner(), 0);
}


/** Test PHost simulation, planet.
    A: prepare ships and planet, PHost simulation.
    E: expected results and metadata produced (verified against PCC2 playvcr). */
void
TestGameSimRun::testPHostPlanet()
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrPHost4, game::sim::Configuration::BalanceNone);

    // Setup
    Ship* s = addOutrider(h.setup, 1, 5, h.list);
    Planet* p = addPlanet(h.setup, 1, 4);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result
    // - rng has not been touched because we use seed control
    TS_ASSERT_EQUALS(h.rng.getSeed(), 42U);

    // - a battle has been created
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 1U);
    TS_ASSERT_EQUALS(h.result.this_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.total_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.series_length, 110);
    TS_ASSERT_EQUALS(h.result.this_battle_index, 0);

    // - statistics
    TS_ASSERT_EQUALS(h.stats.size(), 2U);
    TS_ASSERT_EQUALS(h.stats[0].getMinFightersAboard(), 0);
    TS_ASSERT_EQUALS(h.stats[0].getNumTorpedoHits(), 0);
    TS_ASSERT_EQUALS(h.stats[1].getMinFightersAboard(), 0);
    TS_ASSERT_EQUALS(h.stats[1].getNumTorpedoHits(), 0);

    // - ship 1
    TS_ASSERT_EQUALS(s->getDamage(), 100);
    TS_ASSERT_EQUALS(s->getShield(), 0);
    TS_ASSERT_EQUALS(s->getCrew(), 131);
    TS_ASSERT_EQUALS(s->getOwner(), 0);

    // - ship 2
    TS_ASSERT_EQUALS(p->getDamage(), 0);
    TS_ASSERT_EQUALS(p->getShield(), 100);
    TS_ASSERT_EQUALS(p->getOwner(), 4);
}

/** Test PHost simulation, planet with torpedo tubes.
    A: prepare ships and planet, set PlanetsHaveTubes=Yes, PHost simulation.
    E: expected results and metadata produced (verified against PCC2 playvcr). */
void
TestGameSimRun::testPHostPlanetTubes()
{
    // Environment
    TestHarness h;
    h.config[game::config::HostConfiguration::PlanetsHaveTubes].set(true);
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrPHost4, game::sim::Configuration::BalanceNone);

    // Setup
    Ship* s = addAnnihilation(h.setup, 1, 6, h.list);
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
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result
    // - rng has not been touched because we use seed control
    TS_ASSERT_EQUALS(h.rng.getSeed(), 42U);

    // - a battle has been created
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(0, false)->getNumTorpedoes(), 320);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(0, false)->getNumFighters(), 0);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(1, false)->getNumTorpedoes(), 72);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(1, false)->getNumFighters(), 48);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 1U);
    TS_ASSERT_EQUALS(h.result.this_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.total_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.series_length, 110);
    TS_ASSERT_EQUALS(h.result.this_battle_index, 0);

    // - statistics
    TS_ASSERT_EQUALS(h.stats.size(), 2U);
    TS_ASSERT_EQUALS(h.stats[0].getMinFightersAboard(), 0);
    TS_ASSERT_EQUALS(h.stats[0].getNumTorpedoHits(), 16);
    TS_ASSERT_EQUALS(h.stats[1].getMinFightersAboard(), 0);
    // FIXME: missing -> TS_ASSERT_EQUALS(h.stats[1].getNumTorpedoHits(), 0);

    // - ship 1
    TS_ASSERT_EQUALS(s->getDamage(), 100);
    TS_ASSERT_EQUALS(s->getShield(), 0);
    TS_ASSERT_EQUALS(s->getCrew(), 2884);
    TS_ASSERT_EQUALS(s->getOwner(), 0);
    TS_ASSERT_EQUALS(s->getAmmo(), 290);

    // - ship 2
    TS_ASSERT_EQUALS(p->getDamage(), 84);
    TS_ASSERT_EQUALS(p->getShield(), 0);
    TS_ASSERT_EQUALS(p->getOwner(), 9);

    /* Existing torpedoes are worth 20*12 + 30*13 = 630 mc = 48 torpedoes effectively,
       plus 3*8 = 24 from PlanetaryTorpsPerTube = 78 total.
       We fire 24 torpedoes = 312 mc worth,
       and thus remove ceil(312 / (12+13)) = 13 of each. */
    TS_ASSERT_EQUALS(p->getNumBaseTorpedoes(5), 7);
    TS_ASSERT_EQUALS(p->getNumBaseTorpedoes(6), 17);
}

/** Test PHost simulation, intercept-attack.
    A: prepare four ships, with two of them intercepting one, PHost simulation.
    E: expected results and metadata produced (verified against PCC2 playvcr). */
void
TestGameSimRun::testPHostIntercept()
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrPHost4, game::sim::Configuration::BalanceNone);

    // Setup
    /*Ship* s1 =*/ addOutrider(h.setup, 1, 1, h.list);
    Ship* s2 = addOutrider(h.setup, 2, 2, h.list);
    Ship* s3 = addOutrider(h.setup, 3, 3, h.list);
    Ship* s4 = addOutrider(h.setup, 4, 4, h.list);
    s3->setAggressiveness(2);
    s3->setInterceptId(2);
    s3->setFriendlyCode("200");
    s4->setAggressiveness(2);
    s4->setInterceptId(2);
    s4->setFriendlyCode("100");

    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result
    // - rng has not been touched because we use seed control
    TS_ASSERT_EQUALS(h.rng.getSeed(), 42U);

    // - a battle has been created
    // Note that as of 20200923, this result is DIFFERENT from PCC2 2.0.9:
    // PCC2 places the interceptor on the left side, whereas we place them on the right (same as in THost and c2web).
    // This is not a difference from actual host behaviour because PHost always randomizes sides;
    // this test only disabled random left/right for determinism of test results.
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(0, false)->getId(), 2);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(1, false)->getId(), 4);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 1U);
    TS_ASSERT_EQUALS(h.result.this_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.total_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.series_length, 110);
    TS_ASSERT_EQUALS(h.result.this_battle_index, 0);

    // - statistics
    TS_ASSERT_EQUALS(h.stats.size(), 4U);

    // - ship 2
    TS_ASSERT_EQUALS(s2->getDamage(), 100);
    TS_ASSERT_EQUALS(s2->getShield(), 0);
    TS_ASSERT_EQUALS(s2->getCrew(), 110);
    TS_ASSERT_EQUALS(s2->getOwner(), 0);

    // - ship 4
    TS_ASSERT_EQUALS(s4->getDamage(), 100);
    TS_ASSERT_EQUALS(s4->getShield(), 0);
    TS_ASSERT_EQUALS(s4->getCrew(), 132);
    TS_ASSERT_EQUALS(s4->getOwner(), 0);
}

/** Test multi-ship PHost simulation.
    A: prepare multiple ships, PHost simulation.
    E: expected results and metadata produced. Expected battle order produced. This is a regression test to ensure constant behaviour. */
void
TestGameSimRun::testPHostMulti()
{
    // Environment
    TestHarness h;
    game::TeamSettings team;
    h.opts.setMode(game::sim::Configuration::VcrPHost2, team, h.config);

    // Setup
    Ship* s1 = addOutrider(h.setup, 1, 1, h.list);
    Ship* s2 = addOutrider(h.setup, 2, 2, h.list);
    Ship* s3 = addOutrider(h.setup, 3, 2, h.list);
    Ship* s4 = addOutrider(h.setup, 4, 2, h.list);
    Planet* p = addPlanet(h.setup, 17, 1);
    s1->setFriendlyCode("-20");
    s2->setFriendlyCode("100");
    s3->setFriendlyCode("300");
    s4->setFriendlyCode("200");
    p->setFriendlyCode("ATT");

    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result
    // - rng has been used
    TS_ASSERT_EQUALS(h.rng.getSeed(), 3638705852U);

    // - battles have been created; series length unchanged
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 4U);
    TS_ASSERT_EQUALS(h.result.this_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.total_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.series_length, 110);
    TS_ASSERT_EQUALS(h.result.this_battle_index, 0);
    
    // - first battle (#1 is aggressor, #1 wins)
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(0, false)->getId(), 1);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(1, false)->getId(), 2);

    // - second battle (#4 is aggressor, #4 wins)
    TS_ASSERT_EQUALS(h.result.battles->getBattle(1)->getObject(0, false)->getId(), 1);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(1)->getObject(1, false)->getId(), 4);

    // - third battle (#4 is aggressor, #17 wins)
    TS_ASSERT_EQUALS(h.result.battles->getBattle(2)->getObject(0, false)->getId(), 4);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(2)->getObject(1, false)->getId(), 17);

    // - fourth battle (#3 is aggressor, #17 wins)
    TS_ASSERT_EQUALS(h.result.battles->getBattle(3)->getObject(0, false)->getId(), 3);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(3)->getObject(1, false)->getId(), 17);

    // - statistics
    TS_ASSERT_EQUALS(h.stats.size(), 5U);

    // - ship 1
    TS_ASSERT_EQUALS(s1->getDamage(), 100);
    TS_ASSERT_EQUALS(s1->getShield(), 0);
    TS_ASSERT_EQUALS(s1->getCrew(), 84);
    TS_ASSERT_EQUALS(s1->getOwner(), 0);

    // - ship 2
    TS_ASSERT_EQUALS(s2->getDamage(), 100);
    TS_ASSERT_EQUALS(s2->getShield(), 0);
    TS_ASSERT_EQUALS(s2->getCrew(), 88);
    TS_ASSERT_EQUALS(s2->getOwner(), 0);

    // - ship 3
    TS_ASSERT_EQUALS(s3->getDamage(), 100);
    TS_ASSERT_EQUALS(s3->getShield(), 0);
    TS_ASSERT_EQUALS(s3->getCrew(), 107);
    TS_ASSERT_EQUALS(s3->getOwner(), 0);

    // - ship 4
    TS_ASSERT_EQUALS(s4->getDamage(), 100);
    TS_ASSERT_EQUALS(s4->getShield(), 0);
    TS_ASSERT_EQUALS(s4->getCrew(), 94);
    TS_ASSERT_EQUALS(s4->getOwner(), 0);

    // - planet
    TS_ASSERT_EQUALS(p->getDamage(), 0);
    TS_ASSERT_EQUALS(p->getShield(), 100);
    TS_ASSERT_EQUALS(p->getOwner(), 1);
}

/** Test PHost simulation, with commanders.
    A: prepare multiple ships including a Commander, PHost simulation.
    E: expected results and metadata produced (verified against PCC2 playvcr). */
void
TestGameSimRun::testShipCommander()
{
    // Environment
    TestHarness h;
    h.config[game::config::HostConfiguration::NumExperienceLevels].set(4);
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrPHost4, game::sim::Configuration::BalanceNone);

    // Setup
    Ship* s1 = addOutrider(h.setup, 1, 1, h.list);
    Ship* s2 = addOutrider(h.setup, 2, 2, h.list);
    Ship* s3 = addOutrider(h.setup, 3, 2, h.list);
    s1->setAggressiveness(Ship::agg_Passive);
    s2->setAggressiveness(Ship::agg_Kill);
    s3->setAggressiveness(Ship::agg_Passive);
    s3->setExperienceLevel(3);
    s3->setFlags(Ship::fl_Commander | Ship::fl_CommanderSet);

    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result
    // - rng has not been used
    TS_ASSERT_EQUALS(h.rng.getSeed(), 42U);

    // - battles have been created
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 1U);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(0, false)->getId(), 1);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(0, false)->getExperienceLevel(), 0);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(1, false)->getId(), 2);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(1, false)->getExperienceLevel(), 1);
    TS_ASSERT_EQUALS(h.result.this_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.total_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.series_length, 110);
    TS_ASSERT_EQUALS(h.result.this_battle_index, 0);
    
    // - statistics
    TS_ASSERT_EQUALS(h.stats.size(), 3U);
    TS_ASSERT_EQUALS(h.stats[0].getNumFights(), 1);
    TS_ASSERT_EQUALS(h.stats[1].getNumFights(), 1);
    TS_ASSERT_EQUALS(h.stats[2].getNumFights(), 0);

    // - ship 1
    TS_ASSERT_EQUALS(s1->getDamage(), 37);
    TS_ASSERT_EQUALS(s1->getShield(), 0);
    TS_ASSERT_EQUALS(s1->getCrew(), 140);
    TS_ASSERT_EQUALS(s1->getOwner(), 1);

    // - ship 2
    TS_ASSERT_EQUALS(s2->getDamage(), 100);
    TS_ASSERT_EQUALS(s2->getShield(), 0);
    TS_ASSERT_EQUALS(s2->getCrew(), 92);
    TS_ASSERT_EQUALS(s2->getOwner(), 0);
}

/** Test deactivated ship.
    A: prepare two ships, one deactivated.
    E: no fight happens. */
void
TestGameSimRun::testShipDeactivated()
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    // As of 20200920, setting an Intercept Id will try to match the ships even though #1 is not part of battle order due to being disabled.
    addOutrider(h.setup, 1, 12, h.list)->setFlags(Ship::fl_Deactivated);
    addOutrider(h.setup, 2, 11, h.list)->setInterceptId(1);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result: no fight
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 0U);
}

/** Test allied ships.
    A: prepare two ships, bidirectional alliance.
    E: no fight happens. */
void
TestGameSimRun::testShipAllied()
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);
    h.opts.allianceSettings().set(11, 12, true);
    h.opts.allianceSettings().set(12, 11, true);

    // Setup
    addOutrider(h.setup, 1, 12, h.list);
    addOutrider(h.setup, 2, 11, h.list);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result: no fight
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 0U);
}

/** Test passive ships.
    A: prepare two ships, passive.
    E: no fight happens. */
void
TestGameSimRun::testShipPassive()
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    addOutrider(h.setup, 1, 12, h.list)->setAggressiveness(Ship::agg_Passive);
    addOutrider(h.setup, 2, 11, h.list)->setAggressiveness(Ship::agg_Passive);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result: no fight
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 0U);
}

/** Test non-hostile ships.
    A: prepare two ships, mismatching primary enemy.
    E: no fight happens. */
void
TestGameSimRun::testShipNotEnemy()
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    addOutrider(h.setup, 1, 12, h.list)->setAggressiveness(7);
    addOutrider(h.setup, 2, 11, h.list)->setAggressiveness(2);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result: no fight
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 0U);
}

/** Test hostile ships.
    A: prepare two ships, one passive, one with primary enemy.
    E: fight happens. */
void
TestGameSimRun::testShipEnemy()
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    addOutrider(h.setup, 1, 12, h.list)->setAggressiveness(11);
    addOutrider(h.setup, 2, 11, h.list)->setAggressiveness(2);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result: no fight
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 1U);
}

/** Test hostile ships, via persistent enemies.
    A: prepare two ships, one passive, one with mismatching primary enemy but persistent enemy setting
    E: fight happens. */
void
TestGameSimRun::testShipPersistentEnemy()
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);
    h.opts.enemySettings().set(11, 12, true);

    // Setup
    addOutrider(h.setup, 1, 12, h.list)->setAggressiveness(5);
    addOutrider(h.setup, 2, 11, h.list)->setAggressiveness(2);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result: no fight
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 1U);
}

/** Test cloaked ships.
    A: prepare two ships, one cloaked.
    E: no fight happens. */
void
TestGameSimRun::testShipCloaked()
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);
    h.config[game::config::HostConfiguration::AllowCloakedShipsAttack].set(0);

    // Setup
    addOutrider(h.setup, 1, 12, h.list)->setFlags(Ship::fl_Cloaked);
    addOutrider(h.setup, 2, 11, h.list);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result: no fight
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 0U);
}

/** Test ships, matching friendly codes.
    A: prepare two ships with matching friendly codes.
    E: no fight happens. */
void
TestGameSimRun::testShipFriendlyCodeMatch()
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    addOutrider(h.setup, 1, 12, h.list)->setFriendlyCode("abc");
    addOutrider(h.setup, 2, 11, h.list)->setFriendlyCode("abc");
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result: no fight
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 0U);
}

/** Test ships, no fuel.
    A: prepare two ships, one with no fuel.
    E: no fight happens. */
void
TestGameSimRun::testShipNoFuel()
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    addOutrider(h.setup, 1, 12, h.list)->setAggressiveness(Ship::agg_NoFuel);
    addOutrider(h.setup, 2, 11, h.list);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result: no fight
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 0U);
}

/** Test ships, Cloaked Fighter Bays ability.
    A: prepare three ships; one passive with Cloaked Fighter Bays ability.
    E: expected results and metadata produced (verified against PCC2 playvcr). */
void
TestGameSimRun::testShipCloakedFighterBays()
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrNuHost, game::sim::Configuration::BalanceNone);

    // Setup
    Ship* s1 = addGorbie(h.setup, 1, 8, h.list);
    Ship* s2 = addGorbie(h.setup, 2, 4, h.list);
    Ship* s3 = addGorbie(h.setup, 3, 8, h.list);
    h.result.init(h.opts, 0);
    s3->setAggressiveness(Ship::agg_Passive);
    s3->setFlags(Ship::fl_Cloaked | Ship::fl_CloakedBays | Ship::fl_CloakedBaysSet);
    // This line is not needed if Klingon ships automatically have DoubleBeamChargeAbility in NuHost:
    // s2->setFlags(Ship::fl_DoubleBeamCharge | Ship::fl_DoubleBeamChargeSet);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result
    // - rng has not been touched because we use seed control
    TS_ASSERT_EQUALS(h.rng.getSeed(), 42U);

    // - a battle has been created
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 1U);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(0, false)->getId(), 2);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(0, false)->getNumBays(), 10);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(0, false)->getNumFighters(), 250);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(1, false)->getId(), 1);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(1, false)->getNumBays(), 20);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(1, false)->getNumFighters(), 500);
    TS_ASSERT_EQUALS(h.result.this_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.total_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.series_length, 118);
    TS_ASSERT_EQUALS(h.result.this_battle_index, 0);

    // - statistics
    TS_ASSERT_EQUALS(h.stats.size(), 3U);
    TS_ASSERT_EQUALS(h.stats[0].getNumFights(), 1);
    TS_ASSERT_EQUALS(h.stats[1].getNumFights(), 1);
    TS_ASSERT_EQUALS(h.stats[2].getNumFights(), 0);

    // - ship 1
    TS_ASSERT_EQUALS(s1->getDamage(), 9);
    TS_ASSERT_EQUALS(s1->getShield(), 0);
    TS_ASSERT_EQUALS(s1->getCrew(), 2287);
    TS_ASSERT_EQUALS(s1->getOwner(), 8);
    TS_ASSERT_EQUALS(s1->getAmmo(), 183);

    // - ship 2
    TS_ASSERT_EQUALS(s2->getDamage(), 102);
    TS_ASSERT_EQUALS(s2->getShield(), 0);
    TS_ASSERT_EQUALS(s2->getCrew(), 2287);
    TS_ASSERT_EQUALS(s2->getOwner(), 0);
    TS_ASSERT_EQUALS(s2->getAmmo(), 150);

    // - ship 3
    TS_ASSERT_EQUALS(s3->getDamage(), 0);
    TS_ASSERT_EQUALS(s3->getShield(), 100);
    TS_ASSERT_EQUALS(s3->getCrew(), 2287);
    TS_ASSERT_EQUALS(s3->getOwner(), 8);
    TS_ASSERT_EQUALS(s3->getAmmo(), 183);
}

/** Test ships, Cloaked Fighter Bays ability, ammo limit (bug #416).
    A: prepare three ships; one passive with Cloaked Fighter Bays ability, one aggressive with ammo limit.
    E: expected results and metadata produced (verified against PCC2 playvcr). In particular, correct fighter amounts lost. */
void
TestGameSimRun::testShipCloakedFighterBaysNT()
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrNuHost, game::sim::Configuration::BalanceNone);

    // Setup
    Ship* s1 = addOutrider(h.setup, 1, 12, h.list);
    Ship* s2 = addGorbie(h.setup, 2, 8, h.list);
    Ship* s3 = addGorbie(h.setup, 3, 8, h.list);
    h.result.init(h.opts, 0);
    s2->setFriendlyCode("NT1");
    s3->setAggressiveness(Ship::agg_Passive);
    s3->setFlags(Ship::fl_Cloaked | Ship::fl_CloakedBays | Ship::fl_CloakedBaysSet);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result
    // - rng has not been touched because we use seed control
    TS_ASSERT_EQUALS(h.rng.getSeed(), 42U);

    // - a battle has been created
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 1U);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(0, false)->getId(), 2);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(0, false)->getNumBays(), 20);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(0, false)->getNumFighters(), 10); /* limit applied */
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(1, false)->getId(), 1);
    TS_ASSERT_EQUALS(h.result.this_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.total_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.series_length, 118);
    TS_ASSERT_EQUALS(h.result.this_battle_index, 0);

    // - statistics
    TS_ASSERT_EQUALS(h.stats.size(), 3U);
    TS_ASSERT_EQUALS(h.stats[0].getNumFights(), 1);
    TS_ASSERT_EQUALS(h.stats[1].getNumFights(), 1);
    TS_ASSERT_EQUALS(h.stats[2].getNumFights(), 0);

    // - ship 1
    TS_ASSERT_EQUALS(s1->getDamage(), 187);
    TS_ASSERT_EQUALS(s1->getShield(), 0);
    TS_ASSERT_EQUALS(s1->getCrew(), 64);
    TS_ASSERT_EQUALS(s1->getOwner(), 0);
    TS_ASSERT_EQUALS(s1->getAmmo(), 0);

    // - ship 2
    TS_ASSERT_EQUALS(s2->getDamage(), 0);
    TS_ASSERT_EQUALS(s2->getShield(), 100);
    TS_ASSERT_EQUALS(s2->getCrew(), 2287);
    TS_ASSERT_EQUALS(s2->getOwner(), 8);
    TS_ASSERT_EQUALS(s2->getAmmo(), 248);

    // - ship 3
    TS_ASSERT_EQUALS(s3->getDamage(), 0);
    TS_ASSERT_EQUALS(s3->getShield(), 100);
    TS_ASSERT_EQUALS(s3->getCrew(), 2287);
    TS_ASSERT_EQUALS(s3->getOwner(), 8);
    TS_ASSERT_EQUALS(s3->getAmmo(), 249);
}

/** Test ships, Squadron ability.
    A: prepare two ships; a small Squadron one with three beams, and a big one.
    E: expected results and metadata produced (verified against PCC2 playvcr). */
void
TestGameSimRun::testShipSquadron()
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrNuHost, game::sim::Configuration::BalanceNone);

    // Setup
    Ship* s1 = addGorbie(h.setup, 1, 8, h.list);
    Ship* s2 = addGorbie(h.setup, 2, 4, h.list);
    h.result.init(h.opts, 0);
    s1->setHullType(0, h.list);
    s1->setMass(200);
    s1->setNumBeams(3);
    s1->setNumBays(0);
    s1->setFlags(Ship::fl_Squadron | Ship::fl_SquadronSet);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result
    // - rng has not been touched because we use seed control
    TS_ASSERT_EQUALS(h.rng.getSeed(), 42U);

    // - a battle has been created
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 1U);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(0, false)->getId(), 2);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(0, false)->getNumBeams(), 10);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(1, false)->getId(), 1);
    TS_ASSERT_EQUALS(h.result.battles->getBattle(0)->getObject(1, false)->getNumBeams(), 3);
    TS_ASSERT_EQUALS(h.result.this_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.total_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.series_length, 118);
    TS_ASSERT_EQUALS(h.result.this_battle_index, 0);

    // - statistics
    TS_ASSERT_EQUALS(h.stats.size(), 2U);

    // - ship 1
    TS_ASSERT_EQUALS(s1->getDamage(), 0);
    TS_ASSERT_EQUALS(s1->getShield(), 100);
    TS_ASSERT_EQUALS(s1->getCrew(), 2287);
    TS_ASSERT_EQUALS(s1->getOwner(), 8);
    TS_ASSERT_EQUALS(s1->getNumBeams(), 2);       // <- changed

    // - ship 2
    TS_ASSERT_EQUALS(s2->getDamage(), 0);
    TS_ASSERT_EQUALS(s2->getShield(), 100);
    TS_ASSERT_EQUALS(s2->getCrew(), 2287);
    TS_ASSERT_EQUALS(s2->getOwner(), 4);
    TS_ASSERT_EQUALS(s2->getAmmo(), 244);
}

/** Test deactivated planet.
    A: prepare ship and planet, planet deactivated.
    E: no fight happens. */
void
TestGameSimRun::testPlanetDeactivated()
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    addOutrider(h.setup, 1, 5, h.list);
    addPlanet(h.setup, 1, 4)->setFlags(Planet::fl_Deactivated);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result: no fight
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 0U);
}

/** Test cloaked ship at planet.
    A: prepare ship and planet, ship cloaked.
    E: no fight happens. */
void
TestGameSimRun::testPlanetCloaked()
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);
    h.config[game::config::HostConfiguration::AllowCloakedShipsAttack].set(0);

    // Setup
    addOutrider(h.setup, 1, 5, h.list)->setFlags(Ship::fl_Cloaked);
    addPlanet(h.setup, 1, 4)->setFriendlyCode("ATT");
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result: no fight
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 0U);
}

/** Test ship and planet with matching friendly codes.
    A: prepare ship and planet with matching friendly codes.
    E: no fight happens. */
void
TestGameSimRun::testPlanetFriendlyCodeMatch()
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    addOutrider(h.setup, 1, 5, h.list)->setFriendlyCode("xyz");
    addPlanet(h.setup, 1, 4)->setFriendlyCode("xyz");
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result: no fight
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 0U);
}

/** Test ship and planet, allied.
    A: prepare ship and planet, set up alliance.
    E: no fight happens. */
void
TestGameSimRun::testPlanetAllied()
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);
    h.opts.allianceSettings().set(4, 5, true);
    h.opts.allianceSettings().set(5, 4, true);

    // Setup
    addOutrider(h.setup, 1, 5, h.list);
    addPlanet(h.setup, 1, 4);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result: no fight
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 0U);
}

/** Test ship and planet, not aggressive.
    A: prepare ship and planet, none is aggressive.
    E: no fight happens. */
void
TestGameSimRun::testPlanetNotAggressive()
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    addOutrider(h.setup, 1, 5, h.list)->setAggressiveness(Ship::agg_Passive);
    addPlanet(h.setup, 1, 4)->setFriendlyCode("123");
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result: no fight
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 0U);
}

/** Test ship and planet, mismatching primary enemy.
    A: prepare ship and planet, planet not aggressive, ship with mismatching enemy.
    E: no fight happens. */
void
TestGameSimRun::testPlanetNotEnemy()
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    addOutrider(h.setup, 1, 5, h.list)->setAggressiveness(7);
    addPlanet(h.setup, 1, 4)->setFriendlyCode("123");
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result: no fight
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 0U);
}

/** Test ship and planet, ship is immune (by being Klingon).
    A: prepare ship and planet, ship is of an immune race, planet is aggressive.
    E: no fight happens. */
void
TestGameSimRun::testPlanetImmuneRace()
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    addOutrider(h.setup, 1, 4, h.list)->setAggressiveness(7);
    addPlanet(h.setup, 1, 2)->setFriendlyCode("ATT");
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result: no fight
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 0U);
}

/** Test ship and planet, ship is immune (by being Bird without fuel).
    A: prepare ship and planet, ship is Bird and fuelless, planet is aggressive.
    E: no fight happens. */
void
TestGameSimRun::testPlanetBird()
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    addOutrider(h.setup, 1, 3, h.list)->setAggressiveness(Ship::agg_NoFuel);
    addPlanet(h.setup, 1, 2)->setFriendlyCode("NUK");
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result: no fight
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 0U);
}

/** Test ship and planet, primary enemy.
    A: prepare ship and planet, ship has PE.
    E: fight happens. */
void
TestGameSimRun::testPlanetPrimaryEnemy()
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    addOutrider(h.setup, 1, 9, h.list)->setAggressiveness(2);
    addPlanet(h.setup, 1, 2)->setFriendlyCode("qqq");
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result: no fight
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 1U);
}

/** Test ship and planet, planet has NUK.
    A: prepare ship and planet, ship has no fuel, planet has NUK.
    E: fight happens. */
void
TestGameSimRun::testPlanetNuk()
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrHost, game::sim::Configuration::BalanceNone);

    // Setup
    addOutrider(h.setup, 1, 9, h.list)->setAggressiveness(Ship::agg_NoFuel);
    addPlanet(h.setup, 1, 2)->setFriendlyCode("NUK");
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result: no fight
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 1U);
}

/** Test basic FLAK simulation.
    A: prepare two ships, FLAK simulation.
    E: expected results and metadata produced. This is a regression test to ensure constant behaviour. */
void
TestGameSimRun::testFLAK()
{
    // Environment
    TestHarness h;
    setDeterministicConfig(h.opts, h.config, game::sim::Configuration::VcrFLAK, game::sim::Configuration::BalanceNone);

    // Setup
    Ship* s1 = addOutrider(h.setup, 1, 12, h.list);
    Ship* s2 = addOutrider(h.setup, 2, 11, h.list);
    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result
    // FIXME? Other alogs verify that rng has not been touched because we use seed control, but FLAK does touch it.

    // - a battle has been created
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 1U);
    TS_ASSERT_EQUALS(h.result.this_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.total_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.series_length, 110);
    TS_ASSERT_EQUALS(h.result.this_battle_index, 0);

    // - statistics
    TS_ASSERT_EQUALS(h.stats.size(), 2U);

    // - ship 1
    TS_ASSERT_EQUALS(s1->getDamage(), 71);
    TS_ASSERT_EQUALS(s1->getShield(), 0);
    TS_ASSERT_EQUALS(s1->getCrew(), 131);
    TS_ASSERT_EQUALS(s1->getOwner(), 12);

    // - ship 2
    TS_ASSERT_EQUALS(s2->getDamage(), 103);
    TS_ASSERT_EQUALS(s2->getShield(), 0);
    TS_ASSERT_EQUALS(s2->getCrew(), 109);
    TS_ASSERT_EQUALS(s2->getOwner(), 0);
}

/** Test multi-ship FLAK simulation.
    A: prepare multiple ships, FLAK simulation.
    E: expected results and metadata produced. This is a regression test to ensure constant behaviour. */
void
TestGameSimRun::testFLAKMulti()
{
    // Environment
    TestHarness h;
    game::TeamSettings team;
    h.opts.setMode(game::sim::Configuration::VcrFLAK, team, h.config);

    // Setup
    Ship* s1 = addOutrider(h.setup, 1, 1, h.list);
    Ship* s2 = addOutrider(h.setup, 2, 2, h.list);
    Ship* s3 = addOutrider(h.setup, 3, 2, h.list);
    Ship* s4 = addOutrider(h.setup, 4, 2, h.list);
    Planet* p = addPlanet(h.setup, 17, 1);
    s1->setFriendlyCode("-20");
    s2->setFriendlyCode("100");
    s3->setFriendlyCode("300");
    s4->setFriendlyCode("200");
    p->setFriendlyCode("ATT");
    p->setNumBaseFighters(60);

    h.result.init(h.opts, 0);

    // Do it
    game::sim::runSimulation(h.setup, h.stats, h.result, h.opts, h.list, h.config, h.rng);

    // Verify result
    // - battles have been created; series length unchanged
    TS_ASSERT(h.result.battles.get() != 0);
    TS_ASSERT_EQUALS(h.result.battles->getNumBattles(), 1U);
    TS_ASSERT_EQUALS(h.result.this_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.total_battle_weight, 1);
    TS_ASSERT_EQUALS(h.result.series_length, 110);
    TS_ASSERT_EQUALS(h.result.this_battle_index, 0);

    // - statistics
    TS_ASSERT_EQUALS(h.stats.size(), 5U);
    TS_ASSERT_EQUALS(h.stats.at(4).getMinFightersAboard(), 39);

    // - ship 1
    TS_ASSERT_EQUALS(s1->getDamage(), 0);
    TS_ASSERT_EQUALS(s1->getShield(), 52);
    TS_ASSERT_EQUALS(s1->getCrew(), 180);
    TS_ASSERT_EQUALS(s1->getOwner(), 1);

    // - ship 2
    TS_ASSERT_EQUALS(s2->getDamage(), 159);
    TS_ASSERT_EQUALS(s2->getShield(), 0);
    TS_ASSERT_EQUALS(s2->getCrew(), 101);
    TS_ASSERT_EQUALS(s2->getOwner(), 0);

    // - ship 3
    TS_ASSERT_EQUALS(s3->getDamage(), 151);
    TS_ASSERT_EQUALS(s3->getShield(), 0);
    TS_ASSERT_EQUALS(s3->getCrew(), 105);
    TS_ASSERT_EQUALS(s3->getOwner(), 0);

    // - ship 4
    TS_ASSERT_EQUALS(s4->getDamage(), 155);
    TS_ASSERT_EQUALS(s4->getShield(), 0);
    TS_ASSERT_EQUALS(s4->getCrew(), 97);
    TS_ASSERT_EQUALS(s4->getOwner(), 0);

    // - planet
    TS_ASSERT_EQUALS(p->getDamage(), 0);
    TS_ASSERT_EQUALS(p->getShield(), 100);
    TS_ASSERT_EQUALS(p->getOwner(), 1);
}
