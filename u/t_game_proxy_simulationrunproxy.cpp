/**
  *  \file u/t_game_proxy_simulationrunproxy.cpp
  *  \brief Test for game::proxy::SimulationRunProxy
  */

#include "game/proxy/simulationrunproxy.hpp"

#include "t_game_proxy.hpp"
#include "afl/sys/thread.hpp"
#include "game/proxy/simulationsetupproxy.hpp"
#include "game/proxy/vcrdatabaseproxy.hpp"
#include "game/test/counter.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/shiplist.hpp"
#include "game/test/waitindicator.hpp"
#include "afl/string/nulltranslator.hpp"

using game::test::SessionThread;
using game::test::WaitIndicator;
using game::test::Counter;
using game::proxy::SimulationRunProxy;
using game::proxy::SimulationSetupProxy;

namespace {
    void prepare(game::test::SessionThread& h)
    {
        // Ship list
        afl::base::Ptr<game::spec::ShipList> shipList = new game::spec::ShipList();
        game::test::initStandardTorpedoes(*shipList);
        game::test::initStandardBeams(*shipList);
        game::test::addTranswarp(*shipList);
        game::test::addOutrider(*shipList);
        h.session().setShipList(shipList);

        // Root
        afl::base::Ptr<game::Root> root = new game::test::Root(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0)));
        h.session().setRoot(root);
    }
}

/** Test behaviour on empty session.
    A: create empty session. Call runFinite().
    E: sig_stop emitted, count reported as 0. */
void
TestGameProxySimulationRunProxy::testEmpty()
{
    SessionThread h;
    WaitIndicator ind;
    SimulationSetupProxy setup(h.gameSender(), ind);
    SimulationRunProxy t(setup, ind);
    Counter c;
    t.sig_stop.add(&c, &Counter::increment);

    t.runFinite(20);
    h.sync();
    ind.processQueue();

    TS_ASSERT_EQUALS(c.get(), 1);
    TS_ASSERT_EQUALS(t.getNumBattles(), 0U);
}

/** Test normal behaviour.
    A: create session and set up a simulation. Call runFinite(1).
    E: sig_stop emitted, count reported as 1, results reported. */
void
TestGameProxySimulationRunProxy::testNormal()
{
    SessionThread h;
    WaitIndicator ind;
    prepare(h);

    // Create two hostile ships
    SimulationSetupProxy setup(h.gameSender(), ind);
    setup.addShip(ind, 0, 2);
    setup.setOwner(1, 3);

    // Run one simulation
    SimulationRunProxy t(setup, ind);
    Counter c;
    t.sig_stop.add(&c, &Counter::increment);
    t.runFinite(1);
    h.sync();
    ind.processQueue();

    // Verify
    TS_ASSERT_EQUALS(c.get(), 1);
    TS_ASSERT_EQUALS(t.getNumBattles(), 1U);

    // Verify results
    // - class results
    TS_ASSERT_EQUALS(t.getNumClassResults(), 1U);
    TS_ASSERT_EQUALS(t.getClassResults().size(), 1U);
    //   - first class
    TS_ASSERT(t.getClassInfo(0) != 0);
    TS_ASSERT_EQUALS(t.getClassInfo(0)->label, "1\xC3\x97 (100.0%)");
    TS_ASSERT_EQUALS(t.getClassInfo(0)->hasSample, true);
    //   - no second class
    TS_ASSERT(t.getClassInfo(1) == 0);

    // - unit results
    TS_ASSERT_EQUALS(t.getNumUnitResults(), 2U);
    TS_ASSERT_EQUALS(t.getUnitResults().size(), 2U);
    //   - first unit
    TS_ASSERT(t.getUnitInfo(0) != 0);
    TS_ASSERT_EQUALS(t.getUnitInfo(0)->numFights, 1);
    //   - second unit
    TS_ASSERT(t.getUnitInfo(1) != 0);
    TS_ASSERT_EQUALS(t.getUnitInfo(1)->numFights, 1);
    //   - no third unit
    TS_ASSERT(t.getUnitInfo(2) == 0);

    // Run 3 more
    t.runFinite(3);
    h.sync();
    ind.processQueue();

    // Verify
    TS_ASSERT_EQUALS(c.get(), 2);
    TS_ASSERT_EQUALS(t.getNumBattles(), 4U);
}

/** Test running a series.
    A: create session and set up a simulation. Call runSeries().
    E: sig_stop emitted, count reported as 110, results reported. */
void
TestGameProxySimulationRunProxy::testSeries()
{
    SessionThread h;
    WaitIndicator ind;
    prepare(h);

    // Create two hostile ships
    SimulationSetupProxy setup(h.gameSender(), ind);
    setup.addShip(ind, 0, 2);
    setup.setOwner(1, 3);

    // Run series
    SimulationRunProxy t(setup, ind);
    Counter c;
    t.sig_stop.add(&c, &Counter::increment);
    t.runSeries();
    h.sync();
    ind.processQueue();

    // Verify
    TS_ASSERT_EQUALS(c.get(), 1);
    TS_ASSERT_EQUALS(t.getNumBattles(), 110U);
}

/** Test infinite run.
    A: create session and set up a simulation. Call runInfinite(), wait briefly, then stop.
    E: sig_stop emitted, count reported as nonzero, results reported. */
void
TestGameProxySimulationRunProxy::testInfinite()
{
    SessionThread h;
    WaitIndicator ind;
    prepare(h);

    // Create two hostile ships
    SimulationSetupProxy setup(h.gameSender(), ind);
    setup.addShip(ind, 0, 2);
    setup.setOwner(1, 3);

    // Run infinitely
    SimulationRunProxy t(setup, ind);
    Counter c;
    t.sig_stop.add(&c, &Counter::increment);
    t.runInfinite();
    afl::sys::Thread::sleep(100);
    t.stop();
    h.sync();
    ind.processQueue();

    // Verify
    TS_ASSERT_EQUALS(c.get(), 1);
    TS_ASSERT(t.getNumBattles() > 0U);
}

/** Test run with no fights generated.
    A: create session and set up a simulation that generates no fights. Call runFinite().
    E: sig_stop emitted, count reported as zero. */
void
TestGameProxySimulationRunProxy::testNoFight()
{
    SessionThread h;
    WaitIndicator ind;
    prepare(h);

    // Create two friendly ships (just don't set an owner)
    SimulationSetupProxy setup(h.gameSender(), ind);
    setup.addShip(ind, 0, 2);

    // Run one simulation
    SimulationRunProxy t(setup, ind);
    Counter c;
    t.sig_stop.add(&c, &Counter::increment);
    t.runFinite(1);
    h.sync();
    ind.processQueue();

    // Verify
    TS_ASSERT_EQUALS(c.get(), 1);
    TS_ASSERT_EQUALS(t.getNumBattles(), 0U);
}

/** Test makeClassResultBattleAdaptor().
    A: create a session and set up a fight.
    E: VcrDatabaseProxy for class result produces expected result. */
void
TestGameProxySimulationRunProxy::testClassResultBattleAdaptor()
{
    SessionThread h;
    WaitIndicator ind;
    prepare(h);

    // Add ship and planet
    SimulationSetupProxy setup(h.gameSender(), ind);
    setup.addShip(ind, 0, 1);
    setup.addPlanet(ind);
    setup.setName(0, "Oliver's Kahn");
    setup.setName(1, "Pizza Planet");
    setup.setOwner(0, 4);
    setup.setOwner(1, 7);

    // Run one simulation
    SimulationRunProxy t(setup, ind);
    Counter c;
    t.sig_stop.add(&c, &Counter::increment);
    t.runFinite(1);
    h.sync();
    ind.processQueue();

    // Verify
    TS_ASSERT_EQUALS(c.get(), 1);
    TS_ASSERT_EQUALS(t.getNumBattles(), 1U);
    TS_ASSERT_EQUALS(t.getNumClassResults(), 1U);

    // Create VcrDatabaseProxy and verify it
    afl::string::NullTranslator tx;
    game::proxy::VcrDatabaseProxy dbProxy(t.makeClassResultBattleAdaptor(0), ind, tx, std::auto_ptr<game::spec::info::PictureNamer>(0));
    game::proxy::VcrDatabaseProxy::Status st;
    dbProxy.getStatus(ind, st);
    TS_ASSERT_EQUALS(st.numBattles, 1U);
    TS_ASSERT_EQUALS(st.currentBattle, 0U);
}

/** Test makeUnitResultBattleAdaptor().
    A: create a session and set up a fight.
    E: VcrDatabaseProxy for class result produces expected result. */
void
TestGameProxySimulationRunProxy::testUnitResultBattleAdaptor()
{
    SessionThread h;
    WaitIndicator ind;
    prepare(h);

    // Add ship and planet
    SimulationSetupProxy setup(h.gameSender(), ind);
    setup.addShip(ind, 0, 1);
    setup.addPlanet(ind);
    setup.setName(0, "Oliver's Kahn");
    setup.setName(1, "Pizza Planet");
    setup.setOwner(0, 4);
    setup.setOwner(1, 7);

    // Run one simulation
    SimulationRunProxy t(setup, ind);
    Counter c;
    t.sig_stop.add(&c, &Counter::increment);
    t.runFinite(1);
    h.sync();
    ind.processQueue();

    // Verify
    TS_ASSERT_EQUALS(c.get(), 1);
    TS_ASSERT_EQUALS(t.getNumBattles(), 1U);
    TS_ASSERT_EQUALS(t.getNumClassResults(), 1U);

    // Create VcrDatabaseProxy and verify it
    afl::string::NullTranslator tx;
    game::proxy::VcrDatabaseProxy dbProxy(t.makeUnitResultBattleAdaptor(0, game::sim::ResultList::UnitInfo::Damage, true), ind, tx, std::auto_ptr<game::spec::info::PictureNamer>(0));
    game::proxy::VcrDatabaseProxy::Status st;
    dbProxy.getStatus(ind, st);
    TS_ASSERT_EQUALS(st.numBattles, 1U);
    TS_ASSERT_EQUALS(st.currentBattle, 0U);
}

