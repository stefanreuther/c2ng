/**
  *  \file test/server/user/tokentest.cpp
  *  \brief Test for server::user::Token
  */

#include "server/user/token.hpp"

#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/test/testrunner.hpp"

/** Trivial functionality/syntax test. */
AFL_TEST("server.user.Token", a)
{
    afl::net::redis::InternalDatabase db;
    afl::net::redis::HashKey k(db, "x");

    // Construction
    server::user::Token testee(k);

    // Access
    testee.userId().set("a");
    testee.tokenType().set("t");
    testee.validUntil().set(3);
    a.check("01. exists", k.exists());

    testee.remove();
    a.check("11. exists", !k.exists());
}
