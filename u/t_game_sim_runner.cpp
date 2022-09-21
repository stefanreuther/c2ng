/**
  *  \file u/t_game_sim_runner.cpp
  *  \brief Test for game::sim::Runner
  *
  *  Runner is abstract.
  *  Instead of mocking its run() (which would look mostly like SimpleRunner::run),
  *  test the actual implementations (SimpleRunner, ParallelRunner) against each other.
  *  Both must produce the same results and external behaviour.
  */

#include "game/sim/runner.hpp"

#include "t_game_sim.hpp"
#include "afl/base/signalconnection.hpp"
#include "afl/sys/log.hpp"
#include "game/sim/parallelrunner.hpp"
#include "game/sim/setup.hpp"
#include "game/sim/ship.hpp"
#include "game/sim/simplerunner.hpp"
#include "game/test/shiplist.hpp"

using game::sim::Ship;

namespace {
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
        ship->setEngineType(game::test::TRANSWARP_ENGINE_ID);
        ship->setAggressiveness(Ship::agg_Kill);
        ship->setInterceptId(0);
        return ship;
    }

    Ship* addOutrider(game::sim::Setup& setup, int id, int owner, const game::spec::ShipList& list)
    {
        Ship* ship = addShip(setup, game::test::OUTRIDER_HULL_ID, id, owner, list);
        TS_ASSERT_EQUALS(ship->getCrew(), 180);  // verify that setHullType worked as planned
        return ship;
    }

    Ship* addGorbie(game::sim::Setup& setup, int id, int owner, const game::spec::ShipList& list)
    {
        Ship* ship = addShip(setup, game::test::GORBIE_HULL_ID, id, owner, list);
        TS_ASSERT_EQUALS(ship->getCrew(), 2287);
        return ship;
    }

    /* Verification for Gorbie vs Outriders test */
    void checkRegression1(const char* name, game::sim::Runner& runner)
    {
        TSM_ASSERT_EQUALS(name, runner.resultList().getNumBattles(), 110U);
        TSM_ASSERT_EQUALS(name, runner.resultList().getNumClassResults(), 1U);
        TSM_ASSERT_EQUALS(name, runner.resultList().getNumUnitResults(), 4U);

        // Class result
        const game::sim::ClassResult* c = runner.resultList().getClassResult(0);
        TSM_ASSERT_EQUALS(name, c->getClass().get(1), 0);
        TSM_ASSERT_EQUALS(name, c->getClass().get(8), 1);
        TSM_ASSERT_EQUALS(name, c->getWeight(), 110);

        // Unit result: Gorbie
        const game::sim::UnitResult* ug = runner.resultList().getUnitResult(0);
        TSM_ASSERT_EQUALS(name, ug->getNumFightsWon(), 110);
        TSM_ASSERT_EQUALS(name, ug->getNumFights(), 110);
        TSM_ASSERT_EQUALS(name, ug->getNumCaptures(), 0);
        TSM_ASSERT_EQUALS(name, ug->getNumFightersLost().min, 6);
        TSM_ASSERT_EQUALS(name, ug->getNumFightersLost().max, 6);
        TSM_ASSERT_EQUALS(name, ug->getNumFightersLost().totalScaled, 660);
        TSM_ASSERT_EQUALS(name, ug->getShield().min, 100);
        TSM_ASSERT_EQUALS(name, ug->getShield().max, 100);
        TSM_ASSERT_EQUALS(name, ug->getShield().totalScaled, 11000);

        // Unit result: unfortunate outrider
        const game::sim::UnitResult* uo = runner.resultList().getUnitResult(1);
        TSM_ASSERT_EQUALS(name, uo->getNumFightsWon(), 0);
        TSM_ASSERT_EQUALS(name, uo->getNumFights(), 110);
        TSM_ASSERT_EQUALS(name, uo->getNumCaptures(), 0);
        TSM_ASSERT_EQUALS(name, uo->getShield().min, 0);
        TSM_ASSERT_EQUALS(name, uo->getShield().max, 0);
        TSM_ASSERT_EQUALS(name, uo->getShield().totalScaled, 0);
    }

    /* Verification for Outriders vs Outriders test */
    void checkRegression2(const char* name, game::sim::Runner& runner)
    {
        TSM_ASSERT_EQUALS(name, runner.resultList().getNumBattles(), 1000U);
        TSM_ASSERT_EQUALS(name, runner.resultList().getNumClassResults(), 2U);
        TSM_ASSERT_EQUALS(name, runner.resultList().getNumUnitResults(), 6U);

        // Class results
        const game::sim::ClassResult* c1 = runner.resultList().getClassResult(0);
        TSM_ASSERT_EQUALS(name, c1->getClass().get(4), 0);
        TSM_ASSERT_EQUALS(name, c1->getClass().get(6), 1);
        TSM_ASSERT_EQUALS(name, c1->getWeight(), 914);

        const game::sim::ClassResult* c2 = runner.resultList().getClassResult(1);
        TSM_ASSERT_EQUALS(name, c2->getClass().get(4), 1);
        TSM_ASSERT_EQUALS(name, c2->getClass().get(6), 0);
        TSM_ASSERT_EQUALS(name, c2->getWeight(), 86);

        // Unit result: first outrider
        const game::sim::UnitResult* u1 = runner.resultList().getUnitResult(0);
        TSM_ASSERT_EQUALS(name, u1->getNumFightsWon(), 0);
        TSM_ASSERT_EQUALS(name, u1->getNumFights(), 1000);
        TSM_ASSERT_EQUALS(name, u1->getNumCaptures(), 0);
        TSM_ASSERT_EQUALS(name, u1->getShield().min, 0);
        TSM_ASSERT_EQUALS(name, u1->getShield().max, 0);
        TSM_ASSERT_EQUALS(name, u1->getShield().totalScaled, 0);
        TSM_ASSERT_EQUALS(name, u1->getDamage().min, 106);
        TSM_ASSERT_EQUALS(name, u1->getDamage().max, 133);
        TSM_ASSERT_EQUALS(name, u1->getDamage().totalScaled, 108990);

        // Unit result: third outrider
        const game::sim::UnitResult* u3 = runner.resultList().getUnitResult(2);
        TSM_ASSERT_EQUALS(name, u3->getNumFightsWon(), 86);
        TSM_ASSERT_EQUALS(name, u3->getNumFights(), 1000);
        TSM_ASSERT_EQUALS(name, u3->getNumCaptures(), 0);
        TSM_ASSERT_EQUALS(name, u3->getShield().min, 0);
        TSM_ASSERT_EQUALS(name, u3->getShield().max, 2);
        TSM_ASSERT_EQUALS(name, u3->getShield().totalScaled, 2);
        TSM_ASSERT_EQUALS(name, u3->getDamage().min, 0);
        TSM_ASSERT_EQUALS(name, u3->getDamage().max, 108);
        TSM_ASSERT_EQUALS(name, u3->getDamage().totalScaled, 100076);

        // Unit result: sixth outrider
        const game::sim::UnitResult* u6 = runner.resultList().getUnitResult(5);
        TSM_ASSERT_EQUALS(name, u6->getNumFightsWon(), 914);
        TSM_ASSERT_EQUALS(name, u6->getNumFights(), 1000);
        TSM_ASSERT_EQUALS(name, u6->getNumCaptures(), 0);
        TSM_ASSERT_EQUALS(name, u6->getShield().min, 0);
        TSM_ASSERT_EQUALS(name, u6->getShield().max, 4);
        TSM_ASSERT_EQUALS(name, u6->getShield().totalScaled, 287);
        TSM_ASSERT_EQUALS(name, u6->getDamage().min, 0);
        TSM_ASSERT_EQUALS(name, u6->getDamage().max, 107);
        TSM_ASSERT_EQUALS(name, u6->getDamage().totalScaled, 42523);
    }

    void checkInterrupt(const char* name, game::sim::Runner& runner)
    {
        util::StopSignal sig;
        afl::base::SignalConnection conn(runner.sig_update.add(&sig, &util::StopSignal::set));
        runner.setUpdateInterval(20);
        runner.run(runner.makeNoLimit(), sig);

        TSM_ASSERT(name, runner.resultList().getNumBattles() != 0);
    }
}

/** Regression test 1: Gorbie vs 3 Outriders.
    This is a boring fight, Gorbie destroys everyone without getting a scratch. */
void
TestGameSimRunner::testRegression1()
{
    // Ship list
    game::spec::ShipList shipList;
    game::test::initStandardBeams(shipList);
    game::test::initStandardTorpedoes(shipList);
    game::test::addOutrider(shipList);
    game::test::addGorbie(shipList);
    game::test::addTranswarp(shipList);

    // Setup
    game::sim::Setup setup;
    addGorbie(setup, 100, 8, shipList);
    addOutrider(setup, 50, 1, shipList);
    addOutrider(setup, 51, 1, shipList);
    addOutrider(setup, 52, 1, shipList);

    // Host configuration
    game::config::HostConfiguration config;
    game::vcr::flak::Configuration flakConfiguration;

    // Configuration
    game::sim::Configuration opts;
    game::TeamSettings team;
    opts.setMode(game::sim::Configuration::VcrHost, team, config);

    // Stop signal (not used)
    util::StopSignal sig;

    // Logger (not used)
    afl::sys::Log log;

    // SimpleRunner
    util::RandomNumberGenerator simpleRNG(42);
    game::sim::SimpleRunner simpleRunner(setup, opts, shipList, config, flakConfiguration, log, simpleRNG);
    simpleRunner.init();
    TS_ASSERT_EQUALS(simpleRunner.resultList().getNumBattles(), 1U);

    simpleRunner.run(simpleRunner.makeSeriesLimit(), sig);
    checkRegression1("SimpleRunner", simpleRunner);

    // ParallelRunner
    util::RandomNumberGenerator parallelRNG(42);
    game::sim::ParallelRunner parallelRunner(setup, opts, shipList, config, flakConfiguration, log, parallelRNG, 1);
    parallelRunner.init();
    TS_ASSERT_EQUALS(parallelRunner.resultList().getNumBattles(), 1U);

    parallelRunner.run(parallelRunner.makeSeriesLimit(), sig);
    checkRegression1("ParallelRunner", parallelRunner);

    TS_ASSERT_EQUALS(parallelRNG.getSeed(), simpleRNG.getSeed());
}

/** Regression test 2: 3 vs 3 outriders. */
void
TestGameSimRunner::testRegression2()
{
    // Ship list
    game::spec::ShipList shipList;
    game::test::initStandardBeams(shipList);
    game::test::initStandardTorpedoes(shipList);
    game::test::addOutrider(shipList);
    game::test::addTranswarp(shipList);

    // Setup
    game::sim::Setup setup;
    addOutrider(setup, 50, 4, shipList);
    addOutrider(setup, 51, 4, shipList);
    addOutrider(setup, 52, 4, shipList);

    addOutrider(setup, 70, 6, shipList);
    addOutrider(setup, 71, 6, shipList);
    addOutrider(setup, 72, 6, shipList);

    // Host configuration
    game::config::HostConfiguration config;
    game::vcr::flak::Configuration flakConfiguration;

    // Configuration
    game::sim::Configuration opts;
    game::TeamSettings team;
    opts.setMode(game::sim::Configuration::VcrHost, team, config);

    // Stop signal (not used)
    util::StopSignal sig;

    // Logger (not used)
    afl::sys::Log log;

    // SimpleRunner
    util::RandomNumberGenerator simpleRNG(77);
    game::sim::SimpleRunner simpleRunner(setup, opts, shipList, config, flakConfiguration, log, simpleRNG);
    simpleRunner.init();
    TS_ASSERT_EQUALS(simpleRunner.resultList().getNumBattles(), 1U);

    simpleRunner.run(simpleRunner.makeFiniteLimit(999), sig);
    checkRegression2("SimpleRunner", simpleRunner);

    // ParallelRunner
    util::RandomNumberGenerator parallelRNG(77);
    game::sim::ParallelRunner parallelRunner(setup, opts, shipList, config, flakConfiguration, log, parallelRNG, 5);
    parallelRunner.init();
    TS_ASSERT_EQUALS(parallelRunner.resultList().getNumBattles(), 1U);

    parallelRunner.run(parallelRunner.makeFiniteLimit(999), sig);
    checkRegression2("ParallelRunner", parallelRunner);

    TS_ASSERT_EQUALS(parallelRNG.getSeed(), simpleRNG.getSeed());
}

/** Test interruptability. This test will not terminate on error.
    A: create a Runner. Hook sig_update and give a stop signal from there.
    E: test completes */
void
TestGameSimRunner::testInterrupt()
{
    // Ship list
    game::spec::ShipList shipList;
    game::test::initStandardBeams(shipList);
    game::test::initStandardTorpedoes(shipList);
    game::test::addOutrider(shipList);
    game::test::addTranswarp(shipList);

    // Setup
    game::sim::Setup setup;
    addOutrider(setup, 1, 4, shipList);
    addOutrider(setup, 2, 6, shipList);

    // Host configuration
    game::config::HostConfiguration config;
    game::vcr::flak::Configuration flakConfiguration;

    // Configuration
    game::sim::Configuration opts;
    game::TeamSettings team;
    opts.setMode(game::sim::Configuration::VcrHost, team, config);

    // Logger (not used)
    afl::sys::Log log;

    // SimpleRunner
    util::RandomNumberGenerator simpleRNG(77);
    game::sim::SimpleRunner simpleRunner(setup, opts, shipList, config, flakConfiguration, log, simpleRNG);
    simpleRunner.init();
    checkInterrupt("SimpleRunner", simpleRunner);

    // ParallelRunner
    util::RandomNumberGenerator parallelRNG(77);
    game::sim::ParallelRunner parallelRunner(setup, opts, shipList, config, flakConfiguration, log, parallelRNG, 5);
    parallelRunner.init();
    checkInterrupt("SimpleRunner", parallelRunner);
}

