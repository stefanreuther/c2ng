/**
  *  \file test/server/router/configurationtest.cpp
  *  \brief Test for server::router::Configuration
  */

#include "server/router/configuration.hpp"
#include "afl/test/testrunner.hpp"

/** Test that everything is initialized (check with valgrind). */
AFL_TEST("server.router.Configuration:init", a)
{
    server::router::Configuration testee;
    a.check("01", !testee.serverPath.empty());
    a.check("02", testee.normalTimeout > 0);
    a.check("03", testee.virginTimeout > 0);
    a.check("04", testee.maxSessions > 0);
    a.check("05", !testee.newSessionsWin);
}
