/**
  *  \file u/t_server_console_dumbterminal.cpp
  *  \brief Test for server::console::DumbTerminal
  */

#include "server/console/dumbterminal.hpp"

#include "t_server_console.hpp"
#include "server/test/terminalverifier.hpp"

/** Simple test. */
void
TestServerConsoleDumbTerminal::testIt()
{
    server::test::verifyInteractiveTerminal<server::console::DumbTerminal>("DumbTerminal");
}
