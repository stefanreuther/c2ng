/**
  *  \file test/game/test/interpreterinterfacetest.cpp
  *  \brief Test for game::test::InterpreterInterface
  */

#include "game/test/interpreterinterface.hpp"

#include "afl/test/testrunner.hpp"

using game::test::InterpreterInterface;

/* Just quick coverage test. */
AFL_TEST("game.test.InterpreterInterface", a)
{
    InterpreterInterface testee;
    a.checkEqual("01. getComment", testee.getComment(InterpreterInterface::Ship,   1), "");
    a.checkEqual("02. getComment", testee.getComment(InterpreterInterface::Planet, 1), "");
    a.checkEqual("03. hasTask",    testee.hasTask(InterpreterInterface::Ship, 1),   false);
    a.checkEqual("04. hasTask",    testee.hasTask(InterpreterInterface::Planet, 1), false);

    a.check("11. getHullShortName",   !testee.getHullShortName(1).isValid());
    a.check("12. getPlayerAdjective", !testee.getPlayerAdjective(1).isValid());
}
