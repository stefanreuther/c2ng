/**
  *  \file u/t_server_router_configuration.cpp
  *  \brief Test for server::router::Configuration
  */

#include "server/router/configuration.hpp"

#include "t_server_router.hpp"

/** Test that everything is initialized (check with valgrind). */
void
TestServerRouterConfiguration::testInit()
{
    server::router::Configuration testee;
    TS_ASSERT(!testee.serverPath.empty());
    TS_ASSERT(testee.normalTimeout > 0);
    TS_ASSERT(testee.virginTimeout > 0);
    TS_ASSERT(testee.maxSessions > 0);
    TS_ASSERT(!testee.newSessionsWin);
}
