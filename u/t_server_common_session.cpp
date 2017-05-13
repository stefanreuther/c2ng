/**
  *  \file u/t_server_common_session.cpp
  *  \brief Test for server::common::Session
  */

#include "server/common/session.hpp"

#include "t_server_common.hpp"

/** Simple test. */
void
TestServerCommonSession::testIt()
{
    server::common::Session testee;

    // Test initial state
    TS_ASSERT(testee.isAdmin());
    TS_ASSERT_EQUALS(testee.getUser(), "");
    TS_ASSERT_THROWS(testee.checkUser(), std::exception);
    TS_ASSERT_THROWS_NOTHING(testee.checkAdmin());

    // Test user
    testee.setUser("1003");
    TS_ASSERT(!testee.isAdmin());
    TS_ASSERT_EQUALS(testee.getUser(), "1003");
    TS_ASSERT_THROWS_NOTHING(testee.checkUser());
    TS_ASSERT_THROWS(testee.checkAdmin(), std::exception);

    // Reset to admin
    testee.setUser("");
    TS_ASSERT(testee.isAdmin());
    TS_ASSERT_EQUALS(testee.getUser(), "");
    TS_ASSERT_THROWS(testee.checkUser(), std::exception);
    TS_ASSERT_THROWS_NOTHING(testee.checkAdmin());
}
