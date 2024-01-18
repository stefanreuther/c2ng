/**
  *  \file test/server/console/colorterminaltest.cpp
  *  \brief Test for server::console::ColorTerminal
  */

#include "server/console/colorterminal.hpp"

#include "afl/test/testrunner.hpp"
#include "server/test/terminalverifier.hpp"

/** Simple test. */
AFL_TEST("server.console.ColorTerminal", a)
{
    server::test::verifyInteractiveTerminal<server::console::ColorTerminal>(a);
}
