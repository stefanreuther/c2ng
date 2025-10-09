/**
  *  \file test/game/proxy/fleetcostproxytest.cpp
  *  \brief Test for game::proxy::FleetCostProxy
  */

#include "game/proxy/fleetcostproxy.hpp"

#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/proxy/simulationadaptorfromsession.hpp"
#include "game/proxy/simulationsetupproxy.hpp"
#include "game/teamsettings.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/shiplist.hpp"
#include "game/test/waitindicator.hpp"

using game::proxy::SimulationAdaptorFromSession;
using game::proxy::SimulationSetupProxy;
using game::spec::Cost;
using game::test::SessionThread;
using game::test::WaitIndicator;

namespace {
    void prepare(SessionThread& thread)
    {
        // Shiplist
        afl::base::Ptr<game::spec::ShipList> list = new game::spec::ShipList();
        game::test::initStandardBeams(*list);
        game::test::initStandardTorpedoes(*list);
        game::test::addOutrider(*list);
        game::test::addNovaDrive(*list);
        thread.session().setShipList(list);

        // Root
        afl::base::Ptr<game::Root> root = game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4, 0, 0))).asPtr();
        thread.session().setRoot(root);

        // Game
        afl::base::Ptr<game::Game> g = new game::Game();
        g->teamSettings().setPlayerTeam(2, 5);
        thread.session().setGame(g);
    }
}

/** Test normal behaviour.
    A: create session with all components. Add ships to simulation (using SimulationSetupProxy). Query information.
    E: expected results returned */
AFL_TEST("game.proxy.FleetCostProxy:normal", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add two ships
    t.addShip(ind, 0, 2);
    t.setHullType(0, game::test::OUTRIDER_HULL_ID, true);
    t.setHullType(1, game::test::OUTRIDER_HULL_ID, true);
    t.setOwner(0, 2);
    t.setOwner(1, 4);
    t.setEngineType(0, 5);
    t.setEngineType(1, 5);

    // Verify
    game::proxy::FleetCostProxy testee(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()));

    // Set Inquiry
    a.checkEqual("01. getInvolvedPlayers", testee.getInvolvedPlayers(ind), game::PlayerSet_t() + 2 + 4);
    a.checkEqual("02. getInvolvedTeams",   testee.getInvolvedTeams(ind),   game::PlayerSet_t() +     4 + 5);

    // Cost Inquiry - players
    {
        game::spec::CostSummary result;
        testee.computeFleetCosts(ind, game::PlayerSet_t(2), false, result);

        a.checkEqual("11. getNumItems", result.getNumItems(), 1U);
        // Outrider      40T 20D  5M 50$
        // Heavy Phaser   1T 12D 55M 54$
        a.checkEqual("12. Tritanium", result.get(0)->cost.get(Cost::Tritanium), 41);
        a.checkEqual("13. Money",     result.get(0)->cost.get(Cost::Money),    104);
    }

    // Cost Inquiry - players (empty set)
    {
        game::spec::CostSummary result;
        testee.computeFleetCosts(ind, game::PlayerSet_t(5), false, result);

        a.checkEqual("21. getNumItems", result.getNumItems(), 0U);
    }

    // Cost Inquiry - teams
    {
        game::spec::CostSummary result;
        testee.computeFleetCosts(ind, game::PlayerSet_t(5), true, result);

        a.checkEqual("31. getNumItems", result.getNumItems(), 1U);
        a.checkEqual("32. Tritanium", result.get(0)->cost.get(Cost::Tritanium), 41);
        a.checkEqual("33. Money",     result.get(0)->cost.get(Cost::Money),    104);
    }

    // Config change
    {
        game::sim::FleetCostOptions opts;
        testee.getOptions(ind, opts);
        opts.useEngines = true;
        testee.setOptions(opts);
    }

    // Cost Inquiry - with changed config
    {
        game::spec::CostSummary result;
        testee.computeFleetCosts(ind, game::PlayerSet_t(5), true, result);

        a.checkEqual("41. getNumItems", result.getNumItems(), 1U);
        // Outrider      40T 20D  5M 50$
        // Heavy Phaser   1T 12D 55M 54$
        // Nova Drive     3T  3D  7M 25$
        a.checkEqual("42. Tritanium", result.get(0)->cost.get(Cost::Tritanium), 44);
        a.checkEqual("43. Money",     result.get(0)->cost.get(Cost::Money),    129);
    }
}

/** Test behaviour on empty session.
    A: create empty session. Query information.
    E: empty results returned */
AFL_TEST("game.proxy.FleetCostProxy:empty", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    game::proxy::FleetCostProxy testee(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()));

    // Empty sets
    a.checkEqual("01. getInvolvedPlayers", testee.getInvolvedPlayers(ind), game::PlayerSet_t());
    a.checkEqual("02. getInvolvedTeams",   testee.getInvolvedTeams(ind),   game::PlayerSet_t());

    // Empty list
    game::spec::CostSummary result;
    testee.computeFleetCosts(ind, game::PlayerSet_t::allUpTo(20), false, result);
    a.checkEqual("11. getNumItems", result.getNumItems(), 0U);
}
