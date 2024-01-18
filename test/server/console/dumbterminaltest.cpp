/**
  *  \file test/server/console/dumbterminaltest.cpp
  *  \brief Test for server::console::DumbTerminal
  */

#include "server/console/dumbterminal.hpp"

#include "afl/test/testrunner.hpp"
#include "server/test/terminalverifier.hpp"

/** Simple test. */
AFL_TEST("server.console.DumbTerminal", a)
{
    server::test::verifyInteractiveTerminal<server::console::DumbTerminal>(a);
}
