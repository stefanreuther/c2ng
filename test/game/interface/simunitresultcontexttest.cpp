/**
  *  \file test/game/interface/simunitresultcontexttest.cpp
  *  \brief Test for game::interface::SimUnitResultContext
  */

#include "game/interface/simunitresultcontext.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/sim/result.hpp"
#include "game/test/root.hpp"
#include "game/vcr/statistic.hpp"
#include "interpreter/test/contextverifier.hpp"

using afl::base::Ptr;
using afl::base::Ref;
using afl::string::NullTranslator;
using game::Root;
using game::interface::SimUnitResultContext;
using game::sim::ResultList;
using game::sim::Session;
using game::spec::ShipList;
using interpreter::test::ContextVerifier;

/* Verify basic property access. */
AFL_TEST("game.interface.SimUnitResultContext:basic", a)
{
    Ptr<SimUnitResultContext::Infos_t> infos = new SimUnitResultContext::Infos_t();

    ResultList::UnitInfo ui;
    ui.numFightsWon = 7;
    ui.numFights = 10;
    ui.numCaptures = 2;
    ui.cumulativeWeight = 2;
    ui.info.push_back(ResultList::UnitInfo::Item(ResultList::UnitInfo::Damage, 5, 100, 90, false, false));
    ui.info.push_back(ResultList::UnitInfo::Item(ResultList::UnitInfo::NumBaseFightersLost, 3, 5, 4.5, false, false));
    ui.info.push_back(ResultList::UnitInfo::Item(ResultList::UnitInfo::NumTorpedoHits, 12, 16, 14, false, false));
    infos->push_back(ui);

    Ref<Session> session = *new Session();
    session->setup().addShip().setName("Rocinante");

    Ref<Root> root = game::test::makeRoot(game::HostVersion());
    Ref<ShipList> shipList = *new ShipList();
    NullTranslator tx;

    SimUnitResultContext testee(infos, session, root, shipList, 0, tx);
    a.checkNull("getObject", testee.getObject());

    // Verify basics
    ContextVerifier verif(testee, a);
    verif.verifyBasics();
    verif.verifyNotSerializable();
    verif.verifyTypes();

    // Verify properties
    // - specimen for delegate properties
    verif.verifyString("NAME", "Rocinante");

    // - scalars
    verif.verifyInteger("FIGHTS", 10);
    verif.verifyInteger("FIGHTS.WON", 7);
    verif.verifyInteger("FIGHTS.CAPTURED", 2);

    // - defined properties
    verif.verifyFloat("RESULT.DAMAGE", 90, 0.001);
    verif.verifyInteger("RESULT.DAMAGE.MIN", 5);
    verif.verifyInteger("RESULT.DAMAGE.MAX", 100);

    verif.verifyFloat("RESULT.FIGHTERS.LOST", 4.5, 0.001);
    verif.verifyInteger("RESULT.FIGHTERS.LOST.MIN", 3);
    verif.verifyInteger("RESULT.FIGHTERS.LOST.MAX", 5);

    verif.verifyFloat("RESULT.TORPS.HIT", 14, 0.001);
    verif.verifyInteger("RESULT.TORPS.HIT.MIN", 12);
    verif.verifyInteger("RESULT.TORPS.HIT.MAX", 16);

    // - undefined properties
    verif.verifyNull("RESULT.TORPS.FIRED");
    verif.verifyNull("RESULT.TORPS.FIRED.MIN");
    verif.verifyNull("RESULT.TORPS.FIRED.MAX");
}

/* Verify iteration. */
AFL_TEST("game.interface.SimUnitResultContext:iteration", a)
{
    Ptr<SimUnitResultContext::Infos_t> infos = new SimUnitResultContext::Infos_t();

    {
        ResultList::UnitInfo ui;
        ui.numFightsWon = 99;
        infos->push_back(ui);
    }
    {
        ResultList::UnitInfo ui;
        ui.numFightsWon = 66;
        infos->push_back(ui);
    }

    Ref<Session> session = *new Session();
    session->setup().addShip().setName("One");
    session->setup().addShip().setName("Two");

    Ref<Root> root = game::test::makeRoot(game::HostVersion());
    Ref<ShipList> shipList = *new ShipList();
    NullTranslator tx;

    SimUnitResultContext testee(infos, session, root, shipList, 0, tx);

    // First entry
    {
        ContextVerifier verif(testee, a);
        verif.verifyString("NAME", "One");
        verif.verifyInteger("FIGHTS.WON", 99);
    }

    a.check("we can advance to next", testee.next());

    // First entry
    {
        ContextVerifier verif(testee, a);
        verif.verifyString("NAME", "Two");
        verif.verifyInteger("FIGHTS.WON", 66);
    }

    a.check("no more elements", !testee.next());
}

/* Out-of-range access.
   This normally does not happen. */
AFL_TEST("game.interface.SimUnitResultContext:out-of-range", a)
{
    // Empty environment
    Ptr<SimUnitResultContext::Infos_t> infos = new SimUnitResultContext::Infos_t();
    Ref<Session> session = *new Session();
    Ref<Root> root = game::test::makeRoot(game::HostVersion());
    Ref<ShipList> shipList = *new ShipList();
    NullTranslator tx;

    // Testee. Index 0 is out of range for empty environment.
    SimUnitResultContext testee(infos, session, root, shipList, 0, tx);

    // Verify
    ContextVerifier verif(testee, a);
    verif.verifyNull("NAME");
    verif.verifyNull("FIGHTS.WON");
    verif.verifyNull("RESULT.DAMAGE");
}

/* Verify creation from result. */
AFL_TEST("game.interface.SimUnitResultContext:create", a)
{
    // Session with a single ship
    Ref<Session> session = *new Session();
    game::sim::Ship& sh = session->setup().addShip();
    sh.setName("Rocinante");
    sh.setDamage(5);

    // Environment
    Ref<Root> root = game::test::makeRoot(game::HostVersion());
    Ref<ShipList> shipList = *new ShipList();
    NullTranslator tx;

    // Give the ship some damage in newState
    game::sim::ResultList result;
    game::sim::Setup newState = session->setup();
    newState.getShip(0)->setDamage(50);

    // Add result
    game::vcr::Statistic stat[1];
    game::sim::Result simResult(game::sim::Configuration(), 0);
    result.addResult(session->setup(), newState, stat, simResult);

    // Verify
    std::auto_ptr<SimUnitResultContext> testee(SimUnitResultContext::create(result, session, root, shipList, tx));
    a.checkNonNull("create result", testee.get());

    ContextVerifier verif(*testee, a);
    verif.verifyString("NAME", "Rocinante");
    verif.verifyInteger("RESULT.DAMAGE.MIN", 50);
}

/* Verify creation from empty.
   When given an empty setup, create() must return null. */
AFL_TEST("game.interface.SimUnitResultContext:create:empty", a)
{
    // Empty environment
    Ref<Session> session = *new Session();
    Ref<Root> root = game::test::makeRoot(game::HostVersion());
    Ref<ShipList> shipList = *new ShipList();
    NullTranslator tx;

    // Add an empty result
    game::sim::ResultList result;
    game::vcr::Statistic stat[1];
    game::sim::Result simResult(game::sim::Configuration(), 0);
    result.addResult(session->setup(), session->setup(), stat, simResult);

    // Verify
    std::auto_ptr<SimUnitResultContext> testee(SimUnitResultContext::create(result, session, root, shipList, tx));
    a.checkNull("create result", testee.get());
}
