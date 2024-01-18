/**
  *  \file test/server/mailout/sessiontest.cpp
  *  \brief Test for server::mailout::Session
  */

#include "server/mailout/session.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test.
    Session has no functions; we can only test whether the header file is well-formed. */
AFL_TEST_NOARG("server.mailout.Session")
{
    server::mailout::Session testee;
}
