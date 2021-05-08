/**
  *  \file u/t_game_score_tablebuilder.cpp
  *  \brief Test for game::score::TableBuilder
  */

#include "game/score/tablebuilder.hpp"

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

                // - one turn
                game::score::TurnScore& ta = scores.addTurn(10, game::Timestamp(2000, 10, 10, 12, 0, 0));
                ta.set(cap, 4, 10);
                ta.set(cap, 5, 4);

                // - another turn
                game::score::TurnScore& tb = scores.addTurn(11, game::Timestamp(2000, 10, 11, 12, 0, 0));
                tb.set(cap, 4, 11);
                tb.set(cap, 5, 3);

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
    A: create a TableBuilder. Build standard and difference tables.
    E: verify correct meta-information. Verify correct table being built. */
void
TestGameScoreTableBuilder::testIt()
{
    TestHarness h;
    game::score::TableBuilder testee(h.scores, h.players, h.teams, h.host, h.config, h.tx);

    // There must be variants on offer
    TS_ASSERT(testee.getNumVariants() > 0);

    // Validate the variants
    for (size_t i = 0; i < testee.getNumVariants(); ++i) {
        const game::score::TableBuilder::Variant* v = testee.getVariant(i);
        TS_ASSERT(v != 0);
        TS_ASSERT(!v->name.empty());
        TS_ASSERT(v->score.isValid());
    }

    // Out-of-range access
    TS_ASSERT(testee.getVariant(testee.getNumVariants()) == 0);
    TS_ASSERT(testee.findVariant(game::score::CompoundScore(h.scores, 1000, 1), 0) == 0);

    // Find the "capital ships" score and cross-check
    const game::score::CompoundScore capitalScore(h.scores, game::score::ScoreId_Capital, 1);
    size_t capitalIndex = 0;
    const game::score::TableBuilder::Variant* capitalVariant = testee.findVariant(capitalScore, &capitalIndex);
    TS_ASSERT(capitalVariant != 0);
    TS_ASSERT_EQUALS(capitalVariant, testee.getVariant(capitalIndex));
    TS_ASSERT_EQUALS(capitalVariant, testee.findVariant(capitalScore, 0));

    // Build the score table for turn index 0
    testee.setTurnIndex(0);
    {
        std::auto_ptr<util::DataTable> table(testee.build());
        TS_ASSERT(table.get() != 0);

        // Verify content: check the "capital ships" row
        //            ...    capital
        //  player 4           10
        //  player 5            4
        TS_ASSERT_EQUALS(table->getNumRows(), 2U);
        TS_ASSERT_EQUALS(table->getRow(0)->getName(), "The Klingons");
        TS_ASSERT_EQUALS(table->getRow(0)->get(int(capitalIndex)).orElse(-99), 10);
        TS_ASSERT_EQUALS(table->getRow(1)->getName(), "The Orions");
        TS_ASSERT_EQUALS(table->getRow(1)->get(int(capitalIndex)).orElse(-99), 4);

        TS_ASSERT_EQUALS(table->getColumnName(int(capitalIndex)), capitalVariant->name);
    }

    // Same for turn index 1
    testee.setTurnIndex(1);
    {
        std::auto_ptr<util::DataTable> table(testee.build());
        TS_ASSERT(table.get() != 0);

        //            ...    capital
        //  player 4           11
        //  player 5            3
        TS_ASSERT_EQUALS(table->getNumRows(), 2U);
        TS_ASSERT_EQUALS(table->getRow(0)->getName(), "The Klingons");
        TS_ASSERT_EQUALS(table->getRow(0)->get(int(capitalIndex)).orElse(-99), 11);
        TS_ASSERT_EQUALS(table->getRow(1)->getName(), "The Orions");
        TS_ASSERT_EQUALS(table->getRow(1)->get(int(capitalIndex)).orElse(-99), 3);
    }

    // Same for difference. Note that -1 must be a permitted value!
    testee.setTurnDifferenceIndexes(1, 0);
    {
        std::auto_ptr<util::DataTable> table(testee.build());
        TS_ASSERT(table.get() != 0);

        //            ...    capital
        //  player 4           +1
        //  player 5           -1
        TS_ASSERT_EQUALS(table->getNumRows(), 2U);
        TS_ASSERT_EQUALS(table->getRow(0)->getName(), "The Klingons");
        TS_ASSERT_EQUALS(table->getRow(0)->get(int(capitalIndex)).orElse(-99), +1);
        TS_ASSERT_EQUALS(table->getRow(1)->getName(), "The Orions");
        TS_ASSERT_EQUALS(table->getRow(1)->get(int(capitalIndex)).orElse(-99), -1);
    }
}

/** Test teams.
    A: create a TableBuilder. Enable by-teams. Build table.
    E: verify correct table being built. */
void
TestGameScoreTableBuilder::testTeams()
{
    TestHarness h;
    game::score::TableBuilder testee(h.scores, h.players, h.teams, h.host, h.config, h.tx);

    // Find the "capital ships" score and cross-check
    const game::score::CompoundScore capitalScore(h.scores, game::score::ScoreId_Capital, 1);
    size_t capitalIndex = 0;
    TS_ASSERT(testee.findVariant(capitalScore, &capitalIndex) != 0);

    // Verify content of teams
    testee.setTurnIndex(0);
    testee.setByTeam(true);
    std::auto_ptr<util::DataTable> table(testee.build());
    TS_ASSERT(table.get() != 0);

    //         ...   capital
    // Me              14
    TS_ASSERT_EQUALS(table->getNumRows(), 1U);
    TS_ASSERT_EQUALS(table->getRow(0)->getName(), "Me");
    TS_ASSERT_EQUALS(table->getRow(0)->get(int(capitalIndex)).orElse(-99), 14);
}

