/**
  *  \file test/game/score/tablebuildertest.cpp
  *  \brief Test for game::score::TableBuilder
  */

#include "game/score/tablebuilder.hpp"

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
        afl::base::Ref<game::config::HostConfiguration> config;
        afl::string::NullTranslator tx;

        TestHarness()
            : scores(), players(), teams(), host(game::HostVersion::PHost, MKVERSION(3,0,0)), config(game::config::HostConfiguration::create()), tx()
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
AFL_TEST("game.score.TableBuilder:basics", a)
{
    TestHarness h;
    game::score::TableBuilder testee(h.scores, h.players, h.teams, h.host, *h.config, h.tx);

    // There must be variants on offer
    a.check("01. getNumVariants", testee.getNumVariants() > 0);

    // Validate the variants
    for (size_t i = 0; i < testee.getNumVariants(); ++i) {
        const game::score::TableBuilder::Variant* v = testee.getVariant(i);
        a.checkNonNull("11. getVariant", v);
        a.check("12. name", !v->name.empty());
        a.check("13. score", v->score.isValid());
    }

    // Out-of-range access
    a.checkNull("21. getVariant", testee.getVariant(testee.getNumVariants()));
    a.checkNull("22. findVariant", testee.findVariant(game::score::CompoundScore(h.scores, 1000, 1), 0));

    // Find the "capital ships" score and cross-check
    const game::score::CompoundScore capitalScore(h.scores, game::score::ScoreId_Capital, 1);
    size_t capitalIndex = 0;
    const game::score::TableBuilder::Variant* capitalVariant = testee.findVariant(capitalScore, &capitalIndex);
    a.checkNonNull("31. findVariant", capitalVariant);
    a.checkEqual("32. getVariant", capitalVariant, testee.getVariant(capitalIndex));
    a.checkEqual("33. findVariant", capitalVariant, testee.findVariant(capitalScore, 0));

    // Build the score table for turn index 0
    testee.setTurnIndex(0);
    {
        std::auto_ptr<util::DataTable> table(testee.build());
        a.checkNonNull("41. build", table.get());

        // Verify content: check the "capital ships" row
        //            ...    capital
        //  player 4           10
        //  player 5            4
        a.checkEqual("51. getNumRows", table->getNumRows(), 2U);
        a.checkEqual("52. getName",    table->getRow(0)->getName(), "The Klingons");
        a.checkEqual("53. get",        table->getRow(0)->get(int(capitalIndex)).orElse(-99), 10);
        a.checkEqual("54. getName",    table->getRow(1)->getName(), "The Orions");
        a.checkEqual("55. get",        table->getRow(1)->get(int(capitalIndex)).orElse(-99), 4);

        a.checkEqual("61. getColumnName", table->getColumnName(int(capitalIndex)), capitalVariant->name);
    }

    // Same for turn index 1
    testee.setTurnIndex(1);
    {
        std::auto_ptr<util::DataTable> table(testee.build());
        a.checkNonNull("71", table.get());

        //            ...    capital
        //  player 4           11
        //  player 5            3
        a.checkEqual("81. getNumRows", table->getNumRows(), 2U);
        a.checkEqual("82. getName",    table->getRow(0)->getName(), "The Klingons");
        a.checkEqual("83. get",        table->getRow(0)->get(int(capitalIndex)).orElse(-99), 11);
        a.checkEqual("84. getName",    table->getRow(1)->getName(), "The Orions");
        a.checkEqual("85. get",        table->getRow(1)->get(int(capitalIndex)).orElse(-99), 3);
    }

    // Same for difference. Note that -1 must be a permitted value!
    testee.setTurnDifferenceIndexes(1, 0);
    {
        std::auto_ptr<util::DataTable> table(testee.build());
        a.checkNonNull("91", table.get());

        //            ...    capital
        //  player 4           +1
        //  player 5           -1
        a.checkEqual("101. getNumRows", table->getNumRows(), 2U);
        a.checkEqual("102. getName",    table->getRow(0)->getName(), "The Klingons");
        a.checkEqual("103. get",        table->getRow(0)->get(int(capitalIndex)).orElse(-99), +1);
        a.checkEqual("104. getName",    table->getRow(1)->getName(), "The Orions");
        a.checkEqual("105. get",        table->getRow(1)->get(int(capitalIndex)).orElse(-99), -1);
    }
}

/** Test teams.
    A: create a TableBuilder. Enable by-teams. Build table.
    E: verify correct table being built. */
AFL_TEST("game.score.TableBuilder:setByTeam", a)
{
    TestHarness h;
    game::score::TableBuilder testee(h.scores, h.players, h.teams, h.host, *h.config, h.tx);

    // Find the "capital ships" score and cross-check
    const game::score::CompoundScore capitalScore(h.scores, game::score::ScoreId_Capital, 1);
    size_t capitalIndex = 0;
    a.checkNonNull("01. findVariant", testee.findVariant(capitalScore, &capitalIndex));

    // Verify content of teams
    testee.setTurnIndex(0);
    testee.setByTeam(true);
    std::auto_ptr<util::DataTable> table(testee.build());
    a.checkNonNull("11", table.get());

    //         ...   capital
    // Me              14
    a.checkEqual("21. getNumRows", table->getNumRows(), 1U);
    a.checkEqual("22. getName",    table->getRow(0)->getName(), "Me");
    a.checkEqual("23. get",        table->getRow(0)->get(int(capitalIndex)).orElse(-99), 14);
}
