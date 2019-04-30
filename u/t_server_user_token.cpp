/**
  *  \file u/t_server_user_token.cpp
  *  \brief Test for server::user::Token
  */

#include "server/user/token.hpp"

#include "t_server_user.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/hashkey.hpp"

/** Trivial functionality/syntax test. */
void
TestServerUserToken::testIt()
{
    afl::net::redis::InternalDatabase db;
    afl::net::redis::HashKey k(db, "x");

    // Construction
    server::user::Token testee(k);

    // Access
    testee.userId().set("a");
    testee.tokenType().set("t");
    testee.validUntil().set(3);
    TS_ASSERT(k.exists());

    testee.remove();
    TS_ASSERT(!k.exists());
}

