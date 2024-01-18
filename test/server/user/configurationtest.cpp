/**
  *  \file test/server/user/configurationtest.cpp
  *  \brief Test for server::user::Configuration
  */

#include "server/user/configuration.hpp"
#include "afl/test/testrunner.hpp"

/** Test initialisation (trivial coverage test). */
AFL_TEST("server.user.Configuration", a)
{
    server::user::Configuration testee;
    a.check("01", !testee.userKey.empty());
}
