/**
  *  \file test/game/proxy/simulationrunproxytest.cpp
  *  \brief Test for game::proxy::SimulationRunProxy
  */

#include "game/proxy/simulationrunproxy.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/sys/thread.hpp"
#include "afl/test/testrunner.hpp"
#include "game/proxy/simulationadaptorfromsession.hpp"
#include "game/proxy/simulationsetupproxy.hpp"
#include "game/proxy/vcrdatabaseproxy.hpp"
#include "game/test/counter.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/shiplist.hpp"
#include "game/test/waitindicator.hpp"

using game::test::SessionThread;
using game::test::WaitIndicator;
using game::test::Counter;
using game::proxy::SimulationAdaptorFromSession;
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
        afl::base::Ptr<game::Root> root = game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0))).asPtr();
        h.session().setRoot(root);
    }
}

/** Test behaviour on empty session.
    A: create empty session. Call runFinite().
    E: sig_stop emitted, count reported as 0. */
AFL_TEST("game.proxy.SimulationRunProxy:empty", a)
{
    SessionThread h;
    WaitIndicator ind;
    SimulationSetupProxy setup(h.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);
    SimulationRunProxy t(setup.adaptorSender(), ind);
    Counter c;
    t.sig_stop.add(&c, &Counter::increment);

    t.runFinite(20);
    h.sync();
    ind.processQueue();

    a.checkEqual("01. get", c.get(), 1);
    a.checkEqual("02. getNumBattles", t.getNumBattles(), 0U);
}

/** Test normal behaviour.
    A: create session and set up a simulation. Call runFinite(1).
    E: sig_stop emitted, count reported as 1, results reported. */
AFL_TEST("game.proxy.SimulationRunProxy:runFinite", a)
{
    SessionThread h;
    WaitIndicator ind;
    prepare(h);

    // Create two hostile ships
    SimulationSetupProxy setup(h.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);
    setup.addShip(ind, 0, 2);
    setup.setOwner(1, 3);

    // Run one simulation
    SimulationRunProxy t(setup.adaptorSender(), ind);
    Counter c;
    t.sig_stop.add(&c, &Counter::increment);
    t.runFinite(1);
    h.sync();
    ind.processQueue();

    // Verify
    a.checkEqual("01. get", c.get(), 1);
    a.checkEqual("02. getNumBattles", t.getNumBattles(), 1U);

    // Verify results
    // - class results
    a.checkEqual("11. getNumClassResults", t.getNumClassResults(), 1U);
    a.checkEqual("12. getClassResults", t.getClassResults().size(), 1U);
    //   - first class
    a.checkNonNull("13. getClassInfo", t.getClassInfo(0));
    a.checkEqual("14. label", t.getClassInfo(0)->label, "1\xC3\x97 (100.0%)");
    a.checkEqual("15. hasSample", t.getClassInfo(0)->hasSample, true);
    //   - no second class
    a.checkNull("16. getClassInfo", t.getClassInfo(1));

    // - unit results
    a.checkEqual("21. getNumUnitResults", t.getNumUnitResults(), 2U);
    a.checkEqual("22. getUnitResults", t.getUnitResults().size(), 2U);
    //   - first unit
    a.checkNonNull("23. getUnitInfo", t.getUnitInfo(0));
    a.checkEqual("24. numFights", t.getUnitInfo(0)->numFights, 1);
    //   - second unit
    a.checkNonNull("25. getUnitInfo", t.getUnitInfo(1));
    a.checkEqual("26. numFights", t.getUnitInfo(1)->numFights, 1);
    //   - no third unit
    a.checkNull("27. getUnitInfo", t.getUnitInfo(2));

    // Run 3 more
    t.runFinite(3);
    h.sync();
    ind.processQueue();

    // Verify
    a.checkEqual("31. get", c.get(), 2);
    a.checkEqual("32. getNumBattles", t.getNumBattles(), 4U);
}

/** Test running a series.
    A: create session and set up a simulation. Call runSeries().
    E: sig_stop emitted, count reported as 110, results reported. */
AFL_TEST("game.proxy.SimulationRunProxy:runSeries", a)
{
    SessionThread h;
    WaitIndicator ind;
    prepare(h);

    // Create two hostile ships
    SimulationSetupProxy setup(h.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);
    setup.addShip(ind, 0, 2);
    setup.setOwner(1, 3);

    // Run series
    SimulationRunProxy t(setup.adaptorSender(), ind);
    Counter c;
    t.sig_stop.add(&c, &Counter::increment);
    t.runSeries();
    h.sync();
    ind.processQueue();

    // Verify
    a.checkEqual("01. get", c.get(), 1);
    a.checkEqual("02. getNumBattles", t.getNumBattles(), 110U);
}

/** Test infinite run.
    A: create session and set up a simulation. Call runInfinite(), wait briefly, then stop.
    E: sig_stop emitted, count reported as nonzero, results reported. */
AFL_TEST("game.proxy.SimulationRunProxy:runInfinite", a)
{
    SessionThread h;
    WaitIndicator ind;
    prepare(h);

    // Create two hostile ships
    SimulationSetupProxy setup(h.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);
    setup.addShip(ind, 0, 2);
    setup.setOwner(1, 3);

    // Run infinitely
    SimulationRunProxy t(setup.adaptorSender(), ind);
    Counter c;
    t.sig_stop.add(&c, &Counter::increment);
    t.runInfinite();
    afl::sys::Thread::sleep(100);
    t.stop();
    h.sync();
    ind.processQueue();

    // Verify
    a.checkEqual("01. get", c.get(), 1);
    a.check("02. getNumBattles", t.getNumBattles() > 0U);
}

/** Test run with no fights generated.
    A: create session and set up a simulation that generates no fights. Call runFinite().
    E: sig_stop emitted, count reported as zero. */
AFL_TEST("game.proxy.SimulationRunProxy:no-fight", a)
{
    SessionThread h;
    WaitIndicator ind;
    prepare(h);

    // Create two friendly ships (just don't set an owner)
    SimulationSetupProxy setup(h.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);
    setup.addShip(ind, 0, 2);

    // Run one simulation
    SimulationRunProxy t(setup.adaptorSender(), ind);
    Counter c;
    t.sig_stop.add(&c, &Counter::increment);
    t.runFinite(1);
    h.sync();
    ind.processQueue();

    // Verify
    a.checkEqual("01. get", c.get(), 1);
    a.checkEqual("02. getNumBattles", t.getNumBattles(), 0U);
}

/** Test makeClassResultBattleAdaptor().
    A: create a session and set up a fight.
    E: VcrDatabaseProxy for class result produces expected result. */
AFL_TEST("game.proxy.SimulationRunProxy:makeClassResultBattleAdaptor", a)
{
    SessionThread h;
    WaitIndicator ind;
    prepare(h);

    // Add ship and planet
    SimulationSetupProxy setup(h.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);
    setup.addShip(ind, 0, 1);
    setup.addPlanet(ind);
    setup.setName(0, "Oliver's Kahn");
    setup.setName(1, "Pizza Planet");
    setup.setOwner(0, 4);
    setup.setOwner(1, 7);

    // Run one simulation
    SimulationRunProxy t(setup.adaptorSender(), ind);
    Counter c;
    t.sig_stop.add(&c, &Counter::increment);
    t.runFinite(1);
    h.sync();
    ind.processQueue();

    // Verify
    a.checkEqual("01. get", c.get(), 1);
    a.checkEqual("02. getNumBattles", t.getNumBattles(), 1U);
    a.checkEqual("03. getNumClassResults", t.getNumClassResults(), 1U);

    // Create VcrDatabaseProxy and verify it
    afl::string::NullTranslator tx;
    game::proxy::VcrDatabaseProxy dbProxy(t.makeClassResultBattleAdaptor(0), ind, tx, std::auto_ptr<game::spec::info::PictureNamer>(0));
    game::proxy::VcrDatabaseProxy::Status st;
    dbProxy.getStatus(ind, st);
    a.checkEqual("11. numBattles", st.numBattles, 1U);
    a.checkEqual("12. currentBattle", st.currentBattle, 0U);
}

/** Test makeUnitResultBattleAdaptor().
    A: create a session and set up a fight.
    E: VcrDatabaseProxy for class result produces expected result. */
AFL_TEST("game.proxy.SimulationRunProxy:makeUnitResultBattleAdaptor", a)
{
    SessionThread h;
    WaitIndicator ind;
    prepare(h);

    // Add ship and planet
    SimulationSetupProxy setup(h.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);
    setup.addShip(ind, 0, 1);
    setup.addPlanet(ind);
    setup.setName(0, "Oliver's Kahn");
    setup.setName(1, "Pizza Planet");
    setup.setOwner(0, 4);
    setup.setOwner(1, 7);

    // Run one simulation
    SimulationRunProxy t(setup.adaptorSender(), ind);
    Counter c;
    t.sig_stop.add(&c, &Counter::increment);
    t.runFinite(1);
    h.sync();
    ind.processQueue();

    // Verify
    a.checkEqual("01. get", c.get(), 1);
    a.checkEqual("02. getNumBattles", t.getNumBattles(), 1U);
    a.checkEqual("03. getNumClassResults", t.getNumClassResults(), 1U);

    // Create VcrDatabaseProxy and verify it
    afl::string::NullTranslator tx;
    game::proxy::VcrDatabaseProxy dbProxy(t.makeUnitResultBattleAdaptor(0, game::sim::ResultList::UnitInfo::Damage, true), ind, tx, std::auto_ptr<game::spec::info::PictureNamer>(0));
    game::proxy::VcrDatabaseProxy::Status st;
    dbProxy.getStatus(ind, st);
    a.checkEqual("11. numBattles", st.numBattles, 1U);
    a.checkEqual("12. currentBattle", st.currentBattle, 0U);
}
