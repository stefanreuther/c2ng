/**
  *  \file test/server/host/configurationtest.cpp
  *  \brief Test for server::host::Configuration
  */

#include "server/host/configuration.hpp"
#include "afl/test/testrunner.hpp"

/** Simple tests. */
AFL_TEST("server.host.Configuration:init", a)
{
    // Verify defaults
    server::host::Configuration testee;
    a.checkEqual("01. timeScale",              testee.timeScale, 60);
    a.checkEqual("02. workDirectory",          testee.workDirectory, "");
    a.checkEqual("03. useCron",                testee.useCron, true);
    a.checkEqual("04. hostFileAddress",        testee.hostFileAddress.toString(), "127.0.0.1:7776");
    a.checkEqual("05. usersSeeTemporaryTurns", testee.usersSeeTemporaryTurns, true);
    a.checkEqual("06. maxStoredKeys",          testee.maxStoredKeys, 10);

    // Must be copyable
    server::host::Configuration t = testee;
    a.checkEqual("11. copied timeScale", t.timeScale, 60);
}

/** Test getUserTimeFromTime(). */
AFL_TEST("server.host.Configuration:getUserTimeFromTime", a)
{
    server::host::Configuration testee;

    // Default is 60 which is just passed through
    testee.timeScale = 60;
    a.checkEqual("timeScale=60", testee.getUserTimeFromTime(5000), 5000);

    // If system runs at second scale, we must scale down for user time which expects minutes.
    testee.timeScale = 1;
    a.checkEqual("timeScale=1", testee.getUserTimeFromTime(1200), 20);

    // For completeness, if we're running too slow, we must scale up
    testee.timeScale = 100;
    a.checkEqual("timeScale=100", testee.getUserTimeFromTime(3000), 5000);
}
