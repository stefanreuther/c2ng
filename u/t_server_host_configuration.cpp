/**
  *  \file u/t_server_host_configuration.cpp
  *  \brief Test for server::host::Configuration
  */

#include "server/host/configuration.hpp"

#include "t_server_host.hpp"

/** Simple tests. */
void
TestServerHostConfiguration::testBase()
{
    // Verify defaults
    server::host::Configuration testee;
    TS_ASSERT_EQUALS(testee.timeScale, 60);
    TS_ASSERT_EQUALS(testee.workDirectory, "");
    TS_ASSERT_EQUALS(testee.useCron, true);
    TS_ASSERT_EQUALS(testee.hostFileAddress.toString(), "127.0.0.1:7776");
    TS_ASSERT_EQUALS(testee.usersSeeTemporaryTurns, true);
    TS_ASSERT_EQUALS(testee.maxStoredKeys, 10);

    // Must be copyable
    server::host::Configuration t = testee;
    TS_ASSERT_EQUALS(t.timeScale, 60);
}

/** Test getUserTimeFromTime(). */
void
TestServerHostConfiguration::testTime()
{
    server::host::Configuration testee;

    // Default is 60 which is just passed through
    testee.timeScale = 60;
    TS_ASSERT_EQUALS(testee.getUserTimeFromTime(5000), 5000);

    // If system runs at second scale, we must scale down for user time which expects minutes.
    testee.timeScale = 1;
    TS_ASSERT_EQUALS(testee.getUserTimeFromTime(1200), 20);

    // For completeness, if we're running too slow, we must scale up
    testee.timeScale = 100;
    TS_ASSERT_EQUALS(testee.getUserTimeFromTime(3000), 5000);
}
