/**
  *  \file u/t_game_historyturnlist.cpp
  *  \brief Test for game::HistoryTurnList
  */

#include "game/historyturnlist.hpp"

#include "t_game.hpp"

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

