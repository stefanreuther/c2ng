/**
  *  \file test/server/portstest.cpp
  *  \brief Test for server::Ports
  */

#include "server/ports.hpp"

#include "afl/base/staticassert.hpp"
#include "afl/test/testrunner.hpp"

/** The main objective of this test is to verify that the header file is self-contained. */
AFL_TEST_NOARG("server.Ports")
{
    static_assert(server::ROUTER_PORT != 0, "ROUTER_PORT");
}
