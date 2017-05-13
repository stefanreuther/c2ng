/**
  *  \file u/t_server_file_session.cpp
  *  \brief Test for server::file::Session
  */

#include "server/file/session.hpp"

#include "t_server_file.hpp"

/** Simple test. */
void
TestServerFileSession::testIt()
{
    server::file::Session testee;

    // Test initial state
    TS_ASSERT(testee.isAdmin());
    TS_ASSERT_EQUALS(testee.getUser(), "");

    // Test user
    testee.setUser("1003");
    TS_ASSERT(!testee.isAdmin());
    TS_ASSERT_EQUALS(testee.getUser(), "1003");

    // Reset to admin
    testee.setUser("");
    TS_ASSERT(testee.isAdmin());
    TS_ASSERT_EQUALS(testee.getUser(), "");
}
