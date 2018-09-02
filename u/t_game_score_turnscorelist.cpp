/**
  *  \file u/t_game_score_turnscorelist.cpp
  *  \brief Test for game::score::TurnScoreList
  */

#include "game/score/turnscorelist.hpp"

#include "t_game_score.hpp"

namespace gp = game::parser;

/** Test standard schema. */
void
TestGameScoreTurnScoreList::testSchema()
{
    game::score::TurnScoreList testee;
    game::score::TurnScoreList::Slot_t slot;
    game::score::TurnScoreList::Slot_t slot2;
    game::score::ScoreId_t scoreId;

    // Default schema must contain these scores:
    TS_ASSERT(testee.getSlot(game::score::ScoreId_Planets, slot));
    TS_ASSERT(testee.getSlot(game::score::ScoreId_Capital, slot));
    TS_ASSERT(testee.getSlot(game::score::ScoreId_Freighters, slot));
    TS_ASSERT(testee.getSlot(game::score::ScoreId_Bases, slot));
    TS_ASSERT(testee.getSlot(game::score::ScoreId_BuildPoints, slot));

    // Forward mapping:
    TS_ASSERT_EQUALS(testee.getNumScores(), 5U);
    TS_ASSERT(testee.getScoreByIndex(0, scoreId));
    TS_ASSERT(!testee.getScoreByIndex(5, scoreId));

    // File must still be "safe"
    TS_ASSERT(!testee.hasFutureFeatures());

    // Add a slot
    TS_ASSERT(!testee.getSlot(1000, slot));
    slot2 = testee.addSlot(1000);
    TS_ASSERT(testee.getSlot(1000, slot));
    TS_ASSERT_EQUALS(slot, slot2);
    TS_ASSERT_EQUALS(testee.getNumScores(), 6U);
    TS_ASSERT(testee.getScoreByIndex(5, scoreId));
    TS_ASSERT_EQUALS(scoreId, 1000);

    // Adding existing slot
    TS_ASSERT(testee.getSlot(game::score::ScoreId_BuildPoints, slot));
    slot2 = testee.addSlot(game::score::ScoreId_BuildPoints);
    TS_ASSERT_EQUALS(slot, slot2);
    TS_ASSERT(testee.getSlot(game::score::ScoreId_BuildPoints, slot));
    TS_ASSERT_EQUALS(slot, slot2);

    // Verify the "future" flag
    TS_ASSERT(!testee.hasFutureFeatures());
    testee.setFutureFeatures(true);
    TS_ASSERT(testee.hasFutureFeatures());
    testee.setFutureFeatures(false);
    TS_ASSERT(!testee.hasFutureFeatures());
}

/** Test descriptions. */
void
TestGameScoreTurnScoreList::testDescription()
{
    game::score::TurnScoreList testee;

    // No descriptions by default
    TS_ASSERT(testee.getDescription(game::score::ScoreId_Planets) == 0);
    TS_ASSERT(testee.getDescription(game::score::ScoreId_Capital) == 0);
    TS_ASSERT_EQUALS(testee.getNumDescriptions(), 0U);
    TS_ASSERT(testee.getDescriptionByIndex(0) == 0);

    // Add some
    game::score::TurnScoreList::Description d;
    d.name = "FooScore";
    d.scoreId = game::score::ScoreId_Score;
    TS_ASSERT(testee.addDescription(d));
    TS_ASSERT(!testee.addDescription(d));   // Second add is no change

    // Request it
    const game::score::TurnScoreList::Description* pd = testee.getDescription(game::score::ScoreId_Score);
    TS_ASSERT(pd != 0);
    TS_ASSERT(pd != &d);      // It's copied!
    TS_ASSERT_EQUALS(pd->name, "FooScore");

    // Update
    d.name = "BarScore";
    TS_ASSERT(testee.addDescription(d));
    pd = testee.getDescription(game::score::ScoreId_Score);
    TS_ASSERT(pd != 0);
    TS_ASSERT(pd != &d);      // It's copied!
    TS_ASSERT_EQUALS(pd->name, "BarScore");

    // Test
    TS_ASSERT_EQUALS(testee.getNumDescriptions(), 1U);
    TS_ASSERT(testee.getDescriptionByIndex(0) == pd);
}

/** Test handling of turns. */
void
TestGameScoreTurnScoreList::testTurn()
{
    game::score::TurnScoreList testee;

    // No turns
    for (int i = 1; i < 10; ++i) {
        TS_ASSERT(testee.getTurn(i) == 0);
    }
    TS_ASSERT_EQUALS(testee.getNumTurns(), 0U);
    TS_ASSERT(testee.getTurnByIndex(0) == 0);

    // Add some turns
    testee.addTurn(1, game::Timestamp(2000, 1, 1, 1, 1, 1));
    testee.addTurn(3, game::Timestamp(2000, 3, 1, 1, 1, 1));
    testee.addTurn(5, game::Timestamp(2000, 5, 1, 1, 1, 1));
    testee.addTurn(7, game::Timestamp(2000, 7, 1, 1, 1, 1));

    TS_ASSERT(testee.getTurn(1) != 0);
    TS_ASSERT(testee.getTurn(2) == 0);
    TS_ASSERT(testee.getTurn(3) != 0);
    TS_ASSERT(testee.getTurn(4) == 0);
    TS_ASSERT(testee.getTurn(5) != 0);
    TS_ASSERT(testee.getTurn(6) == 0);
    TS_ASSERT(testee.getTurn(7) != 0);
    TS_ASSERT(testee.getTurn(8) == 0);

    TS_ASSERT_EQUALS(testee.getNumTurns(), 4U);
    TS_ASSERT(testee.getTurnByIndex(0) != 0);
    TS_ASSERT(testee.getTurnByIndex(1) != 0);
    TS_ASSERT(testee.getTurnByIndex(2) != 0);
    TS_ASSERT(testee.getTurnByIndex(3) != 0);
    TS_ASSERT(testee.getTurnByIndex(4) == 0);
    TS_ASSERT_EQUALS(testee.getTurnByIndex(0)->getTurnNumber(), 1);
    TS_ASSERT_EQUALS(testee.getTurnByIndex(1)->getTurnNumber(), 3);
    TS_ASSERT_EQUALS(testee.getTurnByIndex(2)->getTurnNumber(), 5);
    TS_ASSERT_EQUALS(testee.getTurnByIndex(3)->getTurnNumber(), 7);

    // Add some more turns
    testee.addTurn(2, game::Timestamp(2000, 2, 1, 1, 1, 1));
    testee.addTurn(4, game::Timestamp(2000, 4, 1, 1, 1, 1));
    testee.addTurn(6, game::Timestamp(2000, 6, 1, 1, 1, 1));
    testee.addTurn(8, game::Timestamp(2000, 8, 1, 1, 1, 1));

    TS_ASSERT(testee.getTurn(1) != 0);
    TS_ASSERT(testee.getTurn(2) != 0);
    TS_ASSERT(testee.getTurn(3) != 0);
    TS_ASSERT(testee.getTurn(4) != 0);
    TS_ASSERT(testee.getTurn(5) != 0);
    TS_ASSERT(testee.getTurn(6) != 0);
    TS_ASSERT(testee.getTurn(7) != 0);
    TS_ASSERT(testee.getTurn(8) != 0);

    TS_ASSERT_EQUALS(testee.getNumTurns(), 8U);
    TS_ASSERT(testee.getTurnByIndex(0) != 0);
    TS_ASSERT(testee.getTurnByIndex(1) != 0);
    TS_ASSERT(testee.getTurnByIndex(2) != 0);
    TS_ASSERT(testee.getTurnByIndex(3) != 0);
    TS_ASSERT(testee.getTurnByIndex(4) != 0);
    TS_ASSERT(testee.getTurnByIndex(5) != 0);
    TS_ASSERT(testee.getTurnByIndex(6) != 0);
    TS_ASSERT(testee.getTurnByIndex(7) != 0);
    TS_ASSERT(testee.getTurnByIndex(8) == 0);
    TS_ASSERT_EQUALS(testee.getTurnByIndex(0)->getTurnNumber(), 1);
    TS_ASSERT_EQUALS(testee.getTurnByIndex(1)->getTurnNumber(), 2);
    TS_ASSERT_EQUALS(testee.getTurnByIndex(2)->getTurnNumber(), 3);
    TS_ASSERT_EQUALS(testee.getTurnByIndex(3)->getTurnNumber(), 4);
    TS_ASSERT_EQUALS(testee.getTurnByIndex(4)->getTurnNumber(), 5);
    TS_ASSERT_EQUALS(testee.getTurnByIndex(5)->getTurnNumber(), 6);
    TS_ASSERT_EQUALS(testee.getTurnByIndex(6)->getTurnNumber(), 7);
    TS_ASSERT_EQUALS(testee.getTurnByIndex(7)->getTurnNumber(), 8);

    // Set some scores
    {
        game::score::TurnScore& t = testee.addTurn(6, game::Timestamp(2000, 6, 1, 1, 1, 1));
        t.set(1, 1, 100);
        t.set(1, 2, 200);
    }

    // Verify
    {
        game::score::TurnScore& t = testee.addTurn(6, game::Timestamp(2000, 6, 1, 1, 1, 1));
        TS_ASSERT_EQUALS(t.get(1, 1).orElse(-1), 100);
        TS_ASSERT_EQUALS(t.get(1, 2).orElse(-1), 200);
    }

    // Set again with new timestamp. This clears the original data
    {
        game::score::TurnScore& t = testee.addTurn(6, game::Timestamp(2000, 6, 1, 1, 1, 2));
        t.set(1, 3, 300);
    }

    // Verify
    {
        game::score::TurnScore& t = testee.addTurn(6, game::Timestamp(2000, 6, 1, 1, 1, 2));
        TS_ASSERT(!t.get(1, 1).isValid());
        TS_ASSERT(!t.get(1, 2).isValid());
        TS_ASSERT_EQUALS(t.get(1, 3).orElse(-1), 300);
    }
}

/** Test the Description constructor. */
void
TestGameScoreTurnScoreList::testDescriptionConstructor()
{
    const game::score::TurnScoreList::Description d("name", 30, 5, 300);
    TS_ASSERT_EQUALS(d.name, "name");
    TS_ASSERT_EQUALS(d.scoreId, 30);
    TS_ASSERT_EQUALS(d.turnLimit, 5);
    TS_ASSERT_EQUALS(d.winLimit, 300);
}

/** Test addMessageInformation(), complete data. */
void
TestGameScoreTurnScoreList::testAddMessageInformationComplete()
{
    game::score::TurnScoreList testee;

    // Add message information
    gp::MessageInformation mi(gp::MessageInformation::PlayerScore, 300, 42);
    mi.addValue(gp::mi_ScoreTurnLimit, 5);
    mi.addValue(gp::mi_ScoreWinLimit, 1000);
    mi.addScoreValue(3, 400);
    mi.addScoreValue(9, 100);
    mi.addScoreValue(2, 50);
    mi.addValue(gp::ms_Name, "xScore");
    testee.addMessageInformation(mi, game::Timestamp());

    // Verify resulting description
    const game::score::TurnScoreList::Description* desc = testee.getDescription(300);
    TS_ASSERT(desc != 0);
    TS_ASSERT_EQUALS(desc->name, "xScore");
    TS_ASSERT_EQUALS(desc->winLimit, 1000);
    TS_ASSERT_EQUALS(desc->turnLimit, 5);
    TS_ASSERT_EQUALS(desc->scoreId, 300);

    // Verify resulting slot
    game::score::TurnScoreList::Slot_t id;
    TS_ASSERT(testee.getSlot(300, id));

    // Verify resulting score
    const game::score::TurnScore* ts = testee.getTurn(42);
    TS_ASSERT(ts != 0);
    TS_ASSERT_EQUALS(ts->getTurnNumber(), 42);
    TS_ASSERT_EQUALS(ts->get(id, 3).orElse(0), 400);
    TS_ASSERT_EQUALS(ts->get(id, 9).orElse(0), 100);
    TS_ASSERT_EQUALS(ts->get(id, 2).orElse(0),  50);
    TS_ASSERT_EQUALS(ts->get(id, 1).orElse(-1), -1);
}

/** Test addMessageInformation(), just Id given.
    Must take over partial data. */
void
TestGameScoreTurnScoreList::testAddMessageInformationJustId()
{
    game::score::TurnScoreList testee;

    // Define pre-existing score
    game::score::TurnScoreList::Description origDesc;
    origDesc.name = "orig name";
    origDesc.scoreId = 30;
    origDesc.turnLimit = 3;
    origDesc.winLimit = 900;
    testee.addDescription(origDesc);

    // Add message information
    gp::MessageInformation mi(gp::MessageInformation::PlayerScore, 30, 42);
    mi.addValue(gp::mi_ScoreTurnLimit, 5);
    mi.addScoreValue(3, 400);
    testee.addMessageInformation(mi, game::Timestamp());

    // Verify resulting description
    const game::score::TurnScoreList::Description* desc = testee.getDescription(30);
    TS_ASSERT(desc != 0);
    TS_ASSERT_EQUALS(desc->name, "orig name");    // kept
    TS_ASSERT_EQUALS(desc->winLimit, 900);        // kept
    TS_ASSERT_EQUALS(desc->turnLimit, 5);         // overridden
    TS_ASSERT_EQUALS(desc->scoreId, 30);          // kept

    // Verify resulting slot
    game::score::TurnScoreList::Slot_t id;
    TS_ASSERT(testee.getSlot(30, id));

    // Verify resulting score
    const game::score::TurnScore* ts = testee.getTurn(42);
    TS_ASSERT(ts != 0);
    TS_ASSERT_EQUALS(ts->getTurnNumber(), 42);
    TS_ASSERT_EQUALS(ts->get(id, 3).orElse(0), 400);
}

/** Test addMessageInformation(), just name given.
    Must take over partial data. */
void
TestGameScoreTurnScoreList::testAddMessageInformationJustName()
{
    game::score::TurnScoreList testee;

    // Define pre-existing score
    game::score::TurnScoreList::Description origDesc;
    origDesc.name = "name";
    origDesc.scoreId = 777;
    origDesc.turnLimit = 3;
    origDesc.winLimit = 900;
    testee.addDescription(origDesc);

    // Add message information
    gp::MessageInformation mi(gp::MessageInformation::PlayerScore, 0, 42);
    mi.addValue(gp::mi_ScoreWinLimit, 200);
    mi.addValue(gp::ms_Name, "name");
    mi.addScoreValue(3, 400);
    testee.addMessageInformation(mi, game::Timestamp());

    // Verify resulting description
    const game::score::TurnScoreList::Description* desc = testee.getDescription(777);
    TS_ASSERT(desc != 0);
    TS_ASSERT_EQUALS(desc->name, "name");         // kept
    TS_ASSERT_EQUALS(desc->winLimit, 200);        // overridden
    TS_ASSERT_EQUALS(desc->turnLimit, 3);         // kept
    TS_ASSERT_EQUALS(desc->scoreId, 777);         // kept

    // Verify resulting slot
    game::score::TurnScoreList::Slot_t id;
    TS_ASSERT(testee.getSlot(777, id));

    // Verify resulting score
    const game::score::TurnScore* ts = testee.getTurn(42);
    TS_ASSERT(ts != 0);
    TS_ASSERT_EQUALS(ts->getTurnNumber(), 42);
    TS_ASSERT_EQUALS(ts->get(id, 3).orElse(0), 400);
}

/** Test addMessageInformation(), just name given, no pre-existing value.
    Must take over partial data. */
void
TestGameScoreTurnScoreList::testAddMessageInformationJustNameNew()
{
    game::score::TurnScoreList testee;

    // Add message information
    gp::MessageInformation mi(gp::MessageInformation::PlayerScore, 0, 42);
    mi.addValue(gp::mi_ScoreWinLimit, 200);
    mi.addValue(gp::ms_Name, "new name");
    mi.addScoreValue(3, 400);
    testee.addMessageInformation(mi, game::Timestamp());

    // Verify resulting description
    TS_ASSERT(testee.getNumDescriptions() > 0);
    const game::score::TurnScoreList::Description* desc = testee.getDescriptionByIndex(testee.getNumDescriptions()-1);
    TS_ASSERT(desc != 0);
    TS_ASSERT_EQUALS(desc->name, "new name");
    TS_ASSERT_EQUALS(desc->winLimit, 200);
    TS_ASSERT_EQUALS(desc->turnLimit, -1);      // not given, set to default
    TS_ASSERT_DIFFERS(desc->scoreId, 0);
}

