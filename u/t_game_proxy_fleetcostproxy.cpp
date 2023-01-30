/**
  *  \file u/t_game_proxy_fleetcostproxy.cpp
  *  \brief Test for game::proxy::FleetCostProxy
  */

#include "game/proxy/fleetcostproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/game.hpp"
#include "game/proxy/simulationsetupproxy.hpp"
#include "game/teamsettings.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/shiplist.hpp"
#include "game/test/waitindicator.hpp"

using game::spec::Cost;
using game::test::SessionThread;
using game::test::WaitIndicator;
using game::proxy::SimulationSetupProxy;

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
void
TestGameProxyFleetCostProxy::testIt()
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender(), ind);

    // Add two ships
    t.addShip(ind, 0, 2);
    t.setHullType(0, game::test::OUTRIDER_HULL_ID, true);
    t.setHullType(1, game::test::OUTRIDER_HULL_ID, true);
    t.setOwner(0, 2);
    t.setOwner(1, 4);
    t.setEngineType(0, 5);
    t.setEngineType(1, 5);

    // Verify
    game::proxy::FleetCostProxy testee(t);

    // Set Inquiry
    TS_ASSERT_EQUALS(testee.getInvolvedPlayers(ind), game::PlayerSet_t() + 2 + 4);
    TS_ASSERT_EQUALS(testee.getInvolvedTeams(ind),   game::PlayerSet_t() +     4 + 5);

    // Cost Inquiry - players
    {
        game::spec::CostSummary result;
        testee.computeFleetCosts(ind, game::PlayerSet_t(2), false, result);

        TS_ASSERT_EQUALS(result.getNumItems(), 1U);
        // Outrider      40T 20D  5M 50$
        // Heavy Phaser   1T 12D 55M 54$
        TS_ASSERT_EQUALS(result.get(0)->cost.get(Cost::Tritanium), 41);
        TS_ASSERT_EQUALS(result.get(0)->cost.get(Cost::Money),    104);
    }

    // Cost Inquiry - players (empty set)
    {
        game::spec::CostSummary result;
        testee.computeFleetCosts(ind, game::PlayerSet_t(5), false, result);

        TS_ASSERT_EQUALS(result.getNumItems(), 0U);
    }

    // Cost Inquiry - teams
    {
        game::spec::CostSummary result;
        testee.computeFleetCosts(ind, game::PlayerSet_t(5), true, result);

        TS_ASSERT_EQUALS(result.getNumItems(), 1U);
        TS_ASSERT_EQUALS(result.get(0)->cost.get(Cost::Tritanium), 41);
        TS_ASSERT_EQUALS(result.get(0)->cost.get(Cost::Money),    104);
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

        TS_ASSERT_EQUALS(result.getNumItems(), 1U);
        // Outrider      40T 20D  5M 50$
        // Heavy Phaser   1T 12D 55M 54$
        // Nova Drive     3T  3D  7M 25$
        TS_ASSERT_EQUALS(result.get(0)->cost.get(Cost::Tritanium), 44);
        TS_ASSERT_EQUALS(result.get(0)->cost.get(Cost::Money),    129);
    }
}

/** Test behaviour on empty session.
    A: create empty session. Query information.
    E: empty results returned */
void
TestGameProxyFleetCostProxy::testEmpty()
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender(), ind);
    game::proxy::FleetCostProxy testee(t);

    // Empty sets
    TS_ASSERT_EQUALS(testee.getInvolvedPlayers(ind), game::PlayerSet_t());
    TS_ASSERT_EQUALS(testee.getInvolvedTeams(ind),   game::PlayerSet_t());

    // Empty list
    game::spec::CostSummary result;
    testee.computeFleetCosts(ind, game::PlayerSet_t::allUpTo(20), false, result);
    TS_ASSERT_EQUALS(result.getNumItems(), 0U);
}
