/**
  *  \file u/t_game_sim_resultlist.cpp
  *  \brief Test for game::sim::ResultList
  */

#include "game/sim/resultlist.hpp"

#include "t_game_sim.hpp"
#include "game/vcr/classic/database.hpp"
#include "game/sim/setup.hpp"
#include "game/sim/ship.hpp"
#include "game/sim/planet.hpp"

using game::vcr::Statistic;
using game::sim::Setup;

namespace {
    void addShip(Setup& setup, int owner, int damage, int fighters)
    {
        game::sim::Ship* sh = setup.addShip();
        TS_ASSERT(sh);
        sh->setOwner(owner);
        sh->setDamage(damage);
        sh->setNumBays(3);
        sh->setAmmo(fighters);
    }

    void addPlanet(Setup& setup, int owner, int fighters)
    {
        game::sim::Planet* pl = setup.addPlanet();
        TS_ASSERT(pl);
        pl->setOwner(owner);
        pl->setNumBaseFighters(fighters);
    }

    Statistic makeStatistic(int fighters)
    {
        game::vcr::Object obj;
        obj.setNumFighters(fighters);

        Statistic st;
        st.init(obj, 1);
        return st;
    }
}

void
TestGameSimResultList::testIt()
{
    game::sim::ResultList testee;

    // Add a class result (1x player 2, 2x player 7)
    game::sim::Result result;
    result.init(game::sim::Configuration(), 0);   // 0 to initialize
    result.battles = new game::vcr::classic::Database();

    {
        Setup before;
        addShip(before, 7,  0, 10);
        addShip(before, 2,  0, 70);
        addShip(before, 2, 50, 10);
        addPlanet(before, 2, 30);

        Setup after;
        addShip(after, 7,  20, 10);     // 20 damage taken
        addShip(after, 2,   0, 20);     // 50 fighters lost
        addShip(after, 0, 100, 0);
        addPlanet(after, 7, 20);        // 10 fighters lost

        Statistic stats[] = {
            makeStatistic(5),
            makeStatistic(15),
            makeStatistic(0),
            makeStatistic(10)
        };

        testee.addResult(before, after, stats, result);
    }

    // Verify
    TS_ASSERT_EQUALS(testee.getCumulativeWeight(), 1);
    TS_ASSERT_EQUALS(testee.getTotalWeight(), 1);
    TS_ASSERT_EQUALS(testee.getNumClassResults(), 1U);
    TS_ASSERT_EQUALS(testee.getNumUnitResults(), 4U);
    TS_ASSERT_EQUALS(testee.getNumBattles(), 1U);

    TS_ASSERT_EQUALS(testee.getUnitResult(0)->getNumFightsWon(), 1);
    TS_ASSERT_EQUALS(testee.getUnitResult(1)->getNumFightsWon(), 1);
    TS_ASSERT_EQUALS(testee.getUnitResult(2)->getNumFightsWon(), 0);
    TS_ASSERT_EQUALS(testee.getUnitResult(3)->getNumFightsWon(), 0);

    TS_ASSERT_EQUALS(testee.getUnitResult(0)->getMinFightersAboard().min, 5);
    TS_ASSERT_EQUALS(testee.getUnitResult(1)->getMinFightersAboard().min, 15);
    TS_ASSERT_EQUALS(testee.getUnitResult(2)->getMinFightersAboard().min, 0);
    TS_ASSERT_EQUALS(testee.getUnitResult(3)->getMinFightersAboard().min, 10);

    TS_ASSERT_EQUALS(testee.getUnitResult(0)->getMinFightersAboard().minSpecimen, result.battles);
    TS_ASSERT_EQUALS(testee.getUnitResult(0)->getMinFightersAboard().maxSpecimen, result.battles);

    TS_ASSERT_EQUALS(testee.getClassResult(0)->getClass().get(2), 1);
    TS_ASSERT_EQUALS(testee.getClassResult(0)->getClass().get(7), 2);
    TS_ASSERT_EQUALS(testee.getClassResult(0)->getSampleBattle(), result.battles);
}

void
TestGameSimResultList::testIncrease()
{
    game::sim::ResultList testee;

    // Add a class result (2x player 1) with weight 1
    game::sim::Result result1;
    result1.init(game::sim::Configuration(), 0);   // 0 to initialize
    result1.battles = new game::vcr::classic::Database();

    {
        Setup before; addShip(before,   1,  0, 10); addPlanet(before, 2, 30);
        Setup after;  addShip(after,    1, 20, 10); addPlanet(after,  1, 20);
        Statistic stats[] = { makeStatistic(8), makeStatistic(18) };

        testee.addResult(before, after, stats, result1);
    }

    // Add another class result (2x player 1) with weight 10
    game::sim::Result result2;
    result2.init(game::sim::Configuration(), 1);   // 1 to add
    result2.battles = new game::vcr::classic::Database();
    result2.addSeries(2);
    result2.total_battle_weight *= 10;
    result2.this_battle_weight *= 5;

    {
        Setup before; addShip(before,   1,  0, 10); addPlanet(before, 2, 30);
        Setup after;  addShip(after,    1, 20, 10); addPlanet(after,  1, 20);
        Statistic stats[] = { makeStatistic(4), makeStatistic(12) };
        testee.addResult(before, after, stats, result2);
    }

    // Verify
    TS_ASSERT_EQUALS(testee.getCumulativeWeight(), 15);       // first battle upscaled to 10, plus 5 from second
    TS_ASSERT_EQUALS(testee.getTotalWeight(), 10);            // from second battle
    TS_ASSERT_EQUALS(testee.getNumClassResults(), 1U);
    TS_ASSERT_EQUALS(testee.getNumUnitResults(), 2U);
    TS_ASSERT_EQUALS(testee.getNumBattles(), 2U);

    TS_ASSERT_EQUALS(testee.getUnitResult(0)->getMinFightersAboard().totalScaled, 100);   // 10x8 from first battle, 5x4 from second
    TS_ASSERT_EQUALS(testee.getUnitResult(0)->getMinFightersAboard().min, 4);
    TS_ASSERT_EQUALS(testee.getUnitResult(0)->getMinFightersAboard().max, 8);
    TS_ASSERT_EQUALS(testee.getUnitResult(0)->getMinFightersAboard().minSpecimen, result2.battles);
    TS_ASSERT_EQUALS(testee.getUnitResult(0)->getMinFightersAboard().maxSpecimen, result1.battles);
}

void
TestGameSimResultList::testDecrease()
{
    game::sim::ResultList testee;

    // Add class result (2x player 1) with weight 10 first
    game::sim::Result result2;
    result2.init(game::sim::Configuration(), 0);   // 0 to initialize
    result2.battles = new game::vcr::classic::Database();
    result2.addSeries(2);
    result2.total_battle_weight *= 10;
    result2.this_battle_weight *= 5;

    {
        Setup before; addShip(before,   1,  0, 10); addPlanet(before, 2, 30);
        Setup after;  addShip(after,    1, 20, 10); addPlanet(after,  1, 20);
        Statistic stats[] = { makeStatistic(4), makeStatistic(12) };
        testee.addResult(before, after, stats, result2);
    }

    // Add a class result (2x player 1) with weight 1
    game::sim::Result result1;
    result1.init(game::sim::Configuration(), 1);   // 0 to add
    result1.battles = new game::vcr::classic::Database();

    {
        Setup before; addShip(before,   1,  0, 10); addPlanet(before, 2, 30);
        Setup after;  addShip(after,    1, 20, 10); addPlanet(after,  1, 20);
        Statistic stats[] = { makeStatistic(8), makeStatistic(18) };

        testee.addResult(before, after, stats, result1);
    }

    // Verify
    TS_ASSERT_EQUALS(testee.getCumulativeWeight(), 15);       // first battle upscaled to 10, plus 5 from second
    TS_ASSERT_EQUALS(testee.getTotalWeight(), 10);            // from second battle
    TS_ASSERT_EQUALS(testee.getNumClassResults(), 1U);
    TS_ASSERT_EQUALS(testee.getNumUnitResults(), 2U);
    TS_ASSERT_EQUALS(testee.getNumBattles(), 2U);

    TS_ASSERT_EQUALS(testee.getUnitResult(0)->getMinFightersAboard().totalScaled, 100);   // 10x8 from first battle, 5x4 from second
    TS_ASSERT_EQUALS(testee.getUnitResult(0)->getMinFightersAboard().min, 4);
    TS_ASSERT_EQUALS(testee.getUnitResult(0)->getMinFightersAboard().max, 8);
    TS_ASSERT_EQUALS(testee.getUnitResult(0)->getMinFightersAboard().minSpecimen, result2.battles);
    TS_ASSERT_EQUALS(testee.getUnitResult(0)->getMinFightersAboard().maxSpecimen, result1.battles);
}

