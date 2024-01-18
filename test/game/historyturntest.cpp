/**
  *  \file test/game/historyturntest.cpp
  *  \brief Test for game::HistoryTurn
  */

#include "game/historyturn.hpp"

#include "afl/test/testrunner.hpp"
#include "game/turn.hpp"

/** Test getters/setters. */
AFL_TEST("game.HistoryTurn:basics", a)
{
    // Initial state
    game::HistoryTurn testee(99);
    a.checkEqual("01. getTurnNumber", testee.getTurnNumber(), 99);
    a.checkEqual("02. getTimestamp", testee.getTimestamp(), game::Timestamp());
    a.checkEqual("03. getStatus", testee.getStatus(), game::HistoryTurn::Unknown);
    a.checkNull("04. getTurn", testee.getTurn().get());

    // Timestamp
    const uint8_t data[18] = {'1','2','-','2','4','-','1','9','8','8','2','0',':','1','5',':','3','1'};
    testee.setTimestamp(game::Timestamp(data));
    a.checkEqual("11. getTimestamp", testee.getTimestamp(), game::Timestamp(data));

    // Status
    testee.setStatus(game::HistoryTurn::Failed);
    a.checkEqual("21. getStatus", testee.getStatus(), game::HistoryTurn::Failed);
}

/** Test success cases. */
AFL_TEST("game.HistoryTurn:handleLoadSucceeded", a)
{
    // A turn
    const int NR = 42;
    afl::base::Ref<game::Turn> t = *new game::Turn();
    t->setTurnNumber(NR);

    // Direct load
    {
        game::HistoryTurn testee(NR);
        a.checkEqual("01. getStatus", testee.getStatus(), game::HistoryTurn::Unknown);
        a.check("02. isLoadable", testee.isLoadable());
        testee.handleLoadSucceeded(t);
        a.checkEqual("03. getStatus", testee.getStatus(), game::HistoryTurn::Loaded);
    }

    // Load from WeaklyAvailable
    {
        game::HistoryTurn testee(NR);
        a.checkEqual("11. getStatus", testee.getStatus(), game::HistoryTurn::Unknown);
        testee.setStatus(game::HistoryTurn::WeaklyAvailable);
        a.check("12. isLoadable", testee.isLoadable());
        testee.handleLoadSucceeded(t);
        a.checkEqual("13. getStatus", testee.getStatus(), game::HistoryTurn::Loaded);
    }

    // Load from StronglyAvailable
    {
        game::HistoryTurn testee(NR);
        a.checkEqual("21. getStatus", testee.getStatus(), game::HistoryTurn::Unknown);
        testee.setStatus(game::HistoryTurn::StronglyAvailable);
        a.check("22. isLoadable", testee.isLoadable());
        testee.handleLoadSucceeded(t);
        a.checkEqual("23. getStatus", testee.getStatus(), game::HistoryTurn::Loaded);
    }

    // Load from Unavailable
    {
        game::HistoryTurn testee(NR);
        a.checkEqual("31. getStatus", testee.getStatus(), game::HistoryTurn::Unknown);
        testee.setStatus(game::HistoryTurn::Unavailable);
        a.check("32. isLoadable", !testee.isLoadable());
        testee.handleLoadSucceeded(t);

        // Load has no effect!
        a.checkEqual("41. getStatus", testee.getStatus(), game::HistoryTurn::Unavailable);
    }
}

AFL_TEST("game.HistoryTurn:handleLoadFailed", a)
{
    const int NR = 23;

    // Direct fail -> Unavailable (no violated promise)
    {
        game::HistoryTurn testee(NR);
        a.checkEqual("01. getStatus", testee.getStatus(), game::HistoryTurn::Unknown);
        a.check("02. isLoadable", testee.isLoadable());
        testee.handleLoadFailed();
        a.checkEqual("03. getStatus", testee.getStatus(), game::HistoryTurn::Unavailable);
    }

    // Fail from WeaklyAvailable -> Unavailable (just a weak promise violated)
    {
        game::HistoryTurn testee(NR);
        a.checkEqual("11. getStatus", testee.getStatus(), game::HistoryTurn::Unknown);
        testee.setStatus(game::HistoryTurn::WeaklyAvailable);
        a.check("12. isLoadable", testee.isLoadable());
        testee.handleLoadFailed();
        a.checkEqual("13. getStatus", testee.getStatus(), game::HistoryTurn::Unavailable);
    }

    // Fail from StronglyAvailable -> Failed (promise violated)
    {
        game::HistoryTurn testee(NR);
        a.checkEqual("21. getStatus", testee.getStatus(), game::HistoryTurn::Unknown);
        testee.setStatus(game::HistoryTurn::StronglyAvailable);
        a.check("22. isLoadable", testee.isLoadable());
        testee.handleLoadFailed();
        a.checkEqual("23. getStatus", testee.getStatus(), game::HistoryTurn::Failed);
    }

    // Load from Unavailable
    {
        game::HistoryTurn testee(NR);
        a.checkEqual("31. getStatus", testee.getStatus(), game::HistoryTurn::Unknown);
        testee.setStatus(game::HistoryTurn::Unavailable);
        a.check("32. isLoadable", !testee.isLoadable());
        testee.handleLoadFailed();
        a.checkEqual("33. getStatus", testee.getStatus(), game::HistoryTurn::Unavailable);
    }
}
