/**
  *  \file u/t_server_user_usertoken.cpp
  *  \brief Test for server::user::UserToken
  */

#include <stdexcept>
#include "server/user/usertoken.hpp"

#include "t_server_user.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/string/format.hpp"
#include "server/common/numericalidgenerator.hpp"
#include "server/common/randomidgenerator.hpp"
#include "server/user/classicencrypter.hpp"
#include "server/user/root.hpp"
#include "server/user/token.hpp"
#include "server/user/user.hpp"

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

/** Test that we can create many tokens.
    This test case used to hang someday. */
void
TestServerUserUserToken::testMany()
{
    afl::io::NullFileSystem fs;
    server::common::RandomIdGenerator gen(fs);
    server::user::ClassicEncrypter enc("foo");
    afl::net::redis::InternalDatabase db;
    server::user::Root root(db, gen, enc, server::user::Configuration());

    // Testee
    server::user::UserToken testee(root);

    // Create 1000 tokens
    for (size_t i = 0; i < 1000; ++i) {
        String_t user = afl::string::Format("%d", i);
        testee.getToken(user, "login");
    }
}

/** Test token renewal.
    If a user repeatedly uses an old token, only a single new token must be created. */
void
TestServerUserUserToken::testTokenRenewal()
{
    afl::io::NullFileSystem fs;
    server::common::RandomIdGenerator gen(fs);
    server::user::ClassicEncrypter enc("foo");
    afl::net::redis::InternalDatabase db;
    server::user::Root root(db, gen, enc, server::user::Configuration());

    // Manually create a single token that is about to expire
    const server::Time_t now = root.getTime();
    const String_t oldToken = "oooooooo";
    const String_t userId = "1002";
    const String_t tokenType = "login";
    root.allTokens().add(oldToken);
    root.tokenById(oldToken).userId().set(userId);
    root.tokenById(oldToken).tokenType().set(tokenType);
    root.tokenById(oldToken).validUntil().set(now + 24*60);   // expires tomorrow
    server::user::User(root, userId).tokensByType(tokenType).add(oldToken);

    // Verify using old token
    server::user::UserToken testee(root);
    server::interface::UserToken::Info info = testee.checkToken(oldToken, tokenType, true);
    TS_ASSERT_EQUALS(info.userId, userId);
    TS_ASSERT_EQUALS(info.tokenType, tokenType);
    TS_ASSERT_EQUALS(info.userId, userId);
    TS_ASSERT_DIFFERS(info.newToken.orElse(""), "");

    // Verify again using same old token - should report the same new token
    server::interface::UserToken::Info info2 = testee.checkToken(oldToken, tokenType, true);
    TS_ASSERT_EQUALS(info2.userId, userId);
    TS_ASSERT_EQUALS(info2.tokenType, tokenType);
    TS_ASSERT_EQUALS(info2.userId, userId);
    TS_ASSERT_DIFFERS(info2.newToken.orElse(""), "");
    TS_ASSERT_EQUALS(info2.newToken.orElse(""), info.newToken.orElse(""));

    // Old token must still exist, it's not yet expired
    TS_ASSERT(root.allTokens().contains(oldToken));
}

/** Test use of expired token.
    Access must be refused, token deleted. */
void
TestServerUserUserToken::testTokenExpired()
{
    afl::io::NullFileSystem fs;
    server::common::RandomIdGenerator gen(fs);
    server::user::ClassicEncrypter enc("foo");
    afl::net::redis::InternalDatabase db;
    server::user::Root root(db, gen, enc, server::user::Configuration());

    // Manually create a single token that is expired
    const server::Time_t now = root.getTime();
    const String_t oldToken = "oooooooo";
    const String_t userId = "1002";
    const String_t tokenType = "login";
    root.allTokens().add(oldToken);
    root.tokenById(oldToken).userId().set(userId);
    root.tokenById(oldToken).tokenType().set(tokenType);
    root.tokenById(oldToken).validUntil().set(now - 1);
    server::user::User(root, userId).tokensByType(tokenType).add(oldToken);

    // Verify using old token
    server::user::UserToken testee(root);
    TS_ASSERT_THROWS(testee.checkToken(oldToken, tokenType, true), std::runtime_error);

    // Token must be gone
    TS_ASSERT(!root.allTokens().contains(oldToken));

    // Still fails
    TS_ASSERT_THROWS(testee.checkToken(oldToken, tokenType, true), std::runtime_error);
}

/** Test retrieval of expired token.
    Expired token must be removed, new one created. */
void
TestServerUserUserToken::testTokenExpiredCreate()
{
    afl::io::NullFileSystem fs;
    server::common::RandomIdGenerator gen(fs);
    server::user::ClassicEncrypter enc("foo");
    afl::net::redis::InternalDatabase db;
    server::user::Root root(db, gen, enc, server::user::Configuration());

    // Manually create a single token that is expired
    const server::Time_t now = root.getTime();
    const String_t oldToken = "oooooooo";
    const String_t userId = "1002";
    const String_t tokenType = "login";
    root.allTokens().add(oldToken);
    root.tokenById(oldToken).userId().set(userId);
    root.tokenById(oldToken).tokenType().set(tokenType);
    root.tokenById(oldToken).validUntil().set(now - 1);
    server::user::User(root, userId).tokensByType(tokenType).add(oldToken);

    // Verify using old token
    server::user::UserToken testee(root);
    String_t newToken = testee.getToken(userId, tokenType);

    // Must be a new token
    TS_ASSERT_DIFFERS(oldToken, newToken);

    // Old token must be gone
    TS_ASSERT(!root.allTokens().contains(oldToken));
    TS_ASSERT(root.allTokens().contains(newToken));

    // New one can be reproduced
    TS_ASSERT_EQUALS(newToken, testee.getToken(userId, tokenType));
}

