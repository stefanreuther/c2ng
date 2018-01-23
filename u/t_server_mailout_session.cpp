/**
  *  \file u/t_server_mailout_session.cpp
  *  \brief Test for server::mailout::Session
  */

#include "server/mailout/session.hpp"

#include "t_server_mailout.hpp"

/** Simple test.
    Session has no functions; we can only test whether the header file is well-formed. */
void
TestServerMailoutSession::testIt()
{
    server::mailout::Session testee;
}

