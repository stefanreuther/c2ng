/**
  *  \file test/server/nntp/roottest.cpp
  *  \brief Test for server::nntp::Root
  */

#include "server/nntp/root.hpp"

#include "afl/net/nullcommandhandler.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test. */
AFL_TEST("server.nntp.Root", a)
{
    afl::net::NullCommandHandler nch;
    server::nntp::Root testee(nch, nch, "http://huh");

    // Simple methods
    AFL_CHECK_SUCCEEDS(a("01. log"), testee.log().write(afl::sys::Log::Info, "channel", "msg"));
    AFL_CHECK_SUCCEEDS(a("02. configureReconnect"), testee.configureReconnect());

    // Allocate ID
    uint32_t aa = testee.allocateId();
    uint32_t bb = testee.allocateId();
    a.checkDifferent("11. allocateId", aa, bb);

    // Constructor parameter accessors
    a.checkEqual("21. talk", &nch, &testee.talk());
    a.checkEqual("22. getBaseUrl", testee.getBaseUrl(), "http://huh");
}
