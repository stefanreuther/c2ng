/**
  *  \file u/t_game_sim_fleetcost.cpp
  *  \brief Test for game::sim::FleetCost
  */

#include "game/sim/fleetcost.hpp"

#include "t_game_sim.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/sim/setup.hpp"
#include "game/sim/configuration.hpp"
#include "game/test/shiplist.hpp"
#include "game/sim/ship.hpp"
#include "game/sim/planet.hpp"

using game::sim::FleetCostOptions;
using game::spec::CostSummary;
using game::spec::Cost;

namespace {
    void addAnnihilation(game::sim::Setup& setup, int id, int owner, const game::spec::ShipList& shipList)
    {
        afl::string::NullTranslator tx;
        game::sim::Ship* sh = setup.addShip();
        sh->setHullType(game::test::ANNIHILATION_HULL_ID, shipList);
        sh->setId(id);
        sh->setOwner(owner);
        sh->setDefaultName(tx);
        sh->setEngineType(5);
    }

    void addCustomShip(game::sim::Setup& setup, int id, int owner)
    {
        afl::string::NullTranslator tx;
        game::sim::Ship* sh = setup.addShip();
        sh->setId(id);
        sh->setOwner(owner);
        sh->setDefaultName(tx);
        sh->setNumBays(3);
        sh->setAmmo(5);
    }

    void addPlanet(game::sim::Setup& setup, int owner)
    {
        game::sim::Planet* pl = setup.addPlanet();
        pl->setOwner(owner);
        pl->setBaseBeamTech(3);
        pl->setDefense(17);
        pl->setBaseDefense(5);
    }
}

/** Test behaviour with all-empty content. */
void
TestGameSimFleetCost::testEmpty()
{
    CostSummary out;
    game::sim::Setup in;
    game::sim::Configuration simConfig;
    FleetCostOptions opts;
    game::spec::ShipList shipList;
    game::config::HostConfiguration config;
    game::PlayerList playerList;
    afl::string::NullTranslator tx;

    computeFleetCosts(out, in, simConfig, opts, shipList, config, playerList, game::PlayerSet_t::allUpTo(20), tx);

    TS_ASSERT_EQUALS(out.getNumItems(), 0U);
}

/** Test behaviour with a populated setup. */
void
TestGameSimFleetCost::testNormal()
{
    game::sim::Setup in;
    game::sim::Configuration simConfig;
    game::spec::ShipList shipList;
    game::config::HostConfiguration config;
    game::PlayerList playerList;
    afl::string::NullTranslator tx;

    game::test::initPListBeams(shipList);
    game::test::initPListTorpedoes(shipList);
    game::test::addAnnihilation(shipList);
    game::test::addNovaDrive(shipList);

    // Ship (played by 6)
    addAnnihilation(in, 1, 6, shipList);

    // Ship (played by 3)
    addCustomShip(in, 50, 3);

    // Planet (played by 6)
    addPlanet(in, 6);

    // Compute cost for 6
    {
        FleetCostOptions opts;
        CostSummary out;
        computeFleetCosts(out, in, simConfig, opts, shipList, config, playerList, game::PlayerSet_t(6), tx);

        TS_ASSERT_EQUALS(out.getNumItems(), 3U);

        // First: ship
        const CostSummary::Item* it1 = out.get(0);
        TS_ASSERT_EQUALS(it1->name, "Ship 1 (#1, Player 6 ANNIHILATION CLASS BATTLESHIP)");
        // Hull:           343T 340D 550M   910$
        // Beams(10):      250T 150D 100M  1300$
        // Launchers(10):  150T  50D 200M  1500$
        // Ammo(320):      320D 320D 320M 25600$
        TS_ASSERT_EQUALS(it1->cost.get(Cost::Tritanium),  1063);
        TS_ASSERT_EQUALS(it1->cost.get(Cost::Duranium),    860);
        TS_ASSERT_EQUALS(it1->cost.get(Cost::Molybdenum), 1170);
        TS_ASSERT_EQUALS(it1->cost.get(Cost::Money),     29310);

        // Second: planet
        const CostSummary::Item* it2 = out.get(1);
        TS_ASSERT_EQUALS(it2->name, "Planet");
        // Defense(17):  170$ 17S
        TS_ASSERT_EQUALS(it2->cost.get(Cost::Tritanium),  0);
        TS_ASSERT_EQUALS(it2->cost.get(Cost::Duranium),   0);
        TS_ASSERT_EQUALS(it2->cost.get(Cost::Molybdenum), 0);
        TS_ASSERT_EQUALS(it2->cost.get(Cost::Money),    170);
        TS_ASSERT_EQUALS(it2->cost.get(Cost::Supplies),  17);

        // Third: base
        const CostSummary::Item* it3 = out.get(2);
        TS_ASSERT_EQUALS(it3->name, "Starbase");
        // Base:        402T 120D 340M 900$
        // Defense(5):         5D       50$
        TS_ASSERT_EQUALS(it3->cost.get(Cost::Tritanium),  402);
        TS_ASSERT_EQUALS(it3->cost.get(Cost::Duranium),   125);
        TS_ASSERT_EQUALS(it3->cost.get(Cost::Molybdenum), 340);
        TS_ASSERT_EQUALS(it3->cost.get(Cost::Money),      950);
        TS_ASSERT_EQUALS(it3->cost.get(Cost::Supplies),     0);
    }

    // Compute cost for 6 using tech levels and engines
    {
        FleetCostOptions opts;
        opts.shipTechMode = FleetCostOptions::ShipTech;
        opts.useEngines = true;
        CostSummary out;
        computeFleetCosts(out, in, simConfig, opts, shipList, config, playerList, game::PlayerSet_t(6), tx);

        TS_ASSERT_EQUALS(out.getNumItems(), 3U);

        // First: ship
        const CostSummary::Item* it1 = out.get(0);
        TS_ASSERT_EQUALS(it1->name, "Ship 1 (#1, Player 6 ANNIHILATION CLASS BATTLESHIP)");
        // Hull:           343T 340D 550M   910$
        // Beams(10):      250T 150D 100M  1300$
        // Launchers(10):  150T  50D 200M  1500$
        // Ammo(320):      320D 320D 320M 25600$
        // Engines(6):      18D  18D  42M   150$
        // HullTech:                       4500$
        // BeamTech:                       4500$
        // TorpTech:                       4500$
        // EngineTech:                     1000$
        TS_ASSERT_EQUALS(it1->cost.get(Cost::Tritanium),  1081);
        TS_ASSERT_EQUALS(it1->cost.get(Cost::Duranium),    878);
        TS_ASSERT_EQUALS(it1->cost.get(Cost::Molybdenum), 1212);
        TS_ASSERT_EQUALS(it1->cost.get(Cost::Money),     43960);
    }

    // Compute cost for 3
    {
        FleetCostOptions opts;
        CostSummary out;
        computeFleetCosts(out, in, simConfig, opts, shipList, config, playerList, game::PlayerSet_t(3), tx);

        TS_ASSERT_EQUALS(out.getNumItems(), 1U);

        const CostSummary::Item* it = out.get(0);
        TS_ASSERT_EQUALS(it->name, "Ship 50 (#50, Player 3 custom ship)");
        // Fighters(5):  15T 10M
        TS_ASSERT_EQUALS(it->cost.get(Cost::Tritanium),  15);
        TS_ASSERT_EQUALS(it->cost.get(Cost::Duranium),    0);
        TS_ASSERT_EQUALS(it->cost.get(Cost::Molybdenum), 10);
        TS_ASSERT_EQUALS(it->cost.get(Cost::Money),       0);
    }

    // Compute cost for 3 using fighters built on base
    {
        FleetCostOptions opts;
        opts.fighterMode = FleetCostOptions::BaseFighters;

        CostSummary out;
        computeFleetCosts(out, in, simConfig, opts, shipList, config, playerList, game::PlayerSet_t(3), tx);

        TS_ASSERT_EQUALS(out.getNumItems(), 1U);

        const CostSummary::Item* it = out.get(0);
        TS_ASSERT_EQUALS(it->name, "Ship 50 (#50, Player 3 custom ship)");
        // Fighters(5):  15T 10M 500$
        TS_ASSERT_EQUALS(it->cost.get(Cost::Tritanium),  15);
        TS_ASSERT_EQUALS(it->cost.get(Cost::Duranium),    0);
        TS_ASSERT_EQUALS(it->cost.get(Cost::Molybdenum), 10);
        TS_ASSERT_EQUALS(it->cost.get(Cost::Money),     500);
    }

    // Compute cost for non-present race
    {
        FleetCostOptions opts;
        CostSummary out;
        computeFleetCosts(out, in, simConfig, opts, shipList, config, playerList, game::PlayerSet_t(7), tx);
        TS_ASSERT_EQUALS(out.getNumItems(), 0U);
    }
}

/** Test handling of tech costs.
    A: create setup with two ships. Compute tech costs with different values for shipTechMode.
    E: correct results reported */
void
TestGameSimFleetCost::testTechCost()
{
    game::sim::Setup in;
    game::sim::Configuration simConfig;
    game::spec::ShipList shipList;
    game::config::HostConfiguration config;
    game::PlayerList playerList;
    afl::string::NullTranslator tx;

    game::test::initPListBeams(shipList);
    game::test::initPListTorpedoes(shipList);
    game::test::addAnnihilation(shipList);
    game::test::addNovaDrive(shipList);

    // 2 ships (played by 6)
    addAnnihilation(in, 1, 6, shipList);
    addAnnihilation(in, 2, 6, shipList);

    // Compute cost for 6 using NoTech (default)
    {
        FleetCostOptions opts;
        opts.shipTechMode = FleetCostOptions::NoTech;
        CostSummary out;
        computeFleetCosts(out, in, simConfig, opts, shipList, config, playerList, game::PlayerSet_t(6), tx);

        TS_ASSERT_EQUALS(out.getNumItems(), 2U);

        // First ship
        const CostSummary::Item* it1 = out.get(0);
        // Hull:           343T 340D 550M   910$
        // Beams(10):      250T 150D 100M  1300$
        // Launchers(10):  150T  50D 200M  1500$
        // Ammo(320):      320D 320D 320M 25600$
        TS_ASSERT_EQUALS(it1->cost.get(Cost::Tritanium),  1063);
        TS_ASSERT_EQUALS(it1->cost.get(Cost::Duranium),    860);
        TS_ASSERT_EQUALS(it1->cost.get(Cost::Molybdenum), 1170);
        TS_ASSERT_EQUALS(it1->cost.get(Cost::Money),     29310);

        // Second ship -> same
        const CostSummary::Item* it2 = out.get(1);
        TS_ASSERT_EQUALS(it2->cost.get(Cost::Tritanium),  1063);
        TS_ASSERT_EQUALS(it2->cost.get(Cost::Duranium),    860);
        TS_ASSERT_EQUALS(it2->cost.get(Cost::Molybdenum), 1170);
        TS_ASSERT_EQUALS(it2->cost.get(Cost::Money),     29310);
    }

    // Compute cost for 6 using ShipTech
    {
        FleetCostOptions opts;
        opts.shipTechMode = FleetCostOptions::ShipTech;
        CostSummary out;
        computeFleetCosts(out, in, simConfig, opts, shipList, config, playerList, game::PlayerSet_t(6), tx);

        TS_ASSERT_EQUALS(out.getNumItems(), 2U);

        // First ship
        const CostSummary::Item* it1 = out.get(0);
        // Hull:           343T 340D 550M   910$
        // Beams(10):      250T 150D 100M  1300$
        // Launchers(10):  150T  50D 200M  1500$
        // Ammo(320):      320D 320D 320M 25600$
        // 3x Tech 10                     13500$
        TS_ASSERT_EQUALS(it1->cost.get(Cost::Tritanium),  1063);
        TS_ASSERT_EQUALS(it1->cost.get(Cost::Duranium),    860);
        TS_ASSERT_EQUALS(it1->cost.get(Cost::Molybdenum), 1170);
        TS_ASSERT_EQUALS(it1->cost.get(Cost::Money),     42810);

        // Second ship -> same
        const CostSummary::Item* it2 = out.get(1);
        TS_ASSERT_EQUALS(it2->cost.get(Cost::Tritanium),  1063);
        TS_ASSERT_EQUALS(it2->cost.get(Cost::Duranium),    860);
        TS_ASSERT_EQUALS(it2->cost.get(Cost::Molybdenum), 1170);
        TS_ASSERT_EQUALS(it2->cost.get(Cost::Money),     42810);
    }

    // Compute cost for 6 using PlayerTech: all tech billed to first ship
    {
        FleetCostOptions opts;
        opts.shipTechMode = FleetCostOptions::PlayerTech;
        CostSummary out;
        computeFleetCosts(out, in, simConfig, opts, shipList, config, playerList, game::PlayerSet_t(6), tx);

        TS_ASSERT_EQUALS(out.getNumItems(), 2U);

        // First ship
        const CostSummary::Item* it1 = out.get(0);
        TS_ASSERT_EQUALS(it1->cost.get(Cost::Tritanium),  1063);
        TS_ASSERT_EQUALS(it1->cost.get(Cost::Duranium),    860);
        TS_ASSERT_EQUALS(it1->cost.get(Cost::Molybdenum), 1170);
        TS_ASSERT_EQUALS(it1->cost.get(Cost::Money),     42810);

        // Second ship -> same
        const CostSummary::Item* it2 = out.get(1);
        TS_ASSERT_EQUALS(it2->cost.get(Cost::Tritanium),  1063);
        TS_ASSERT_EQUALS(it2->cost.get(Cost::Duranium),    860);
        TS_ASSERT_EQUALS(it2->cost.get(Cost::Molybdenum), 1170);
        TS_ASSERT_EQUALS(it2->cost.get(Cost::Money),     29310);
    }
}

/** Test enums (getNext, toString). */
void
TestGameSimFleetCost::testEnums()
{
    using game::sim::FleetCostOptions;
    afl::string::NullTranslator tx;

    // TechMode
    {
        FleetCostOptions::TechMode mode = FleetCostOptions::NoTech;
        int n = 0;
        do {
            TS_ASSERT(toString(mode, tx).size() > 1);
            ++n;
            mode = getNext(mode);
            TS_ASSERT(n < 100);
        } while (mode != FleetCostOptions::NoTech);
    }

    // VcrMode
    {
        FleetCostOptions::FighterMode mode = FleetCostOptions::ShipFighters;
        int n = 0;
        do {
            TS_ASSERT(toString(mode, tx).size() > 1);
            ++n;
            mode = getNext(mode);
            TS_ASSERT(n < 100);
        } while (mode != FleetCostOptions::ShipFighters);
    }
}

