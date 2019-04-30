/**
  *  \file u/t_server_user_usertoken.cpp
  *  \brief Test for server::user::UserToken
  */

#include <stdexcept>
#include "server/user/usertoken.hpp"

#include "t_server_user.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "server/common/numericalidgenerator.hpp"
#include "server/user/classicencrypter.hpp"
#include "server/user/root.hpp"

/** Simple functionality test. */
void
TestServerUserUserToken::testIt()
{
    // Environment
    server::common::NumericalIdGenerator gen;
    server::user::ClassicEncrypter enc("foo");
    afl::net::redis::InternalDatabase db;
    server::user::Root root(db, gen, enc, server::user::Configuration());

    // Testee
    server::user::UserToken testee(root);

    // Create a token
    String_t a = testee.getToken("a", "login");
    TS_ASSERT(!a.empty());

    // Requesting another token of the same type must produce the same thing
    String_t b = testee.getToken("a", "login");
    TS_ASSERT_EQUALS(a, b);

    // Requesting a different type must produce a different token
    String_t c = testee.getToken("a", "api");
    TS_ASSERT(!c.empty());
    TS_ASSERT_DIFFERS(a, c);

    // Requesting for a different user must produce a different token
    String_t d = testee.getToken("b", "login");
    TS_ASSERT(!d.empty());
    TS_ASSERT_DIFFERS(a, d);
    TS_ASSERT_DIFFERS(c, d);

    // Retrieve token information
    server::interface::UserToken::Info info = testee.checkToken(a, afl::base::Nothing, false);
    TS_ASSERT_EQUALS(info.userId, "a");
    TS_ASSERT_EQUALS(info.tokenType, "login");
    TS_ASSERT_EQUALS(info.newToken.isValid(), false);

    // Retrieve token with wrong type
    TS_ASSERT_THROWS(testee.checkToken(a, String_t("api"), false), std::exception);

    // Retrieve wrong token
    TS_ASSERT_THROWS(testee.checkToken(a+c+d, afl::base::Nothing, false), std::exception);
}

/** Test token types. */
void
TestServerUserUserToken::testTokenTypes()
{
    // Environment
    server::common::NumericalIdGenerator gen;
    server::user::ClassicEncrypter enc("foo");
    afl::net::redis::InternalDatabase db;
    server::user::Root root(db, gen, enc, server::user::Configuration());

    // Testee
    server::user::UserToken testee(root);

    // Valid types
    String_t a = testee.getToken("x", "login");
    String_t b = testee.getToken("x", "api");
    String_t c = testee.getToken("x", "reset");

    // Invalid
    TS_ASSERT_THROWS(testee.getToken("a", ""), std::exception);
    TS_ASSERT_THROWS(testee.getToken("a", "other"), std::exception);
}

/** Test clearToken(). */
void
TestServerUserUserToken::testClearToken()
{
    // Environment
    server::common::NumericalIdGenerator gen;
    server::user::ClassicEncrypter enc("foo");
    afl::net::redis::InternalDatabase db;
    server::user::Root root(db, gen, enc, server::user::Configuration());

    String_t API[] = { "api" };
    String_t OTHER[] = { "other" };

    // Testee
    server::user::UserToken testee(root);

    // Create some tokens
    String_t a = testee.getToken("x", "login");
    String_t b = testee.getToken("x", "api");
    TS_ASSERT_EQUALS(testee.checkToken(a, afl::base::Nothing, false).userId, "x");
    TS_ASSERT_EQUALS(testee.checkToken(b, afl::base::Nothing, false).userId, "x");

    // Removing other users' tokens does not affect us
    testee.clearToken("y", API);
    TS_ASSERT_EQUALS(testee.checkToken(a, afl::base::Nothing, false).userId, "x");
    TS_ASSERT_EQUALS(testee.checkToken(b, afl::base::Nothing, false).userId, "x");

    // Removing one token does not affect the other
    testee.clearToken("x", API);
    TS_ASSERT_EQUALS(testee.checkToken(a, afl::base::Nothing, false).userId, "x");
    TS_ASSERT_THROWS(testee.checkToken(b, afl::base::Nothing, false), std::exception);

    // We can remove unknown token types
    TS_ASSERT_THROWS_NOTHING(testee.clearToken("x", OTHER));
}

