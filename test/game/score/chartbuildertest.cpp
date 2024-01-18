/**
  *  \file test/game/score/chartbuildertest.cpp
  *  \brief Test for game::score::ChartBuilder
  */

#include "game/score/chartbuilder.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/hostversion.hpp"
#include "game/player.hpp"
#include "game/playerlist.hpp"
#include "game/teamsettings.hpp"

namespace {
    struct TestHarness {
        game::score::TurnScoreList scores;
        game::PlayerList players;
        game::TeamSettings teams;
        game::HostVersion host;
        game::config::HostConfiguration config;
        afl::string::NullTranslator tx;

        TestHarness()
            : scores(), players(), teams(), host(game::HostVersion::PHost, MKVERSION(3,0,0)), config(), tx()
            {
                // Add turns
                game::score::TurnScoreList::Slot_t cap = scores.addSlot(game::score::ScoreId_Capital);
                game::score::TurnScoreList::Slot_t fre = scores.addSlot(game::score::ScoreId_Freighters);

                // - one turn
                game::score::TurnScore& ta = scores.addTurn(10, game::Timestamp(2000, 10, 10, 12, 0, 0));
                ta.set(cap, 4, 10);
                ta.set(fre, 4, 3);
                ta.set(cap, 5, 4);
                ta.set(fre, 5, 7);

                // - another turn
                game::score::TurnScore& tb = scores.addTurn(11, game::Timestamp(2000, 10, 11, 12, 0, 0));
                tb.set(cap, 4, 11);
                tb.set(fre, 4, 3);
                tb.set(cap, 5, 4);
                tb.set(fre, 5, 9);

                // Add players
                players.create(4)->setName(game::Player::ShortName, "The Klingons");
                players.create(5)->setName(game::Player::ShortName, "The Orions");
                players.create(6)->initAlien();      // Aliens need to be ignored

                // Add teams
                teams.setPlayerTeam(4, 4);
                teams.setPlayerTeam(5, 4);
                teams.setTeamName(4, "Me");
            }
    };
}

/** Basic functionality test.
    A: create a ChartBuilder. Build standard table.
    E: verify correct meta-information. Verify correct table being built. */
AFL_TEST("game.score.ChartBuilder:basics", a)
{
    TestHarness h;
    game::score::ChartBuilder testee(h.scores, h.players, h.teams, h.host, h.config, h.tx);

    // There must be variants on offer
    a.check("01. getNumVariants", testee.getNumVariants() > 0);

    // Validate the variants
    for (size_t i = 0; i < testee.getNumVariants(); ++i) {
        const game::score::ChartBuilder::Variant* v = testee.getVariant(i);
        a.checkNonNull("11. getVariant", v);
        a.check("12. name", !v->name.empty());
        a.check("13. score", v->score.isValid());
    }

    // Out-of-range access
    a.checkNull("21. getVariant", testee.getVariant(testee.getNumVariants()));
    a.checkNull("22. findVariant", testee.findVariant(game::score::CompoundScore(h.scores, 1000, 1), 0));

    // Find the "total ships" score and cross-check
    const game::score::CompoundScore totalScore(h.scores, game::score::CompoundScore::TotalShips);
    size_t totalIndex = 0;
    const game::score::ChartBuilder::Variant* totalVariant = testee.findVariant(totalScore, &totalIndex);
    a.checkNonNull("31. totalVariant", totalVariant);
    a.checkEqual("32. getVariant", totalVariant, testee.getVariant(totalIndex));
    a.checkEqual("33. findVariant", totalVariant, testee.findVariant(totalScore, 0));

    // Build the score table
    testee.setVariantIndex(totalIndex);
    std::auto_ptr<util::DataTable> table(testee.build());
    a.checkNonNull("41. build", table.get());

    // Verify content of score table
    //            turn 10     turn 11
    // player 4    13           14
    // player 5    11           13
    a.checkEqual("51. getNumRows", table->getNumRows(), 2U);
    a.checkEqual("52. min", table->getValueRange().min(), 11);
    a.checkEqual("53. max", table->getValueRange().max(), 14);
    a.checkEqual("54. getNumColumns", table->getNumColumns(), 2);
    a.checkEqual("55. getColumnName", table->getColumnName(0), "Turn 10");
    a.checkEqual("56. getColumnName", table->getColumnName(1), "Turn 11");

    const util::DataTable::Row* c1 = table->getRow(0);
    a.checkNonNull("61. getRow", c1);
    a.checkEqual("62. getId", c1->getId(), 4);
    a.checkEqual("63. getName", c1->getName(), "The Klingons");
    a.checkEqual("64. get", c1->get(0).orElse(-1), 13);
    a.checkEqual("65. get", c1->get(1).orElse(-1), 14);

    const util::DataTable::Row* c2 = table->getRow(1);
    a.checkNonNull("71. getRow", c2);
    a.checkEqual("72. getId", c2->getId(), 5);
    a.checkEqual("73. getName", c2->getName(), "The Orions");
    a.checkEqual("74. get", c2->get(0).orElse(-1), 11);
    a.checkEqual("75. get", c2->get(1).orElse(-1), 13);
}

/** Test teams.
    A: create a ChartBuilder. Enable by-teams. Build table.
    E: verify correct table being built. */
AFL_TEST("game.score.ChartBuilder:setByTeam", a)
{
    TestHarness h;
    game::score::ChartBuilder testee(h.scores, h.players, h.teams, h.host, h.config, h.tx);

    // Find the "total ships" score
    const game::score::CompoundScore totalScore(h.scores, game::score::CompoundScore::TotalShips);
    size_t totalIndex = 0;
    a.checkNonNull("01. findVariant", testee.findVariant(totalScore, &totalIndex));

    // Build the score table
    testee.setByTeam(true);
    testee.setVariantIndex(totalIndex);
    std::auto_ptr<util::DataTable> table(testee.build());
    a.checkNonNull("11. build", table.get());

    // Verify content of score table
    //            turn 10     turn 11
    // team 4     13+11        14+13
    a.checkEqual("21. getNumRows", table->getNumRows(), 1U);
    a.checkEqual("22. min", table->getValueRange().min(), 24);
    a.checkEqual("23. max", table->getValueRange().max(), 27);
    a.checkEqual("24. getNumColumns", table->getNumColumns(), 2);

    const util::DataTable::Row* c1 = table->getRow(0);
    a.checkNonNull("31. getRow", c1);
    a.checkEqual("32. getId", c1->getId(), 4);
    a.checkEqual("33. getName", c1->getName(), "Me");
    a.checkEqual("34. get", c1->get(0).orElse(-1), 24);
    a.checkEqual("35. get", c1->get(1).orElse(-1), 27);
}

/** Test cumulative mode.
    A: create a ChartBuilder. Enable cumulative mode. Build table.
    E: verify correct table being built. */
AFL_TEST("game.score.ChartBuilder:setCumulativeMode", a)
{
    TestHarness h;
    game::score::ChartBuilder testee(h.scores, h.players, h.teams, h.host, h.config, h.tx);

    // Find the "total ships" score
    const game::score::CompoundScore totalScore(h.scores, game::score::CompoundScore::TotalShips);
    size_t totalIndex = 0;
    a.checkNonNull("01. findVariant", testee.findVariant(totalScore, &totalIndex));

    // Build the score table
    testee.setCumulativeMode(true);
    testee.setVariantIndex(totalIndex);
    std::auto_ptr<util::DataTable> table(testee.build());
    a.checkNonNull("11. build", table.get());

    // Verify content of score table
    //            turn 10     turn 11
    // player 4    13           14
    // player 5   11+13        13+14
    a.checkEqual("21. getNumRows", table->getNumRows(), 2U);
    a.checkEqual("22. min", table->getValueRange().min(), 13);
    a.checkEqual("23. max", table->getValueRange().max(), 27);
    a.checkEqual("24. getNumColumns", table->getNumColumns(), 2);

    const util::DataTable::Row* c1 = table->getRow(0);
    a.checkNonNull("31. getRow", c1);
    a.checkEqual("32. getId", c1->getId(), 4);
    a.checkEqual("33. getName", c1->getName(), "The Klingons");
    a.checkEqual("34. get", c1->get(0).orElse(-1), 13);
    a.checkEqual("35. get", c1->get(1).orElse(-1), 14);

    const util::DataTable::Row* c2 = table->getRow(1);
    a.checkNonNull("41. getRow", c2);
    a.checkEqual("42. getId", c2->getId(), 5);
    a.checkEqual("43. getName", c2->getName(), "The Orions");
    a.checkEqual("44. get", c2->get(0).orElse(-1), 11+13);
    a.checkEqual("45. get", c2->get(1).orElse(-1), 13+14);
}

/** Test handling of sparse data.
    A: add a turn with gaps. create a ChartBuilder. Build standard table.
    E: verify correct table being built. */
AFL_TEST("game.score.ChartBuilder:sparse", a)
{
    TestHarness h;

    // TestHarness contains turns 10+11. Add turn 13 with data just for player 5.
    game::score::TurnScoreList::Slot_t cap = h.scores.addSlot(game::score::ScoreId_Capital);
    game::score::TurnScoreList::Slot_t fre = h.scores.addSlot(game::score::ScoreId_Freighters);
    game::score::TurnScore& tc = h.scores.addTurn(13, game::Timestamp(2000, 11, 1, 12, 0, 0));
    tc.set(cap, 5, 7);
    tc.set(fre, 5, 10);

    // Find the "total ships" score
    game::score::ChartBuilder testee(h.scores, h.players, h.teams, h.host, h.config, h.tx);
    const game::score::CompoundScore totalScore(h.scores, game::score::CompoundScore::TotalShips);
    size_t totalIndex = 0;
    a.checkNonNull("01. findVariant", testee.findVariant(totalScore, &totalIndex));

    // Build the score table
    testee.setVariantIndex(totalIndex);
    std::auto_ptr<util::DataTable> table(testee.build());
    a.checkNonNull("11. build", table.get());

    // Verify content of score table
    //            turn 10  turn 11  turn 12  turn 13
    // player 4    13       14       -        -
    // player 5    11       13       -        17
    a.checkEqual("21. getNumRows", table->getNumRows(), 2U);
    a.checkEqual("22. min", table->getValueRange().min(), 11);
    a.checkEqual("23. max", table->getValueRange().max(), 17);
    a.checkEqual("24. getNumColumns", table->getNumColumns(), 4);

    const util::DataTable::Row* c1 = table->getRow(0);
    a.checkNonNull("31. getRow", c1);
    a.checkEqual("32. getId", c1->getId(), 4);
    a.checkEqual("33. getName", c1->getName(), "The Klingons");
    a.checkEqual("34. get", c1->get(0).orElse(-1), 13);
    a.checkEqual("35. get", c1->get(1).orElse(-1), 14);
    a.checkEqual("36. get", c1->get(2).orElse(-1), -1);
    a.checkEqual("37. get", c1->get(3).orElse(-1), -1);

    const util::DataTable::Row* c2 = table->getRow(1);
    a.checkNonNull("41. getRow", c2);
    a.checkEqual("42. getId", c2->getId(), 5);
    a.checkEqual("43. getName", c2->getName(), "The Orions");
    a.checkEqual("44. get", c2->get(0).orElse(-1), 11);
    a.checkEqual("45. get", c2->get(1).orElse(-1), 13);
    a.checkEqual("46. get", c2->get(2).orElse(-1), -1);
    a.checkEqual("47. get", c2->get(3).orElse(-1), 17);
}
