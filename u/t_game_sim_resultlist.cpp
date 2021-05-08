/**
  *  \file u/t_game_sim_resultlist.cpp
  *  \brief Test for game::sim::ResultList
  */

#include "game/sim/resultlist.hpp"

#include "t_game_sim.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/sim/planet.hpp"
#include "game/sim/setup.hpp"
#include "game/sim/ship.hpp"
#include "game/vcr/classic/database.hpp"

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

    void addTorpedoShip(Setup& setup, int owner, int damage, int torps)
    {
        game::sim::Ship* sh = setup.addShip();
        TS_ASSERT(sh);
        sh->setOwner(owner);
        sh->setDamage(damage);
        sh->setNumLaunchers(4);
        sh->setTorpedoType(3);
        sh->setAmmo(torps);
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

    game::sim::Result makeResult(int index)
    {
        game::sim::Result r;
        r.init(game::sim::Configuration(), index);   // 0 to initialize
        r.battles = new game::vcr::classic::Database();
        return r;
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
    TS_ASSERT_EQUALS(testee.getLastClassResultIndex(), 0U);

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

    util::NumberFormatter fmt(false, false);
    TS_ASSERT_EQUALS(testee.describeClassResult(0, fmt).label, "1\xC3\x97 (100.0%)");
    TS_ASSERT_EQUALS(testee.describeClassResult(0, fmt).ownedUnits.get(7), 2);
    TS_ASSERT_EQUALS(testee.describeClassResult(0, fmt).weight, 1);
    TS_ASSERT_EQUALS(testee.describeClassResult(0, fmt).hasSample, true);
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

    util::NumberFormatter fmt(false, false);
    TS_ASSERT_EQUALS(testee.describeClassResult(0, fmt).label, "100.0%");
    TS_ASSERT_EQUALS(testee.describeClassResult(0, fmt).ownedUnits.get(1), 2);
    TS_ASSERT_EQUALS(testee.describeClassResult(0, fmt).weight, 15);
    TS_ASSERT_EQUALS(testee.describeClassResult(0, fmt).hasSample, true);
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

void
TestGameSimResultList::testMultipleClasses()
{
    // Setups
    Setup before; addShip(before, 1, 0, 10);    addShip(before, 1, 0, 10);    addShip(before, 2, 0, 10);
    Setup after1; addShip(after1, 1, 30, 10);   addShip(after1, 0, 100, 10);  addShip(after1, 0, 100, 10);
    Setup after2; addShip(after2, 1, 30, 10);   addShip(after2, 1, 30, 10);   addShip(after2, 0, 100, 10);
    Setup after3; addShip(after3, 0, 100, 10);  addShip(after3, 0, 100, 10);  addShip(after3, 2, 80, 10);
    Statistic stats[] = { makeStatistic(8), makeStatistic(18) };

    // Create ResultList with one result
    game::sim::ResultList testee;
    testee.addResult(before, after1, stats, makeResult(0));  // 0 to initialize

    // Result should be
    //        Fed Liz
    //   1x    1   0
    TS_ASSERT_EQUALS(testee.getNumClassResults(), 1U);
    TS_ASSERT_EQUALS(testee.getLastClassResultIndex(), 0U);
    TS_ASSERT_EQUALS(testee.getClassResult(0)->getClass().get(1), 1);
    TS_ASSERT_EQUALS(testee.getClassResult(0)->getClass().get(2), 0);

    // Add more results
    testee.addResult(before, after1, stats, makeResult(1));
    testee.addResult(before, after2, stats, makeResult(2));

    // Result should be
    //        Fed Liz
    //   2x    1   0
    //   1x    2   0
    TS_ASSERT_EQUALS(testee.getNumClassResults(), 2U);
    TS_ASSERT_EQUALS(testee.getLastClassResultIndex(), 1U);
    TS_ASSERT_EQUALS(testee.getClassResult(0)->getClass().get(1), 1);
    TS_ASSERT_EQUALS(testee.getClassResult(0)->getClass().get(2), 0);
    TS_ASSERT_EQUALS(testee.getClassResult(1)->getClass().get(1), 2);
    TS_ASSERT_EQUALS(testee.getClassResult(1)->getClass().get(2), 0);

    // Add one more. Always added to end.
    //        Fed Liz
    //   2x    1   0
    //   1x    2   0
    //   1x    0   1
    testee.addResult(before, after3, stats, makeResult(3));
    TS_ASSERT_EQUALS(testee.getNumClassResults(), 3U);
    TS_ASSERT_EQUALS(testee.getLastClassResultIndex(), 2U);

    // Add again to promote up
    testee.addResult(before, after3, stats, makeResult(3));
    TS_ASSERT_EQUALS(testee.getNumClassResults(), 3U);
    TS_ASSERT_EQUALS(testee.getLastClassResultIndex(), 1U);

    // Once more; verify final state
    //        Fed Liz
    //   3x    0   1
    //   2x    1   0
    //   1x    2   0
    testee.addResult(before, after3, stats, makeResult(3));
    TS_ASSERT_EQUALS(testee.getNumClassResults(), 3U);
    TS_ASSERT_EQUALS(testee.getLastClassResultIndex(), 0U);
    TS_ASSERT_EQUALS(testee.getClassResult(0)->getClass().get(1), 0);
    TS_ASSERT_EQUALS(testee.getClassResult(0)->getClass().get(2), 1);
    TS_ASSERT_EQUALS(testee.getClassResult(1)->getClass().get(1), 1);
    TS_ASSERT_EQUALS(testee.getClassResult(1)->getClass().get(2), 0);
    TS_ASSERT_EQUALS(testee.getClassResult(2)->getClass().get(1), 2);
    TS_ASSERT_EQUALS(testee.getClassResult(2)->getClass().get(2), 0);
}

void
TestGameSimResultList::testDescribe()
{
    // Setups
    Setup before; addShip(before, 1, 0, 10);    addShip(before, 1, 0, 10);
    Setup after1; addShip(after1, 1, 30, 10);   addShip(after1, 0, 100, 10);
    Statistic stats[] = { makeStatistic(8), makeStatistic(18) };

    // Create ResultList with one result
    game::sim::ResultList testee;
    game::sim::Result r = makeResult(0);  // 0 to initialize
    testee.addResult(before, after1, stats, r);

    // Describe the unit result
    game::sim::ResultList::UnitInfo info = testee.describeUnitResult(0, before);
    TS_ASSERT_EQUALS(info.numFights,    1);
    TS_ASSERT_EQUALS(info.numFightsWon, 1);
    TS_ASSERT_EQUALS(info.numCaptures,  0);

    // Regression: validate the infos
    TS_ASSERT_EQUALS(info.info.size(), 6U);
    TS_ASSERT_EQUALS(info.info[0].type, game::sim::ResultList::UnitInfo::Damage);
    TS_ASSERT_EQUALS(info.info[0].min,  30);
    TS_ASSERT_EQUALS(info.info[0].max,  30);
    TS_ASSERT_EQUALS(info.info[1].type, game::sim::ResultList::UnitInfo::Shield);
    TS_ASSERT_EQUALS(info.info[2].type, game::sim::ResultList::UnitInfo::Crew);
    TS_ASSERT_EQUALS(info.info[3].type, game::sim::ResultList::UnitInfo::NumFightersLost);
    TS_ASSERT_EQUALS(info.info[4].type, game::sim::ResultList::UnitInfo::NumFightersRemaining);
    TS_ASSERT_EQUALS(info.info[5].type, game::sim::ResultList::UnitInfo::MinFightersAboard);

    // Verify that everything is accessible
    afl::string::NullTranslator tx;
    for (size_t i = 0; i < info.info.size(); ++i) {
        TS_ASSERT_EQUALS(testee.getUnitSampleBattle(0, info.info[i].type, true).get(), r.battles.get());
        TS_ASSERT_DIFFERS(toString(info.info[i].type, tx), "");
    }
}

void
TestGameSimResultList::testDescribe2()
{
    // Setups
    Setup before; addTorpedoShip(before, 1,   0, 10); addPlanet(before, 2, 30);
    Setup after1; addTorpedoShip(after1, 0, 100,  1); addPlanet(after1, 2, 28);
    Statistic stats[] = { makeStatistic(0), makeStatistic(0) };

    // Create ResultList with one result
    game::sim::ResultList testee;
    game::sim::Result r = makeResult(0);  // 0 to initialize
    testee.addResult(before, after1, stats, r);

    // Describe the unit result
    game::sim::ResultList::UnitInfo info = testee.describeUnitResult(0, before);
    TS_ASSERT_EQUALS(info.numFights,    1);
    TS_ASSERT_EQUALS(info.numFightsWon, 0);
    TS_ASSERT_EQUALS(info.numCaptures,  0);

    // Regression: validate the ship information
    TS_ASSERT_EQUALS(info.info.size(), 6U);
    TS_ASSERT_EQUALS(info.info[0].type, game::sim::ResultList::UnitInfo::Damage);
    TS_ASSERT_EQUALS(info.info[0].min,  100);
    TS_ASSERT_EQUALS(info.info[0].max,  100);
    TS_ASSERT_EQUALS(info.info[1].type, game::sim::ResultList::UnitInfo::Shield);
    TS_ASSERT_EQUALS(info.info[2].type, game::sim::ResultList::UnitInfo::Crew);
    TS_ASSERT_EQUALS(info.info[3].type, game::sim::ResultList::UnitInfo::NumTorpedoesFired);
    TS_ASSERT_EQUALS(info.info[4].type, game::sim::ResultList::UnitInfo::NumTorpedoesRemaining);
    TS_ASSERT_EQUALS(info.info[5].type, game::sim::ResultList::UnitInfo::NumTorpedoHits);

    // Regression: validate the planet information
    info = testee.describeUnitResult(1, before);
    TS_ASSERT_EQUALS(info.numFights,    1);
    TS_ASSERT_EQUALS(info.numFightsWon, 1);
    TS_ASSERT_EQUALS(info.numCaptures,  0);

    TS_ASSERT_EQUALS(info.info.size(), 5U);
    TS_ASSERT_EQUALS(info.info[0].type, game::sim::ResultList::UnitInfo::Damage);
    TS_ASSERT_EQUALS(info.info[0].min,  0);
    TS_ASSERT_EQUALS(info.info[0].max,  0);
    TS_ASSERT_EQUALS(info.info[1].type, game::sim::ResultList::UnitInfo::Shield);
    TS_ASSERT_EQUALS(info.info[2].type, game::sim::ResultList::UnitInfo::DefenseLost);
    TS_ASSERT_EQUALS(info.info[3].type, game::sim::ResultList::UnitInfo::NumBaseFightersLost);
    TS_ASSERT_EQUALS(info.info[4].type, game::sim::ResultList::UnitInfo::MinFightersAboard);
}

void
TestGameSimResultList::testToString()
{
    afl::string::NullTranslator tx;
    for (size_t i = 0; i <= game::sim::ResultList::UnitInfo::MAX_TYPE; ++i) {
        TS_ASSERT_DIFFERS(game::sim::toString(static_cast<game::sim::ResultList::UnitInfo::Type>(i), tx), "");
    }
}

