/**
  *  \file test/game/historyturnlisttest.cpp
  *  \brief Test for game::HistoryTurnList
  */

#include "game/historyturnlist.hpp"

#include "afl/test/testrunner.hpp"
#include "game/turn.hpp"

/** Basic tests. */
AFL_TEST("game.HistoryTurnList:basics", a)
{
    game::HistoryTurnList testee;

    // Test empty state
    a.checkEqual("01. findNewestUnknownTurnNumber", testee.findNewestUnknownTurnNumber(100), 99);
    a.checkEqual("02. findNewestUnknownTurnNumber", testee.findNewestUnknownTurnNumber(42), 41);
    a.checkEqual("03. findNewestUnknownTurnNumber", testee.findNewestUnknownTurnNumber(11), 10);
    a.checkNull("04. get", testee.get(1));
    a.checkNull("05. get", testee.get(10));
    a.checkNull("06. get", testee.get(42));

    game::HistoryTurn* t = testee.create(10);
    a.checkNonNull("11. create", t);
    a.checkEqual("12. get", testee.get(10), t);

    // Set this turn to known-unavailable. findNewestUnknownTurnNumber will go around that.
    t->setStatus(game::HistoryTurn::Unavailable);
    a.checkEqual("21. findNewestUnknownTurnNumber", testee.findNewestUnknownTurnNumber(100), 99);
    a.checkEqual("22. findNewestUnknownTurnNumber", testee.findNewestUnknownTurnNumber(11), 9);

    // Status access
    a.checkEqual("31. getTurnTimestamp", testee.getTurnTimestamp(1), game::Timestamp());
    a.checkEqual("32. getTurnTimestamp", testee.getTurnTimestamp(10), game::Timestamp());
    a.checkEqual("33. getTurnStatus", testee.getTurnStatus(1), game::HistoryTurn::Unknown);
    a.checkEqual("34. getTurnStatus", testee.getTurnStatus(10), game::HistoryTurn::Unavailable);
}

/** Test findNewestUnknownTurnNumber with gaps in turns. */
AFL_TEST("game.HistoryTurnList:gap", a)
{
    game::HistoryTurnList testee;
    testee.create(10)->setStatus(game::HistoryTurn::WeaklyAvailable);
    testee.create(20)->setStatus(game::HistoryTurn::WeaklyAvailable);
    a.checkEqual("01. findNewestUnknownTurnNumber", testee.findNewestUnknownTurnNumber(100), 99);
    a.checkEqual("02. findNewestUnknownTurnNumber", testee.findNewestUnknownTurnNumber(21), 19);
    a.checkEqual("03. findNewestUnknownTurnNumber", testee.findNewestUnknownTurnNumber(20), 19);
}

/** Test findNewestUnknownTurnNumber with Unknown turns. */
AFL_TEST("game.HistoryTurnList:unknown", a)
{
    game::HistoryTurnList testee;
    testee.create(10);
    testee.create(11);
    testee.create(12);
    a.checkEqual("01. findNewestUnknownTurnNumber", testee.findNewestUnknownTurnNumber(13), 12);
    a.checkEqual("02. findNewestUnknownTurnNumber", testee.findNewestUnknownTurnNumber(14), 13);
}

/** Test initFromTurnScores. */
AFL_TEST("game.HistoryTurnList:initFromTurnScores", a)
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
    a.checkEqual("01. getTurnStatus", testee.getTurnStatus(40), game::HistoryTurn::Loaded);
    a.checkEqual("02. getTurnStatus", testee.getTurnStatus(30), game::HistoryTurn::Unknown);
    a.checkEqual("03. getTurnStatus", testee.getTurnStatus(50), game::HistoryTurn::Unknown);
    a.checkEqual("04. getTurnStatus", testee.getTurnStatus(80), game::HistoryTurn::Unknown);

    // Merge scores
    testee.initFromTurnScores(turnScores, 20, 50);
    a.checkEqual("11. getTurnStatus", testee.getTurnStatus(40), game::HistoryTurn::Loaded);
    a.checkEqual("12. getTurnStatus", testee.getTurnStatus(30), game::HistoryTurn::Unknown);
    a.checkEqual("13. getTurnStatus", testee.getTurnStatus(30), game::HistoryTurn::Unknown);
    a.checkEqual("14. getTurnStatus", testee.getTurnStatus(80), game::HistoryTurn::Unknown);

    a.checkEqual("21. getTurnTimestamp", testee.getTurnTimestamp(30).getDateAsString(), "12-24-1930");
    a.checkEqual("22. getTurnTimestamp", testee.getTurnTimestamp(40).getDateAsString(), "00-00-0000");
    a.checkEqual("23. getTurnTimestamp", testee.getTurnTimestamp(80).getDateAsString(), "00-00-0000");
}
