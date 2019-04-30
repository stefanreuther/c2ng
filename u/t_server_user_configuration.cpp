/**
  *  \file u/t_server_user_configuration.cpp
  *  \brief Test for server::user::Configuration
  */

#include "server/user/configuration.hpp"

#include "t_server_user.hpp"

/** Test initialisation (trivial coverage test). */
void
TestServerUserConfiguration::testInitialisation()
{
    server::user::Configuration testee;
    TS_ASSERT(!testee.userKey.empty());
}

