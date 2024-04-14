/**
  *  \file test/game/test/countertest.cpp
  *  \brief Test for game::test::Counter
  */

#include "game/test/counter.hpp"

#include "afl/base/signal.hpp"
#include "afl/test/testrunner.hpp"

using game::test::Counter;

AFL_TEST("game.test.Counter", a)
{
    Counter testee;
    a.checkEqual("01", testee.get(), 0);

    // The main reason for this class is to be used as signal target, so test that.
    afl::base::Signal<void()> sig;
    sig.add(&testee, &Counter::increment);
    sig.raise();

    a.checkEqual("02", testee.get(), 1);
}
