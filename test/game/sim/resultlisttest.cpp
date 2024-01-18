/**
  *  \file test/game/sim/resultlisttest.cpp
  *  \brief Test for game::sim::ResultList
  */

#include "game/sim/resultlist.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/sim/planet.hpp"
#include "game/sim/setup.hpp"
#include "game/sim/ship.hpp"
#include "game/vcr/classic/database.hpp"

using game::vcr::Statistic;
using game::sim::Setup;

namespace {
    void addShip(afl::test::Assert a, Setup& setup, int owner, int damage, int fighters)
    {
        game::sim::Ship* sh = setup.addShip();
        a.check("addShip", sh);
        sh->setOwner(owner);
        sh->setDamage(damage);
        sh->setNumBays(3);
        sh->setAmmo(fighters);
    }

    void addTorpedoShip(afl::test::Assert a, Setup& setup, int owner, int damage, int torps)
    {
        game::sim::Ship* sh = setup.addShip();
        a.check("addTorpedoShip", sh);
        sh->setOwner(owner);
        sh->setDamage(damage);
        sh->setNumLaunchers(4);
        sh->setTorpedoType(3);
        sh->setAmmo(torps);
    }

    void addPlanet(afl::test::Assert a, Setup& setup, int owner, int fighters)
    {
        game::sim::Planet* pl = setup.addPlanet();
        a.check("addPlanet", pl);
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

AFL_TEST("game.sim.ResultList:basics", a)
{
    game::sim::ResultList testee;

    // Add a class result (1x player 2, 2x player 7)
    game::sim::Result result;
    result.init(game::sim::Configuration(), 0);   // 0 to initialize
    result.battles = new game::vcr::classic::Database();

    {
        Setup before;
        addShip(a, before, 7,  0, 10);
        addShip(a, before, 2,  0, 70);
        addShip(a, before, 2, 50, 10);
        addPlanet(a, before, 2, 30);

        Setup after;
        addShip(a, after, 7,  20, 10);     // 20 damage taken
        addShip(a, after, 2,   0, 20);     // 50 fighters lost
        addShip(a, after, 0, 100, 0);
        addPlanet(a, after, 7, 20);        // 10 fighters lost

        Statistic stats[] = {
            makeStatistic(5),
            makeStatistic(15),
            makeStatistic(0),
            makeStatistic(10)
        };

        testee.addResult(before, after, stats, result);
    }

    // Verify
    a.checkEqual("01. getCumulativeWeight",     testee.getCumulativeWeight(), 1);
    a.checkEqual("02. getTotalWeight",          testee.getTotalWeight(), 1);
    a.checkEqual("03. getNumClassResults",      testee.getNumClassResults(), 1U);
    a.checkEqual("04. getNumUnitResults",       testee.getNumUnitResults(), 4U);
    a.checkEqual("05. getNumBattles",           testee.getNumBattles(), 1U);
    a.checkEqual("06. getLastClassResultIndex", testee.getLastClassResultIndex(), 0U);

    a.checkEqual("11. getNumFightsWon", testee.getUnitResult(0)->getNumFightsWon(), 1);
    a.checkEqual("12. getNumFightsWon", testee.getUnitResult(1)->getNumFightsWon(), 1);
    a.checkEqual("13. getNumFightsWon", testee.getUnitResult(2)->getNumFightsWon(), 0);
    a.checkEqual("14. getNumFightsWon", testee.getUnitResult(3)->getNumFightsWon(), 0);

    a.checkEqual("21. getMinFightersAboard.min", testee.getUnitResult(0)->getMinFightersAboard().min, 5);
    a.checkEqual("22. getMinFightersAboard.min", testee.getUnitResult(1)->getMinFightersAboard().min, 15);
    a.checkEqual("23. getMinFightersAboard.min", testee.getUnitResult(2)->getMinFightersAboard().min, 0);
    a.checkEqual("24. getMinFightersAboard.min", testee.getUnitResult(3)->getMinFightersAboard().min, 10);

    a.checkEqual("31. getMinFightersAboard.minSpecimen", testee.getUnitResult(0)->getMinFightersAboard().minSpecimen.get(), result.battles.get());
    a.checkEqual("32. getMinFightersAboard.maxSpecimen", testee.getUnitResult(0)->getMinFightersAboard().maxSpecimen.get(), result.battles.get());

    a.checkEqual("41. getClass", testee.getClassResult(0)->getClass().get(2), 1);
    a.checkEqual("42. getClass", testee.getClassResult(0)->getClass().get(7), 2);
    a.checkEqual("43. getSampleBattle", testee.getClassResult(0)->getSampleBattle().get(), result.battles.get());

    util::NumberFormatter fmt(false, false);
    a.checkEqual("51. label",      testee.describeClassResult(0, fmt).label, "1\xC3\x97 (100.0%)");
    a.checkEqual("52. ownedUnits", testee.describeClassResult(0, fmt).ownedUnits.get(7), 2);
    a.checkEqual("53. weight",     testee.describeClassResult(0, fmt).weight, 1);
    a.checkEqual("54. hasSample",  testee.describeClassResult(0, fmt).hasSample, true);
}

AFL_TEST("game.sim.ResultList:increase-weight", a)
{
    game::sim::ResultList testee;

    // Add a class result (2x player 1) with weight 1
    game::sim::Result result1;
    result1.init(game::sim::Configuration(), 0);   // 0 to initialize
    result1.battles = new game::vcr::classic::Database();

    {
        Setup before; addShip(a, before,   1,  0, 10); addPlanet(a, before, 2, 30);
        Setup after;  addShip(a, after,    1, 20, 10); addPlanet(a, after,  1, 20);
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
        Setup before; addShip(a, before,   1,  0, 10); addPlanet(a, before, 2, 30);
        Setup after;  addShip(a, after,    1, 20, 10); addPlanet(a, after,  1, 20);
        Statistic stats[] = { makeStatistic(4), makeStatistic(12) };
        testee.addResult(before, after, stats, result2);
    }

    // Verify
    a.checkEqual("01. getCumulativeWeight", testee.getCumulativeWeight(), 15);       // first battle upscaled to 10, plus 5 from second
    a.checkEqual("02. getTotalWeight",      testee.getTotalWeight(), 10);            // from second battle
    a.checkEqual("03. getNumClassResults",  testee.getNumClassResults(), 1U);
    a.checkEqual("04. getNumUnitResults",   testee.getNumUnitResults(), 2U);
    a.checkEqual("05. getNumBattles",       testee.getNumBattles(), 2U);

    a.checkEqual("11. getMinFightersAboard.totalScaled", testee.getUnitResult(0)->getMinFightersAboard().totalScaled, 100);   // 10x8 from first battle, 5x4 from second
    a.checkEqual("12. getMinFightersAboard.min",         testee.getUnitResult(0)->getMinFightersAboard().min, 4);
    a.checkEqual("13. getMinFightersAboard.max",         testee.getUnitResult(0)->getMinFightersAboard().max, 8);
    a.checkEqual("14. getMinFightersAboard.minSpecimen", testee.getUnitResult(0)->getMinFightersAboard().minSpecimen.get(), result2.battles.get());
    a.checkEqual("15. getMinFightersAboard.maxSpecimen", testee.getUnitResult(0)->getMinFightersAboard().maxSpecimen.get(), result1.battles.get());

    util::NumberFormatter fmt(false, false);
    a.checkEqual("21. label",      testee.describeClassResult(0, fmt).label, "100.0%");
    a.checkEqual("22. ownedUnits", testee.describeClassResult(0, fmt).ownedUnits.get(1), 2);
    a.checkEqual("23. weight",     testee.describeClassResult(0, fmt).weight, 15);
    a.checkEqual("24. hasSample",  testee.describeClassResult(0, fmt).hasSample, true);
}

AFL_TEST("game.sim.ResultList:decrease-weight", a)
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
        Setup before; addShip(a, before,   1,  0, 10); addPlanet(a, before, 2, 30);
        Setup after;  addShip(a, after,    1, 20, 10); addPlanet(a, after,  1, 20);
        Statistic stats[] = { makeStatistic(4), makeStatistic(12) };
        testee.addResult(before, after, stats, result2);
    }

    // Add a class result (2x player 1) with weight 1
    game::sim::Result result1;
    result1.init(game::sim::Configuration(), 1);   // 0 to add
    result1.battles = new game::vcr::classic::Database();

    {
        Setup before; addShip(a, before,   1,  0, 10); addPlanet(a, before, 2, 30);
        Setup after;  addShip(a, after,    1, 20, 10); addPlanet(a, after,  1, 20);
        Statistic stats[] = { makeStatistic(8), makeStatistic(18) };

        testee.addResult(before, after, stats, result1);
    }

    // Verify
    a.checkEqual("01. getCumulativeWeight", testee.getCumulativeWeight(), 15);       // first battle upscaled to 10, plus 5 from second
    a.checkEqual("02. getTotalWeight",      testee.getTotalWeight(), 10);            // from second battle
    a.checkEqual("03. getNumClassResults",  testee.getNumClassResults(), 1U);
    a.checkEqual("04. getNumUnitResults",   testee.getNumUnitResults(), 2U);
    a.checkEqual("05. getNumBattles",       testee.getNumBattles(), 2U);

    a.checkEqual("11. getMinFightersAboard.totalScaled", testee.getUnitResult(0)->getMinFightersAboard().totalScaled, 100);   // 10x8 from first battle, 5x4 from second
    a.checkEqual("12. getMinFightersAboard.min",         testee.getUnitResult(0)->getMinFightersAboard().min, 4);
    a.checkEqual("13. getMinFightersAboard.max",         testee.getUnitResult(0)->getMinFightersAboard().max, 8);
    a.checkEqual("14. getMinFightersAboard.minSpecimen", testee.getUnitResult(0)->getMinFightersAboard().minSpecimen.get(), result2.battles.get());
    a.checkEqual("15. getMinFightersAboard.maxSpecimen", testee.getUnitResult(0)->getMinFightersAboard().maxSpecimen.get(), result1.battles.get());
}

AFL_TEST("game.sim.ResultList:multiple-classes", a)
{
    // Setups
    Setup before; addShip(a, before, 1, 0, 10);    addShip(a, before, 1, 0, 10);    addShip(a, before, 2, 0, 10);
    Setup after1; addShip(a, after1, 1, 30, 10);   addShip(a, after1, 0, 100, 10);  addShip(a, after1, 0, 100, 10);
    Setup after2; addShip(a, after2, 1, 30, 10);   addShip(a, after2, 1, 30, 10);   addShip(a, after2, 0, 100, 10);
    Setup after3; addShip(a, after3, 0, 100, 10);  addShip(a, after3, 0, 100, 10);  addShip(a, after3, 2, 80, 10);
    Statistic stats[] = { makeStatistic(8), makeStatistic(18) };

    // Create ResultList with one result
    game::sim::ResultList testee;
    testee.addResult(before, after1, stats, makeResult(0));  // 0 to initialize

    // Result should be
    //        Fed Liz
    //   1x    1   0
    a.checkEqual("01. getNumClassResults",      testee.getNumClassResults(), 1U);
    a.checkEqual("02. getLastClassResultIndex", testee.getLastClassResultIndex(), 0U);
    a.checkEqual("03. getClassResult",          testee.getClassResult(0)->getClass().get(1), 1);
    a.checkEqual("04. getClassResult",          testee.getClassResult(0)->getClass().get(2), 0);

    // Add more results
    testee.addResult(before, after1, stats, makeResult(1));
    testee.addResult(before, after2, stats, makeResult(2));

    // Result should be
    //        Fed Liz
    //   2x    1   0
    //   1x    2   0
    a.checkEqual("11. getNumClassResults",      testee.getNumClassResults(), 2U);
    a.checkEqual("12. getLastClassResultIndex", testee.getLastClassResultIndex(), 1U);
    a.checkEqual("13. getClassResult",          testee.getClassResult(0)->getClass().get(1), 1);
    a.checkEqual("14. getClassResult",          testee.getClassResult(0)->getClass().get(2), 0);
    a.checkEqual("15. getClassResult",          testee.getClassResult(1)->getClass().get(1), 2);
    a.checkEqual("16. getClassResult",          testee.getClassResult(1)->getClass().get(2), 0);

    // Add one more. Always added to end.
    //        Fed Liz
    //   2x    1   0
    //   1x    2   0
    //   1x    0   1
    testee.addResult(before, after3, stats, makeResult(3));
    a.checkEqual("21. getNumClassResults",      testee.getNumClassResults(), 3U);
    a.checkEqual("22. getLastClassResultIndex", testee.getLastClassResultIndex(), 2U);

    // Add again to promote up
    testee.addResult(before, after3, stats, makeResult(3));
    a.checkEqual("31. getNumClassResults",      testee.getNumClassResults(), 3U);
    a.checkEqual("32. getLastClassResultIndex", testee.getLastClassResultIndex(), 1U);

    // Once more; verify final state
    //        Fed Liz
    //   3x    0   1
    //   2x    1   0
    //   1x    2   0
    testee.addResult(before, after3, stats, makeResult(3));
    a.checkEqual("41. getNumClassResults",      testee.getNumClassResults(), 3U);
    a.checkEqual("42. getLastClassResultIndex", testee.getLastClassResultIndex(), 0U);
    a.checkEqual("43. getClassResult",          testee.getClassResult(0)->getClass().get(1), 0);
    a.checkEqual("44. getClassResult",          testee.getClassResult(0)->getClass().get(2), 1);
    a.checkEqual("45. getClassResult",          testee.getClassResult(1)->getClass().get(1), 1);
    a.checkEqual("46. getClassResult",          testee.getClassResult(1)->getClass().get(2), 0);
    a.checkEqual("47. getClassResult",          testee.getClassResult(2)->getClass().get(1), 2);
    a.checkEqual("48. getClassResult",          testee.getClassResult(2)->getClass().get(2), 0);
}

AFL_TEST("game.sim.ResultList:describeUnitResult", a)
{
    // Setups
    Setup before; addShip(a, before, 1, 0, 10);    addShip(a, before, 1, 0, 10);
    Setup after1; addShip(a, after1, 1, 30, 10);   addShip(a, after1, 0, 100, 10);
    Statistic stats[] = { makeStatistic(8), makeStatistic(18) };

    // Create ResultList with one result
    game::sim::ResultList testee;
    game::sim::Result r = makeResult(0);  // 0 to initialize
    testee.addResult(before, after1, stats, r);

    // Describe the unit result
    game::sim::ResultList::UnitInfo info = testee.describeUnitResult(0, before);
    a.checkEqual("01. numFights",    info.numFights,    1);
    a.checkEqual("02. numFightsWon", info.numFightsWon, 1);
    a.checkEqual("03. numCaptures",  info.numCaptures,  0);

    // Regression: validate the infos
    a.checkEqual("11. size", info.info.size(), 6U);
    a.checkEqual("12. type", info.info[0].type, game::sim::ResultList::UnitInfo::Damage);
    a.checkEqual("13. min",  info.info[0].min,  30);
    a.checkEqual("14. max",  info.info[0].max,  30);
    a.checkEqual("15. type", info.info[1].type, game::sim::ResultList::UnitInfo::Shield);
    a.checkEqual("16. type", info.info[2].type, game::sim::ResultList::UnitInfo::Crew);
    a.checkEqual("17. type", info.info[3].type, game::sim::ResultList::UnitInfo::NumFightersLost);
    a.checkEqual("18. type", info.info[4].type, game::sim::ResultList::UnitInfo::NumFightersRemaining);
    a.checkEqual("19. type", info.info[5].type, game::sim::ResultList::UnitInfo::MinFightersAboard);

    // Verify that everything is accessible
    afl::string::NullTranslator tx;
    for (size_t i = 0; i < info.info.size(); ++i) {
        a.checkEqual("21. getUnitSampleBattle", testee.getUnitSampleBattle(0, info.info[i].type, true).get(), r.battles.get());
        a.checkDifferent("22. type", toString(info.info[i].type, tx), "");
    }
}

AFL_TEST("game.sim.ResultList:describeUnitResult:2", a)
{
    // Setups
    Setup before; addTorpedoShip(a, before, 1,   0, 10); addPlanet(a, before, 2, 30);
    Setup after1; addTorpedoShip(a, after1, 0, 100,  1); addPlanet(a, after1, 2, 28);
    Statistic stats[] = { makeStatistic(0), makeStatistic(0) };

    // Create ResultList with one result
    game::sim::ResultList testee;
    game::sim::Result r = makeResult(0);  // 0 to initialize
    testee.addResult(before, after1, stats, r);

    // Describe the unit result
    game::sim::ResultList::UnitInfo info = testee.describeUnitResult(0, before);
    a.checkEqual("01. numFights",    info.numFights,    1);
    a.checkEqual("02. numFightsWon", info.numFightsWon, 0);
    a.checkEqual("03. numCaptures",  info.numCaptures,  0);

    // Regression: validate the ship information
    a.checkEqual("11. size", info.info.size(), 6U);
    a.checkEqual("12. type", info.info[0].type, game::sim::ResultList::UnitInfo::Damage);
    a.checkEqual("13. min",  info.info[0].min,  100);
    a.checkEqual("14. max",  info.info[0].max,  100);
    a.checkEqual("15. type", info.info[1].type, game::sim::ResultList::UnitInfo::Shield);
    a.checkEqual("16. type", info.info[2].type, game::sim::ResultList::UnitInfo::Crew);
    a.checkEqual("17. type", info.info[3].type, game::sim::ResultList::UnitInfo::NumTorpedoesFired);
    a.checkEqual("18. type", info.info[4].type, game::sim::ResultList::UnitInfo::NumTorpedoesRemaining);
    a.checkEqual("19. type", info.info[5].type, game::sim::ResultList::UnitInfo::NumTorpedoHits);

    // Regression: validate the planet information
    info = testee.describeUnitResult(1, before);
    a.checkEqual("21. numFights",    info.numFights,    1);
    a.checkEqual("22. numFightsWon", info.numFightsWon, 1);
    a.checkEqual("23. numCaptures",  info.numCaptures,  0);

    a.checkEqual("31. size", info.info.size(), 5U);
    a.checkEqual("32. type", info.info[0].type, game::sim::ResultList::UnitInfo::Damage);
    a.checkEqual("33. min",  info.info[0].min,  0);
    a.checkEqual("34. max",  info.info[0].max,  0);
    a.checkEqual("35. type", info.info[1].type, game::sim::ResultList::UnitInfo::Shield);
    a.checkEqual("36. type", info.info[2].type, game::sim::ResultList::UnitInfo::DefenseLost);
    a.checkEqual("37. type", info.info[3].type, game::sim::ResultList::UnitInfo::NumBaseFightersLost);
    a.checkEqual("38. type", info.info[4].type, game::sim::ResultList::UnitInfo::MinFightersAboard);
}

AFL_TEST("game.sim.ResultList:toString", a)
{
    afl::string::NullTranslator tx;
    for (size_t i = 0; i <= game::sim::ResultList::UnitInfo::MAX_TYPE; ++i) {
        a.checkDifferent("01", game::sim::toString(static_cast<game::sim::ResultList::UnitInfo::Type>(i), tx), "");
    }
}
