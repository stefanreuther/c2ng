/**
  *  \file u/t_server_ports.cpp
  *  \brief Test for server::Ports
  */

#include "server/ports.hpp"

#include "t_server.hpp"
#include "afl/base/staticassert.hpp"

/** The main objective of this test is to verify that the header file is self-contained. */
void
TestServerPorts::testIt()
{
    static_assert(server::ROUTER_PORT != 0, "ROUTER_PORT");
}

