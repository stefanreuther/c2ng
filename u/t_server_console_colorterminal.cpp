/**
  *  \file u/t_server_console_colorterminal.cpp
  *  \brief Test for server::console::ColorTerminal
  */

#include "server/console/colorterminal.hpp"

#include "t_server_console.hpp"
#include "t_server_console_terminal.hpp"

/** Simple test. */
void
TestServerConsoleColorTerminal::testIt()
{
    verifyInteractiveTerminal<server::console::ColorTerminal>("ColorTerminal");
}
