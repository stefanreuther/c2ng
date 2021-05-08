/**
  *  \file u/t_game_score_chartbuilder.cpp
  *  \brief Test for game::score::ChartBuilder
  */

#include "game/score/chartbuilder.hpp"

#include "t_game_score.hpp"
#include "afl/string/nulltranslator.hpp"
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
void
TestGameScoreChartBuilder::testIt()
{
    TestHarness h;
    game::score::ChartBuilder testee(h.scores, h.players, h.teams, h.host, h.config, h.tx);

    // There must be variants on offer
    TS_ASSERT(testee.getNumVariants() > 0);

    // Validate the variants
    for (size_t i = 0; i < testee.getNumVariants(); ++i) {
        const game::score::ChartBuilder::Variant* v = testee.getVariant(i);
        TS_ASSERT(v != 0);
        TS_ASSERT(!v->name.empty());
        TS_ASSERT(v->score.isValid());
    }

    // Out-of-range access
    TS_ASSERT(testee.getVariant(testee.getNumVariants()) == 0);
    TS_ASSERT(testee.findVariant(game::score::CompoundScore(h.scores, 1000, 1), 0) == 0);

    // Find the "total ships" score and cross-check
    const game::score::CompoundScore totalScore(h.scores, game::score::CompoundScore::TotalShips);
    size_t totalIndex = 0;
    const game::score::ChartBuilder::Variant* totalVariant = testee.findVariant(totalScore, &totalIndex);
    TS_ASSERT(totalVariant != 0);
    TS_ASSERT_EQUALS(totalVariant, testee.getVariant(totalIndex));
    TS_ASSERT_EQUALS(totalVariant, testee.findVariant(totalScore, 0));

    // Build the score table
    testee.setVariantIndex(totalIndex);
    std::auto_ptr<util::DataTable> table(testee.build());
    TS_ASSERT(table.get() != 0);

    // Verify content of score table
    //            turn 10     turn 11
    // player 4    13           14
    // player 5    11           13
    TS_ASSERT_EQUALS(table->getNumRows(), 2U);
    TS_ASSERT_EQUALS(table->getValueRange().min(), 11);
    TS_ASSERT_EQUALS(table->getValueRange().max(), 14);
    TS_ASSERT_EQUALS(table->getNumColumns(), 2);
    TS_ASSERT_EQUALS(table->getColumnName(0), "Turn 10");
    TS_ASSERT_EQUALS(table->getColumnName(1), "Turn 11");

    const util::DataTable::Row* c1 = table->getRow(0);
    TS_ASSERT(c1 != 0);
    TS_ASSERT_EQUALS(c1->getId(), 4);
    TS_ASSERT_EQUALS(c1->getName(), "The Klingons");
    TS_ASSERT_EQUALS(c1->get(0).orElse(-1), 13);
    TS_ASSERT_EQUALS(c1->get(1).orElse(-1), 14);

    const util::DataTable::Row* c2 = table->getRow(1);
    TS_ASSERT(c2 != 0);
    TS_ASSERT_EQUALS(c2->getId(), 5);
    TS_ASSERT_EQUALS(c2->getName(), "The Orions");
    TS_ASSERT_EQUALS(c2->get(0).orElse(-1), 11);
    TS_ASSERT_EQUALS(c2->get(1).orElse(-1), 13);
}

/** Test teams.
    A: create a ChartBuilder. Enable by-teams. Build table.
    E: verify correct table being built. */
void
TestGameScoreChartBuilder::testTeam()
{
    TestHarness h;
    game::score::ChartBuilder testee(h.scores, h.players, h.teams, h.host, h.config, h.tx);

    // Find the "total ships" score
    const game::score::CompoundScore totalScore(h.scores, game::score::CompoundScore::TotalShips);
    size_t totalIndex = 0;
    TS_ASSERT(testee.findVariant(totalScore, &totalIndex) != 0);

    // Build the score table
    testee.setByTeam(true);
    testee.setVariantIndex(totalIndex);
    std::auto_ptr<util::DataTable> table(testee.build());
    TS_ASSERT(table.get() != 0);

    // Verify content of score table
    //            turn 10     turn 11
    // team 4     13+11        14+13
    TS_ASSERT_EQUALS(table->getNumRows(), 1U);
    TS_ASSERT_EQUALS(table->getValueRange().min(), 24);
    TS_ASSERT_EQUALS(table->getValueRange().max(), 27);
    TS_ASSERT_EQUALS(table->getNumColumns(), 2);

    const util::DataTable::Row* c1 = table->getRow(0);
    TS_ASSERT(c1 != 0);
    TS_ASSERT_EQUALS(c1->getId(), 4);
    TS_ASSERT_EQUALS(c1->getName(), "Me");
    TS_ASSERT_EQUALS(c1->get(0).orElse(-1), 24);
    TS_ASSERT_EQUALS(c1->get(1).orElse(-1), 27);
}

/** Test cumulative mode.
    A: create a ChartBuilder. Enable cumulative mode. Build table.
    E: verify correct table being built. */
void
TestGameScoreChartBuilder::testCumulative()
{
    TestHarness h;
    game::score::ChartBuilder testee(h.scores, h.players, h.teams, h.host, h.config, h.tx);

    // Find the "total ships" score
    const game::score::CompoundScore totalScore(h.scores, game::score::CompoundScore::TotalShips);
    size_t totalIndex = 0;
    TS_ASSERT(testee.findVariant(totalScore, &totalIndex) != 0);

    // Build the score table
    testee.setCumulativeMode(true);
    testee.setVariantIndex(totalIndex);
    std::auto_ptr<util::DataTable> table(testee.build());
    TS_ASSERT(table.get() != 0);

    // Verify content of score table
    //            turn 10     turn 11
    // player 4    13           14
    // player 5   11+13        13+14
    TS_ASSERT_EQUALS(table->getNumRows(), 2U);
    TS_ASSERT_EQUALS(table->getValueRange().min(), 13);
    TS_ASSERT_EQUALS(table->getValueRange().max(), 27);
    TS_ASSERT_EQUALS(table->getNumColumns(), 2);

    const util::DataTable::Row* c1 = table->getRow(0);
    TS_ASSERT(c1 != 0);
    TS_ASSERT_EQUALS(c1->getId(), 4);
    TS_ASSERT_EQUALS(c1->getName(), "The Klingons");
    TS_ASSERT_EQUALS(c1->get(0).orElse(-1), 13);
    TS_ASSERT_EQUALS(c1->get(1).orElse(-1), 14);

    const util::DataTable::Row* c2 = table->getRow(1);
    TS_ASSERT(c2 != 0);
    TS_ASSERT_EQUALS(c2->getId(), 5);
    TS_ASSERT_EQUALS(c2->getName(), "The Orions");
    TS_ASSERT_EQUALS(c2->get(0).orElse(-1), 11+13);
    TS_ASSERT_EQUALS(c2->get(1).orElse(-1), 13+14);
}

/** Test handling of sparse data.
    A: add a turn with gaps. create a ChartBuilder. Build standard table.
    E: verify correct table being built. */
void
TestGameScoreChartBuilder::testSparse()
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
    TS_ASSERT(testee.findVariant(totalScore, &totalIndex) != 0);

    // Build the score table
    testee.setVariantIndex(totalIndex);
    std::auto_ptr<util::DataTable> table(testee.build());
    TS_ASSERT(table.get() != 0);

    // Verify content of score table
    //            turn 10  turn 11  turn 12  turn 13
    // player 4    13       14       -        -
    // player 5    11       13       -        17
    TS_ASSERT_EQUALS(table->getNumRows(), 2U);
    TS_ASSERT_EQUALS(table->getValueRange().min(), 11);
    TS_ASSERT_EQUALS(table->getValueRange().max(), 17);
    TS_ASSERT_EQUALS(table->getNumColumns(), 4);

    const util::DataTable::Row* c1 = table->getRow(0);
    TS_ASSERT(c1 != 0);
    TS_ASSERT_EQUALS(c1->getId(), 4);
    TS_ASSERT_EQUALS(c1->getName(), "The Klingons");
    TS_ASSERT_EQUALS(c1->get(0).orElse(-1), 13);
    TS_ASSERT_EQUALS(c1->get(1).orElse(-1), 14);
    TS_ASSERT_EQUALS(c1->get(2).orElse(-1), -1);
    TS_ASSERT_EQUALS(c1->get(3).orElse(-1), -1);

    const util::DataTable::Row* c2 = table->getRow(1);
    TS_ASSERT(c2 != 0);
    TS_ASSERT_EQUALS(c2->getId(), 5);
    TS_ASSERT_EQUALS(c2->getName(), "The Orions");
    TS_ASSERT_EQUALS(c2->get(0).orElse(-1), 11);
    TS_ASSERT_EQUALS(c2->get(1).orElse(-1), 13);
    TS_ASSERT_EQUALS(c2->get(2).orElse(-1), -1);
    TS_ASSERT_EQUALS(c2->get(3).orElse(-1), 17);
}

