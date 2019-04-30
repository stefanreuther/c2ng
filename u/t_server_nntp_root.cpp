/**
  *  \file u/t_server_nntp_root.cpp
  *  \brief Test for server::nntp::Root
  */

#include "server/nntp/root.hpp"

#include "t_server_nntp.hpp"
#include "afl/net/nullcommandhandler.hpp"

/** Simple test. */
void
TestServerNntpRoot::testIt()
{
    afl::net::NullCommandHandler nch;
    server::nntp::Root testee(nch, nch, "http://huh");

    // Simple methods
    TS_ASSERT_THROWS_NOTHING(testee.log().write(afl::sys::Log::Info, "channel", "msg"));
    TS_ASSERT_THROWS_NOTHING(testee.configureReconnect());

    // Allocate ID
    uint32_t a = testee.allocateId();
    uint32_t b = testee.allocateId();
    TS_ASSERT_DIFFERS(a, b);

    // Constructor parameter accessors
    TS_ASSERT_EQUALS(&nch, &testee.talk());
    TS_ASSERT_EQUALS(testee.getBaseUrl(), "http://huh");
}

