/**
  *  \file test/game/sim/runnertest.cpp
  *  \brief Test for game::sim::Runner
  *
  *  Runner is abstract.
  *  Instead of mocking its run() (which would look mostly like SimpleRunner::run),
  *  test the actual implementations (SimpleRunner, ParallelRunner) against each other.
  *  Both must produce the same results and external behaviour.
  */

#include "game/sim/runner.hpp"

#include "afl/base/signalconnection.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
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

    Ship* addOutrider(afl::test::Assert a, game::sim::Setup& setup, int id, int owner, const game::spec::ShipList& list)
    {
        Ship* ship = addShip(setup, game::test::OUTRIDER_HULL_ID, id, owner, list);
        a.checkEqual("addOutrider > getCrew", ship->getCrew(), 180);  // verify that setHullType worked as planned
        return ship;
    }

    Ship* addGorbie(afl::test::Assert a, game::sim::Setup& setup, int id, int owner, const game::spec::ShipList& list)
    {
        Ship* ship = addShip(setup, game::test::GORBIE_HULL_ID, id, owner, list);
        a.checkEqual("addGorbie > getCrew", ship->getCrew(), 2287);
        return ship;
    }

    /* Verification for Gorbie vs Outriders test */
    void checkRegression1(afl::test::Assert a, const game::sim::Runner& runner)
    {
        a.checkEqual("01. getNumBattles",      runner.resultList().getNumBattles(), 110U);
        a.checkEqual("02. getNumClassResults", runner.resultList().getNumClassResults(), 1U);
        a.checkEqual("03. getNumUnitResults",  runner.resultList().getNumUnitResults(), 4U);

        // Class result
        const game::sim::ClassResult* c = runner.resultList().getClassResult(0);
        a.checkEqual("11. getClass",  c->getClass().get(1), 0);
        a.checkEqual("12. getClass",  c->getClass().get(8), 1);
        a.checkEqual("13. getWeight", c->getWeight(), 110);

        // Unit result: Gorbie
        const game::sim::UnitResult* ug = runner.resultList().getUnitResult(0);
        a.checkEqual("21. getNumFightsWon",    ug->getNumFightsWon(), 110);
        a.checkEqual("22. getNumFights",       ug->getNumFights(), 110);
        a.checkEqual("23. getNumCaptures",     ug->getNumCaptures(), 0);
        a.checkEqual("24. getNumFightersLost", ug->getNumFightersLost().min, 6);
        a.checkEqual("25. getNumFightersLost", ug->getNumFightersLost().max, 6);
        a.checkEqual("26. getNumFightersLost", ug->getNumFightersLost().totalScaled, 660);
        a.checkEqual("27. getShield",          ug->getShield().min, 100);
        a.checkEqual("28. getShield",          ug->getShield().max, 100);
        a.checkEqual("29. getShield",          ug->getShield().totalScaled, 11000);

        // Unit result: unfortunate outrider
        const game::sim::UnitResult* uo = runner.resultList().getUnitResult(1);
        a.checkEqual("31. getNumFightsWon", uo->getNumFightsWon(), 0);
        a.checkEqual("32. getNumFights",    uo->getNumFights(), 110);
        a.checkEqual("33. getNumCaptures",  uo->getNumCaptures(), 0);
        a.checkEqual("34. getShield",       uo->getShield().min, 0);
        a.checkEqual("35. getShield",       uo->getShield().max, 0);
        a.checkEqual("36. getShield",       uo->getShield().totalScaled, 0);
    }

    /* Verification for Outriders vs Outriders test */
    void checkRegression2(afl::test::Assert a, const game::sim::Runner& runner)
    {
        a.checkEqual("01. getNumBattles",      runner.resultList().getNumBattles(), 1000U);
        a.checkEqual("02. getNumClassResults", runner.resultList().getNumClassResults(), 2U);
        a.checkEqual("03. getNumUnitResults",  runner.resultList().getNumUnitResults(), 6U);

        // Class results
        const game::sim::ClassResult* c1 = runner.resultList().getClassResult(0);
        a.checkEqual("11. get",       c1->getClass().get(4), 0);
        a.checkEqual("12. get",       c1->getClass().get(6), 1);
        a.checkEqual("13. getWeight", c1->getWeight(), 914);

        const game::sim::ClassResult* c2 = runner.resultList().getClassResult(1);
        a.checkEqual("21. get",       c2->getClass().get(4), 1);
        a.checkEqual("22. get",       c2->getClass().get(6), 0);
        a.checkEqual("23. getWeight", c2->getWeight(), 86);

        // Unit result: first outrider
        const game::sim::UnitResult* u1 = runner.resultList().getUnitResult(0);
        a.checkEqual("31. getNumFightsWon", u1->getNumFightsWon(), 0);
        a.checkEqual("32. getNumFights",    u1->getNumFights(), 1000);
        a.checkEqual("33. getNumCaptures",  u1->getNumCaptures(), 0);
        a.checkEqual("34. getShield",       u1->getShield().min, 0);
        a.checkEqual("35. getShield",       u1->getShield().max, 0);
        a.checkEqual("36. getShield",       u1->getShield().totalScaled, 0);
        a.checkEqual("37. getDamage",       u1->getDamage().min, 106);
        a.checkEqual("38. getDamage",       u1->getDamage().max, 133);
        a.checkEqual("39. getDamage",       u1->getDamage().totalScaled, 108990);

        // Unit result: third outrider
        const game::sim::UnitResult* u3 = runner.resultList().getUnitResult(2);
        a.checkEqual("41. getNumFightsWon", u3->getNumFightsWon(), 86);
        a.checkEqual("42. getNumFights",    u3->getNumFights(), 1000);
        a.checkEqual("43. getNumCaptures",  u3->getNumCaptures(), 0);
        a.checkEqual("44. getShield",       u3->getShield().min, 0);
        a.checkEqual("45. getShield",       u3->getShield().max, 2);
        a.checkEqual("46. getShield",       u3->getShield().totalScaled, 2);
        a.checkEqual("47. getDamage",       u3->getDamage().min, 0);
        a.checkEqual("48. getDamage",       u3->getDamage().max, 108);
        a.checkEqual("49. getDamage",       u3->getDamage().totalScaled, 100076);

        // Unit result: sixth outrider
        const game::sim::UnitResult* u6 = runner.resultList().getUnitResult(5);
        a.checkEqual("51. getNumFightsWon", u6->getNumFightsWon(), 914);
        a.checkEqual("52. getNumFights",    u6->getNumFights(), 1000);
        a.checkEqual("53. getNumCaptures",  u6->getNumCaptures(), 0);
        a.checkEqual("54. getShield",       u6->getShield().min, 0);
        a.checkEqual("55. getShield",       u6->getShield().max, 4);
        a.checkEqual("56. getShield",       u6->getShield().totalScaled, 287);
        a.checkEqual("57. getDamage",       u6->getDamage().min, 0);
        a.checkEqual("58. getDamage",       u6->getDamage().max, 107);
        a.checkEqual("59. getDamage",       u6->getDamage().totalScaled, 42523);
    }

    void checkInterrupt(afl::test::Assert a, game::sim::Runner& runner)
    {
        util::StopSignal sig;
        afl::base::SignalConnection conn(runner.sig_update.add(&sig, &util::StopSignal::set));
        runner.setUpdateInterval(20);
        runner.run(runner.makeNoLimit(), sig);

        a.check("checkInterrupt > getNumBattles", runner.resultList().getNumBattles() != 0);
    }
}

/** Regression test 1: Gorbie vs 3 Outriders.
    This is a boring fight, Gorbie destroys everyone without getting a scratch. */
AFL_TEST("game.sim.Runner:regression1", a)
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
    addGorbie(a, setup, 100, 8, shipList);
    addOutrider(a, setup, 50, 1, shipList);
    addOutrider(a, setup, 51, 1, shipList);
    addOutrider(a, setup, 52, 1, shipList);

    // Host configuration
    afl::base::Ref<game::config::HostConfiguration> config = game::config::HostConfiguration::create();
    game::vcr::flak::Configuration flakConfiguration;

    // Configuration
    game::sim::Configuration opts;
    opts.setMode(game::sim::Configuration::VcrHost, 0, *config);

    // Stop signal (not used)
    util::StopSignal sig;

    // Logger (not used)
    afl::sys::Log log;

    // SimpleRunner
    util::RandomNumberGenerator simpleRNG(42);
    game::sim::SimpleRunner simpleRunner(setup, opts, shipList, *config, flakConfiguration, log, simpleRNG);
    simpleRunner.init();
    a.checkEqual("01. getNumBattles", simpleRunner.resultList().getNumBattles(), 1U);

    simpleRunner.run(simpleRunner.makeSeriesLimit(), sig);
    checkRegression1(a("SimpleRunner"), simpleRunner);

    // ParallelRunner
    util::RandomNumberGenerator parallelRNG(42);
    game::sim::ParallelRunner parallelRunner(setup, opts, shipList, *config, flakConfiguration, log, parallelRNG, 1);
    parallelRunner.init();
    a.checkEqual("11. getNumBattles", parallelRunner.resultList().getNumBattles(), 1U);

    parallelRunner.run(parallelRunner.makeSeriesLimit(), sig);
    checkRegression1(a("ParallelRunner"), parallelRunner);

    a.checkEqual("21. getSeed", parallelRNG.getSeed(), simpleRNG.getSeed());
}

/** Regression test 2: 3 vs 3 outriders. */
AFL_TEST("game.sim.Runner:regression2", a)
{
    // Ship list
    game::spec::ShipList shipList;
    game::test::initStandardBeams(shipList);
    game::test::initStandardTorpedoes(shipList);
    game::test::addOutrider(shipList);
    game::test::addTranswarp(shipList);

    // Setup
    game::sim::Setup setup;
    addOutrider(a, setup, 50, 4, shipList);
    addOutrider(a, setup, 51, 4, shipList);
    addOutrider(a, setup, 52, 4, shipList);

    addOutrider(a, setup, 70, 6, shipList);
    addOutrider(a, setup, 71, 6, shipList);
    addOutrider(a, setup, 72, 6, shipList);

    // Host configuration
    afl::base::Ref<game::config::HostConfiguration> config = game::config::HostConfiguration::create();
    game::vcr::flak::Configuration flakConfiguration;

    // Configuration
    game::sim::Configuration opts;
    opts.setMode(game::sim::Configuration::VcrHost, 0, *config);

    // Stop signal (not used)
    util::StopSignal sig;

    // Logger (not used)
    afl::sys::Log log;

    // SimpleRunner
    util::RandomNumberGenerator simpleRNG(77);
    game::sim::SimpleRunner simpleRunner(setup, opts, shipList, *config, flakConfiguration, log, simpleRNG);
    simpleRunner.init();
    a.checkEqual("01. getNumBattles", simpleRunner.resultList().getNumBattles(), 1U);

    simpleRunner.run(simpleRunner.makeFiniteLimit(999), sig);
    checkRegression2(a("SimpleRunner"), simpleRunner);

    // ParallelRunner
    util::RandomNumberGenerator parallelRNG(77);
    game::sim::ParallelRunner parallelRunner(setup, opts, shipList, *config, flakConfiguration, log, parallelRNG, 5);
    parallelRunner.init();
    a.checkEqual("11. getNumBattles", parallelRunner.resultList().getNumBattles(), 1U);

    parallelRunner.run(parallelRunner.makeFiniteLimit(999), sig);
    checkRegression2(a("ParallelRunner"), parallelRunner);

    a.checkEqual("21. getSeed", parallelRNG.getSeed(), simpleRNG.getSeed());
}

/** Test interruptability. This test will not terminate on error.
    A: create a Runner. Hook sig_update and give a stop signal from there.
    E: test completes */
AFL_TEST("game.sim.Runner:interrupt", a)
{
    // Ship list
    game::spec::ShipList shipList;
    game::test::initStandardBeams(shipList);
    game::test::initStandardTorpedoes(shipList);
    game::test::addOutrider(shipList);
    game::test::addTranswarp(shipList);

    // Setup
    game::sim::Setup setup;
    addOutrider(a, setup, 1, 4, shipList);
    addOutrider(a, setup, 2, 6, shipList);

    // Host configuration
    afl::base::Ref<game::config::HostConfiguration> config = game::config::HostConfiguration::create();
    game::vcr::flak::Configuration flakConfiguration;

    // Configuration
    game::sim::Configuration opts;
    opts.setMode(game::sim::Configuration::VcrHost, 0, *config);

    // Logger (not used)
    afl::sys::Log log;

    // SimpleRunner
    util::RandomNumberGenerator simpleRNG(77);
    game::sim::SimpleRunner simpleRunner(setup, opts, shipList, *config, flakConfiguration, log, simpleRNG);
    simpleRunner.init();
    checkInterrupt(a("SimpleRunner"), simpleRunner);

    // ParallelRunner
    util::RandomNumberGenerator parallelRNG(77);
    game::sim::ParallelRunner parallelRunner(setup, opts, shipList, *config, flakConfiguration, log, parallelRNG, 5);
    parallelRunner.init();
    checkInterrupt(a("ParallelRunner"), parallelRunner);
}
