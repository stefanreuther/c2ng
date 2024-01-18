/**
  *  \file test/server/console/nullterminaltest.cpp
  *  \brief Test for server::console::NullTerminal
  */

#include "server/console/nullterminal.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test. */
AFL_TEST("server.console.NullTerminal", a)
{
    server::console::NullTerminal testee;
    server::console::ContextStack_t stack;
    AFL_CHECK_SUCCEEDS(a("01. printBanner"),          testee.printBanner());
    AFL_CHECK_SUCCEEDS(a("02. printPrimaryPrompt"),   testee.printPrimaryPrompt(stack));
    AFL_CHECK_SUCCEEDS(a("03. printSecondaryPrompt"), testee.printSecondaryPrompt());
    AFL_CHECK_SUCCEEDS(a("04. printError"),           testee.printError("x"));
    AFL_CHECK_SUCCEEDS(a("05. printResultPrefix"),    testee.printResultPrefix());
    AFL_CHECK_SUCCEEDS(a("06. printResultSuffix"),    testee.printResultSuffix());
    AFL_CHECK_SUCCEEDS(a("07. printMessage"),         testee.printMessage("x"));
}
