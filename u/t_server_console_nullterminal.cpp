/**
  *  \file u/t_server_console_nullterminal.cpp
  *  \brief Test for server::console::NullTerminal
  */

#include "server/console/nullterminal.hpp"

#include "t_server_console.hpp"

/** Simple test. */
void
TestServerConsoleNullTerminal::testIt()
{
    server::console::NullTerminal testee;
    server::console::ContextStack_t stack;
    TS_ASSERT_THROWS_NOTHING(testee.printBanner());
    TS_ASSERT_THROWS_NOTHING(testee.printPrimaryPrompt(stack));
    TS_ASSERT_THROWS_NOTHING(testee.printSecondaryPrompt());
    TS_ASSERT_THROWS_NOTHING(testee.printError("x"));
    TS_ASSERT_THROWS_NOTHING(testee.printResultPrefix());
    TS_ASSERT_THROWS_NOTHING(testee.printResultSuffix());
    TS_ASSERT_THROWS_NOTHING(testee.printMessage("x"));
}
