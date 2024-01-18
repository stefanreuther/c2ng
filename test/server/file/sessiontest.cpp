/**
  *  \file test/server/file/sessiontest.cpp
  *  \brief Test for server::file::Session
  */

#include "server/file/session.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test. */
AFL_TEST("server.file.Session", a)
{
    server::file::Session testee;

    // Test initial state
    a.check("01. isAdmin", testee.isAdmin());
    a.checkEqual("02. getUser", testee.getUser(), "");

    // Test user
    testee.setUser("1003");
    a.check("11. isAdmin", !testee.isAdmin());
    a.checkEqual("12. getUser", testee.getUser(), "1003");

    // Reset to admin
    testee.setUser("");
    a.check("21. isAdmin", testee.isAdmin());
    a.checkEqual("22. getUser", testee.getUser(), "");
}
