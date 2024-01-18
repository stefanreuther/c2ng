/**
  *  \file test/server/user/usertokentest.cpp
  *  \brief Test for server::user::UserToken
  */

#include "server/user/usertoken.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/string/format.hpp"
#include "afl/test/testrunner.hpp"
#include "server/common/numericalidgenerator.hpp"
#include "server/common/randomidgenerator.hpp"
#include "server/user/classicencrypter.hpp"
#include "server/user/root.hpp"
#include "server/user/token.hpp"
#include "server/user/user.hpp"
#include <stdexcept>

/** Simple functionality test. */
AFL_TEST("server.user.UserToken:basics", a)
{
    // Environment
    server::common::NumericalIdGenerator gen;
    server::user::ClassicEncrypter enc("foo");
    afl::net::redis::InternalDatabase db;
    server::user::Root root(db, gen, enc, server::user::Configuration());

    // Testee
    server::user::UserToken testee(root);

    // Create a token
    String_t ta = testee.getToken("a", "login");
    a.check("01. getToken", !ta.empty());

    // Requesting another token of the same type must produce the same thing
    String_t tb = testee.getToken("a", "login");
    a.checkEqual("11. getToken", ta, tb);

    // Requesting a different type must produce a different token
    String_t tc = testee.getToken("a", "api");
    a.check("21. getToken", !tc.empty());
    a.checkDifferent("22", ta, tc);

    // Requesting for a different user must produce a different token
    String_t td = testee.getToken("b", "login");
    a.check("31. getToken", !td.empty());
    a.checkDifferent("32", ta, td);
    a.checkDifferent("33", tc, td);

    // Retrieve token information
    server::interface::UserToken::Info info = testee.checkToken(ta, afl::base::Nothing, false);
    a.checkEqual("41. userId",    info.userId, "a");
    a.checkEqual("42. tokenType", info.tokenType, "login");
    a.checkEqual("43. newToken",  info.newToken.isValid(), false);

    // Retrieve token with wrong type
    AFL_CHECK_THROWS(a("51. wrong type"), testee.checkToken(ta, String_t("api"), false), std::exception);

    // Retrieve wrong token
    AFL_CHECK_THROWS(a("61. wrong token"), testee.checkToken(ta+tc+td, afl::base::Nothing, false), std::exception);
}

/** Test token types. */
AFL_TEST("server.user.UserToken:token-types", a)
{
    // Environment
    server::common::NumericalIdGenerator gen;
    server::user::ClassicEncrypter enc("foo");
    afl::net::redis::InternalDatabase db;
    server::user::Root root(db, gen, enc, server::user::Configuration());

    // Testee
    server::user::UserToken testee(root);

    // Valid types
    String_t ta = testee.getToken("x", "login");
    String_t tb = testee.getToken("x", "api");
    String_t tc = testee.getToken("x", "reset");

    // Invalid
    AFL_CHECK_THROWS(a("01. wrong type"), testee.getToken("a", ""), std::exception);
    AFL_CHECK_THROWS(a("02. wrong type"), testee.getToken("a", "other"), std::exception);
}

/** Test clearToken(). */
AFL_TEST("server.user.UserToken:clearToken", a)
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
    String_t ta = testee.getToken("x", "login");
    String_t tb = testee.getToken("x", "api");
    a.checkEqual("01. checkToken", testee.checkToken(ta, afl::base::Nothing, false).userId, "x");
    a.checkEqual("02. checkToken", testee.checkToken(tb, afl::base::Nothing, false).userId, "x");

    // Removing other users' tokens does not affect us
    testee.clearToken("y", API);
    a.checkEqual("11. checkToken", testee.checkToken(ta, afl::base::Nothing, false).userId, "x");
    a.checkEqual("12. checkToken", testee.checkToken(tb, afl::base::Nothing, false).userId, "x");

    // Removing one token does not affect the other
    testee.clearToken("x", API);
    a.checkEqual("21. checkToken", testee.checkToken(ta, afl::base::Nothing, false).userId, "x");
    AFL_CHECK_THROWS(a("22. checkToken"), testee.checkToken(tb, afl::base::Nothing, false), std::exception);

    // We can remove unknown token types
    AFL_CHECK_SUCCEEDS(a("31. clearToken"), testee.clearToken("x", OTHER));
}

/** Test that we can create many tokens.
    This test case used to hang someday. */
AFL_TEST_NOARG("server.user.UserToken:many")
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
AFL_TEST("server.user.UserToken:token-renewal", a)
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
    a.checkEqual("01. userId", info.userId, userId);
    a.checkEqual("02. tokenType", info.tokenType, tokenType);
    a.checkEqual("03. userId", info.userId, userId);
    a.checkDifferent("04. newToken", info.newToken.orElse(""), "");

    // Verify again using same old token - should report the same new token
    server::interface::UserToken::Info info2 = testee.checkToken(oldToken, tokenType, true);
    a.checkEqual("11. userId", info2.userId, userId);
    a.checkEqual("12. tokenType", info2.tokenType, tokenType);
    a.checkEqual("13. userId", info2.userId, userId);
    a.checkDifferent("14. newToken", info2.newToken.orElse(""), "");
    a.checkEqual("15. newToken", info2.newToken.orElse(""), info.newToken.orElse(""));

    // Old token must still exist, it's not yet expired
    a.check("21. allTokens", root.allTokens().contains(oldToken));
}

/** Test use of expired token.
    Access must be refused, token deleted. */
AFL_TEST("server.user.UserToken:expired-token", a)
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
    AFL_CHECK_THROWS(a("01. checkToken"), testee.checkToken(oldToken, tokenType, true), std::runtime_error);

    // Token must be gone
    a.check("11. allTokens", !root.allTokens().contains(oldToken));

    // Still fails
    AFL_CHECK_THROWS(a("21. checkToken"), testee.checkToken(oldToken, tokenType, true), std::runtime_error);
}

/** Test retrieval of expired token.
    Expired token must be removed, new one created. */
AFL_TEST("server.user.UserToken:expired-token:create", a)
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
    a.checkDifferent("01. new token", oldToken, newToken);

    // Old token must be gone
    a.check("11. allTokens", !root.allTokens().contains(oldToken));
    a.check("12. allTokens", root.allTokens().contains(newToken));

    // New one can be reproduced
    a.checkEqual("21. newToken", newToken, testee.getToken(userId, tokenType));
}

/** Test token access with a broken token.
    If token creation crashes midway, it may leave us with a token listed for the user, but not in allTokens().
    getToken() must not return such a token, because checkToken() would refuse it. */
AFL_TEST("server.user.UserToken:missing-token", a)
{
    afl::io::NullFileSystem fs;
    server::common::RandomIdGenerator gen(fs);
    server::user::ClassicEncrypter enc("foo");
    afl::net::redis::InternalDatabase db;
    server::user::Root root(db, gen, enc, server::user::Configuration());

    // Manually create a single token that has plenty time remaining, but is not listed in allTokens()
    const server::Time_t now = root.getTime();
    const String_t oldToken = "t";
    const String_t userId = "1002";
    const String_t tokenType = "login";
    root.tokenById(oldToken).userId().set(userId);
    root.tokenById(oldToken).tokenType().set(tokenType);
    root.tokenById(oldToken).validUntil().set(now + 1000000);
    server::user::User(root, userId).tokensByType(tokenType).add(oldToken);

    // Retrieve token
    server::user::UserToken testee(root);
    String_t token = testee.getToken(userId, tokenType);

    // Retrieved token must be usable for checking
    // Do not verify the identity of the token; implementation is free to create a new one or fix the broken one.
    server::interface::UserToken::Info info = testee.checkToken(token, tokenType, false);
    a.checkEqual("01. userId", info.userId, userId);
    a.checkEqual("02. tokenType", info.tokenType, tokenType);
}
