/**
  *  \file u/t_game_historyturnlist.cpp
  *  \brief Test for game::HistoryTurnList
  */

#include "game/historyturnlist.hpp"

#include "t_game.hpp"
#include "game/turn.hpp"

/** Basic tests. */
void
TestGameHistoryTurnList::testIt()
{
    game::HistoryTurnList testee;

    // Test empty state
    TS_ASSERT_EQUALS(testee.findNewestUnknownTurnNumber(100), 99);
    TS_ASSERT_EQUALS(testee.findNewestUnknownTurnNumber(42), 41);
    TS_ASSERT_EQUALS(testee.findNewestUnknownTurnNumber(11), 10);
    TS_ASSERT(testee.get(1) == 0);
    TS_ASSERT(testee.get(10) == 0);
    TS_ASSERT(testee.get(42) == 0);

    game::HistoryTurn* t = testee.create(10);
    TS_ASSERT(t != 0);
    TS_ASSERT_EQUALS(testee.get(10), t);

    // Set this turn to known-unavailable. findNewestUnknownTurnNumber will go around that.
    t->setStatus(game::HistoryTurn::Unavailable);
    TS_ASSERT_EQUALS(testee.findNewestUnknownTurnNumber(100), 99);
    TS_ASSERT_EQUALS(testee.findNewestUnknownTurnNumber(11), 9);

    // Status access
    TS_ASSERT_EQUALS(testee.getTurnTimestamp(1), game::Timestamp());
    TS_ASSERT_EQUALS(testee.getTurnTimestamp(10), game::Timestamp());
    TS_ASSERT_EQUALS(testee.getTurnStatus(1), game::HistoryTurn::Unknown);
    TS_ASSERT_EQUALS(testee.getTurnStatus(10), game::HistoryTurn::Unavailable);
}

/** Test findNewestUnknownTurnNumber with gaps in turns. */
void
TestGameHistoryTurnList::testGap()
{
    game::HistoryTurnList testee;
    testee.create(10)->setStatus(game::HistoryTurn::WeaklyAvailable);
    testee.create(20)->setStatus(game::HistoryTurn::WeaklyAvailable);
    TS_ASSERT_EQUALS(testee.findNewestUnknownTurnNumber(100), 99);
    TS_ASSERT_EQUALS(testee.findNewestUnknownTurnNumber(21), 19);
    TS_ASSERT_EQUALS(testee.findNewestUnknownTurnNumber(20), 19);
}

/** Test findNewestUnknownTurnNumber with Unknown turns. */
void
TestGameHistoryTurnList::testUnknown()
{
    game::HistoryTurnList testee;
    testee.create(10);
    testee.create(11);
    testee.create(12);
    TS_ASSERT_EQUALS(testee.findNewestUnknownTurnNumber(13), 12);
    TS_ASSERT_EQUALS(testee.findNewestUnknownTurnNumber(14), 13);
}

/** Test initFromTurnScores. */
void
TestGameHistoryTurnList::testInitFromTurnScores()
{
    // Scores
    game::score::TurnScoreList turnScores;
    for (int i = 1; i < 99; ++i) {
        uint8_t data[18] = {'1','2','-','2','4','-','1','9',uint8_t('0'+i/10),uint8_t('0'+i%10),'2','0',':','1','5',':','3','1'};
        turnScores.addTurn(i, game::Timestamp(data));
    }

    // HistoryTurnList
    game::HistoryTurnList testee;
    testee.create(40)->handleLoadSucceeded(*new game::Turn());
    TS_ASSERT_EQUALS(testee.getTurnStatus(40), game::HistoryTurn::Loaded);
    TS_ASSERT_EQUALS(testee.getTurnStatus(30), game::HistoryTurn::Unknown);
    TS_ASSERT_EQUALS(testee.getTurnStatus(50), game::HistoryTurn::Unknown);
    TS_ASSERT_EQUALS(testee.getTurnStatus(80), game::HistoryTurn::Unknown);

    // Merge scores
    testee.initFromTurnScores(turnScores, 20, 50);
    TS_ASSERT_EQUALS(testee.getTurnStatus(40), game::HistoryTurn::Loaded);
    TS_ASSERT_EQUALS(testee.getTurnStatus(30), game::HistoryTurn::Unknown);
    TS_ASSERT_EQUALS(testee.getTurnStatus(30), game::HistoryTurn::Unknown);
    TS_ASSERT_EQUALS(testee.getTurnStatus(80), game::HistoryTurn::Unknown);

    TS_ASSERT_EQUALS(testee.getTurnTimestamp(30).getDateAsString(), "12-24-1930");
    TS_ASSERT_EQUALS(testee.getTurnTimestamp(40).getDateAsString(), "00-00-0000");
    TS_ASSERT_EQUALS(testee.getTurnTimestamp(80).getDateAsString(), "00-00-0000");
}

