/**
  *  \file test/game/score/turnscoretest.cpp
  *  \brief Test for game::score::TurnScore
  */

#include "game/score/turnscore.hpp"

#include "afl/test/testrunner.hpp"
#include "game/timestamp.hpp"

/** Simple test. */
AFL_TEST("game.score.TurnScore", a)
{
    game::Timestamp ts(1999, 12, 3, 12, 59, 17);
    game::score::TurnScore testee(99, ts);

    // Initial stat
    a.checkEqual("01. getTurnNumber", testee.getTurnNumber(), 99);
    a.checkEqual("02. getTimestamp", testee.getTimestamp(), ts);
    a.check("03. get", !testee.get(0, 0).isValid());
    a.check("04. get", !testee.get(1, 1).isValid());

    // Set a value
    testee.set(0, 1, 55);
    testee.set(1, 1, 42);
    a.checkEqual("11. get", testee.get(0, 1).orElse(-1), 55);
    a.checkEqual("12. get", testee.get(1, 1).orElse(-1), 42);

    // Test that (1, 1) does not accidentally overlap (0, X).
    a.check("21. get", !testee.get(0, 11).isValid());
    a.check("22. get", !testee.get(0, 12).isValid());
    a.check("23. get", !testee.get(0, 13).isValid());
    a.check("24. get", !testee.get(0, 30).isValid());
    a.check("25. get", !testee.get(0, 31).isValid());
    a.check("26. get", !testee.get(0, 32).isValid());
    a.check("27. get", !testee.get(0, 33).isValid());
    a.check("28. get", !testee.get(0, 34).isValid());

    // We can also make values invalid again
    testee.set(0, 1, afl::base::Nothing);
    a.check("31. get", !testee.get(0, 1).isValid());
    a.checkEqual("32. get", testee.get(1, 1).orElse(-1), 42);

    // Setting out-of-range values does not affect existing values
    testee.set(0, 11, 3);
    testee.set(0, 12, 3);
    testee.set(0, 13, 3);
    testee.set(0, 30, 3);
    testee.set(0, 31, 3);
    testee.set(0, 32, 3);
    testee.set(0, 33, 3);
    testee.set(0, 34, 3);
    a.checkEqual("41. get", testee.get(1, 1).orElse(-1), 42);
}
