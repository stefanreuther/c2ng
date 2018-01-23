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

/** Test formatWord(). */
void
TestServerCommonSession::testFormatWord()
{
    using server::common::Session;

    // Empty
    TS_ASSERT_EQUALS(Session::formatWord("", false), "''");
    TS_ASSERT_EQUALS(Session::formatWord("", true), "''");

    // Placeholder trigger
    // - spaces
    TS_ASSERT_EQUALS(Session::formatWord(" ", false), "...");
    // - special characters
    TS_ASSERT_EQUALS(Session::formatWord("[foo]", false), "...");
    TS_ASSERT_EQUALS(Session::formatWord("a\nb", false), "...");
    TS_ASSERT_EQUALS(Session::formatWord("''", false), "...");
    // - too long
    TS_ASSERT_EQUALS(Session::formatWord(String_t(200, 'x'), false), "...");

    // Censor
    TS_ASSERT_EQUALS(Session::formatWord("x", true), "...");

    // Normal: we want to pass...
    // - normal words
    TS_ASSERT_EQUALS(Session::formatWord("x", false), "x");
    TS_ASSERT_EQUALS(Session::formatWord("x_y", false), "x_y");
    // - file names
    TS_ASSERT_EQUALS(Session::formatWord("a/b/c.dat", false), "a/b/c.dat");
    // - permission strings
    TS_ASSERT_EQUALS(Session::formatWord("g:1,g:2", false), "g:1,g:2");
    TS_ASSERT_EQUALS(Session::formatWord("-all", false), "-all");
    // - wildcards
    TS_ASSERT_EQUALS(Session::formatWord("xy*", false), "xy*");
}

