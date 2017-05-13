/**
  *  \file u/t_game_score_turnscore.cpp
  *  \brief Test for game::score::TurnScore
  */

#include "game/score/turnscore.hpp"

#include "t_game_score.hpp"
#include "game/timestamp.hpp"

/** Simple test. */
void
TestGameScoreTurnScore::testIt()
{
    game::Timestamp ts(1999, 12, 3, 12, 59, 17);
    game::score::TurnScore testee(99, ts);

    // Initial stat
    TS_ASSERT_EQUALS(testee.getTurnNumber(), 99);
    TS_ASSERT_EQUALS(testee.getTimestamp(), ts);
    TS_ASSERT(!testee.get(0, 0).isValid());
    TS_ASSERT(!testee.get(1, 1).isValid());

    // Set a value
    testee.set(0, 1, 55);
    testee.set(1, 1, 42);
    TS_ASSERT_EQUALS(testee.get(0, 1).orElse(-1), 55);
    TS_ASSERT_EQUALS(testee.get(1, 1).orElse(-1), 42);

    // Test that (1, 1) does not accidentally overlap (0, X).
    TS_ASSERT(!testee.get(0, 11).isValid());
    TS_ASSERT(!testee.get(0, 12).isValid());
    TS_ASSERT(!testee.get(0, 13).isValid());
    TS_ASSERT(!testee.get(0, 30).isValid());
    TS_ASSERT(!testee.get(0, 31).isValid());
    TS_ASSERT(!testee.get(0, 32).isValid());
    TS_ASSERT(!testee.get(0, 33).isValid());
    TS_ASSERT(!testee.get(0, 34).isValid());

    // We can also make values invalid again
    testee.set(0, 1, afl::base::Nothing);
    TS_ASSERT(!testee.get(0, 1).isValid());
    TS_ASSERT_EQUALS(testee.get(1, 1).orElse(-1), 42);

    // Setting out-of-range values does not affect existing values
    testee.set(0, 11, 3);
    testee.set(0, 12, 3);
    testee.set(0, 13, 3);
    testee.set(0, 30, 3);
    testee.set(0, 31, 3);
    testee.set(0, 32, 3);
    testee.set(0, 33, 3);
    testee.set(0, 34, 3);
    TS_ASSERT_EQUALS(testee.get(1, 1).orElse(-1), 42);
}
