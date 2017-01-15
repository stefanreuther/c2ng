/**
  *  \file u/t_game_historyturn.cpp
  *  \brief Test for game::HistoryTurn
  */

#include "game/historyturn.hpp"

#include "t_game.hpp"
#include "game/turn.hpp"

/** Test getters/setters. */
void
TestGameHistoryTurn::testSet()
{
    // Initial state
    game::HistoryTurn testee(99);
    TS_ASSERT_EQUALS(testee.getTurnNumber(), 99);
    TS_ASSERT_EQUALS(testee.getTimestamp(), game::Timestamp());
    TS_ASSERT_EQUALS(testee.getStatus(), game::HistoryTurn::Unknown);
    TS_ASSERT(testee.getTurn().get() == 0);

    // Timestamp
    const char data[18] = {'1','2','-','2','4','-','1','9','8','8','2','0',':','1','5',':','3','1'};
    testee.setTimestamp(game::Timestamp(data));
    TS_ASSERT_EQUALS(testee.getTimestamp(), game::Timestamp(data));

    // Status
    testee.setStatus(game::HistoryTurn::Failed);
    TS_ASSERT_EQUALS(testee.getStatus(), game::HistoryTurn::Failed);
}

/** Test success cases. */
void
TestGameHistoryTurn::testSuccess()
{
    // A turn
    const int NR = 42;
    afl::base::Ref<game::Turn> t = *new game::Turn();
    t->setTurnNumber(NR);

    // Direct load
    {
        game::HistoryTurn testee(NR);
        TS_ASSERT_EQUALS(testee.getStatus(), game::HistoryTurn::Unknown);
        TS_ASSERT(testee.isLoadable());
        testee.handleLoadSucceeded(t);
        TS_ASSERT_EQUALS(testee.getStatus(), game::HistoryTurn::Loaded);
    }

    // Load from WeaklyAvailable
    {
        game::HistoryTurn testee(NR);
        TS_ASSERT_EQUALS(testee.getStatus(), game::HistoryTurn::Unknown);
        testee.setStatus(game::HistoryTurn::WeaklyAvailable);
        TS_ASSERT(testee.isLoadable());
        testee.handleLoadSucceeded(t);
        TS_ASSERT_EQUALS(testee.getStatus(), game::HistoryTurn::Loaded);
    }

    // Load from StronglyAvailable
    {
        game::HistoryTurn testee(NR);
        TS_ASSERT_EQUALS(testee.getStatus(), game::HistoryTurn::Unknown);
        testee.setStatus(game::HistoryTurn::StronglyAvailable);
        TS_ASSERT(testee.isLoadable());
        testee.handleLoadSucceeded(t);
        TS_ASSERT_EQUALS(testee.getStatus(), game::HistoryTurn::Loaded);
    }
    
    // Load from Unavailable
    {
        game::HistoryTurn testee(NR);
        TS_ASSERT_EQUALS(testee.getStatus(), game::HistoryTurn::Unknown);
        testee.setStatus(game::HistoryTurn::Unavailable);
        TS_ASSERT(!testee.isLoadable());
        testee.handleLoadSucceeded(t);

        // Load has no effect!
        TS_ASSERT_EQUALS(testee.getStatus(), game::HistoryTurn::Unavailable);
    }
}

void
TestGameHistoryTurn::testFail()
{
    const int NR = 23;

    // Direct fail -> Unavailable (no violated promise)
    {
        game::HistoryTurn testee(NR);
        TS_ASSERT_EQUALS(testee.getStatus(), game::HistoryTurn::Unknown);
        TS_ASSERT(testee.isLoadable());
        testee.handleLoadFailed();
        TS_ASSERT_EQUALS(testee.getStatus(), game::HistoryTurn::Unavailable);
    }

    // Fail from WeaklyAvailable -> Unavailable (just a weak promise violated)
    {
        game::HistoryTurn testee(NR);
        TS_ASSERT_EQUALS(testee.getStatus(), game::HistoryTurn::Unknown);
        testee.setStatus(game::HistoryTurn::WeaklyAvailable);
        TS_ASSERT(testee.isLoadable());
        testee.handleLoadFailed();
        TS_ASSERT_EQUALS(testee.getStatus(), game::HistoryTurn::Unavailable);
    }

    // Fail from StronglyAvailable -> Failed (promise violated)
    {
        game::HistoryTurn testee(NR);
        TS_ASSERT_EQUALS(testee.getStatus(), game::HistoryTurn::Unknown);
        testee.setStatus(game::HistoryTurn::StronglyAvailable);
        TS_ASSERT(testee.isLoadable());
        testee.handleLoadFailed();
        TS_ASSERT_EQUALS(testee.getStatus(), game::HistoryTurn::Failed);
    }
    
    // Load from Unavailable
    {
        game::HistoryTurn testee(NR);
        TS_ASSERT_EQUALS(testee.getStatus(), game::HistoryTurn::Unknown);
        testee.setStatus(game::HistoryTurn::Unavailable);
        TS_ASSERT(!testee.isLoadable());
        testee.handleLoadFailed();
        TS_ASSERT_EQUALS(testee.getStatus(), game::HistoryTurn::Unavailable);
    }
}

