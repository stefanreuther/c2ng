/**
  *  \file u/t_server_console_colorterminal.cpp
  *  \brief Test for server::console::ColorTerminal
  */

#include "server/console/colorterminal.hpp"

#include "t_server_console.hpp"
#include "server/test/terminalverifier.hpp"

/** Simple test. */
void
TestServerConsoleColorTerminal::testIt()
{
    server::test::verifyInteractiveTerminal<server::console::ColorTerminal>("ColorTerminal");
}
