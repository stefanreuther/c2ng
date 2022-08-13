/**
  *  \file u/t_server_user_userdata.cpp
  *  \brief Test for server::user::UserData
  */

#include "server/user/userdata.hpp"

#include "t_server_user.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "server/common/numericalidgenerator.hpp"
#include "server/user/classicencrypter.hpp"
#include "server/user/root.hpp"
#include "server/user/user.hpp"

/** Basic functionality test. */
void
TestServerUserUserData::testIt()
{
    // Setup
    server::common::NumericalIdGenerator gen;
    server::user::ClassicEncrypter enc("foo");
    afl::net::redis::InternalDatabase db;
    server::user::Root root(db, gen, enc, server::user::Configuration());
    server::user::UserData testee(root);

    // No data stored
    TS_ASSERT_EQUALS(testee.get("u", "k"), "");

    // Store some data
    TS_ASSERT_THROWS_NOTHING(testee.set("u", "k",  "one"));
    TS_ASSERT_THROWS_NOTHING(testee.set("u", "k2", "two"));

    // Retrieve data
    TS_ASSERT_EQUALS(testee.get("u", "k"),  "one");
    TS_ASSERT_EQUALS(testee.get("u", "k2"), "two");
}

/** Test expiration upon exceeded size. */
void
TestServerUserUserData::testExpire()
{
    // Setup
    server::common::NumericalIdGenerator gen;
    server::user::ClassicEncrypter enc("foo");
    afl::net::redis::InternalDatabase db;
    server::user::Configuration config;
    config.userDataMaxTotalSize = 100;
    server::user::Root root(db, gen, enc, config);
    server::user::UserData testee(root);

    // Set two values. These should take a total of 2*(2*1 + 43) = 90 bytes.
    String_t value(43, 'x');
    testee.set("u", "a", value);
    testee.set("u", "b", value);

    TS_ASSERT_EQUALS(testee.get("u", "a"), value);
    TS_ASSERT_EQUALS(testee.get("u", "b"), value);

    // Set another value. This should expire 'a'
    testee.set("u", "c", value);
    TS_ASSERT_EQUALS(testee.get("u", "a"), "");
    TS_ASSERT_EQUALS(testee.get("u", "b"), value);
    TS_ASSERT_EQUALS(testee.get("u", "c"), value);

    // Set 'b' again, then another value. This should expire 'c'
    testee.set("u", "b", value);
    testee.set("u", "d", value);
    TS_ASSERT_EQUALS(testee.get("u", "a"), "");
    TS_ASSERT_EQUALS(testee.get("u", "b"), value);
    TS_ASSERT_EQUALS(testee.get("u", "c"), "");
    TS_ASSERT_EQUALS(testee.get("u", "d"), value);

    // Set value on another user. This should not affect this one.
    testee.set("v", "a", value);
    TS_ASSERT_EQUALS(testee.get("u", "a"), "");
    TS_ASSERT_EQUALS(testee.get("u", "b"), value);
    TS_ASSERT_EQUALS(testee.get("u", "c"), "");
    TS_ASSERT_EQUALS(testee.get("u", "d"), value);
    TS_ASSERT_EQUALS(testee.get("v", "a"), value);
}

/** Test expiration upon exceeded size. */
void
TestServerUserUserData::testExpire2()
{
    // Setup
    server::common::NumericalIdGenerator gen;
    server::user::ClassicEncrypter enc("foo");
    afl::net::redis::InternalDatabase db;
    server::user::Configuration config;
    config.userDataMaxTotalSize = 100;
    server::user::Root root(db, gen, enc, config);
    server::user::UserData testee(root);

    // Set three values. These should take a total of 2*(2*1 + 28) = 90 bytes.
    String_t value(28, 'x');
    testee.set("u", "a", value);
    testee.set("u", "b", value);
    testee.set("u", "c", value);

    TS_ASSERT_EQUALS(testee.get("u", "a"), value);
    TS_ASSERT_EQUALS(testee.get("u", "b"), value);
    TS_ASSERT_EQUALS(testee.get("u", "c"), value);

    // Set 'b' to empty, add two values. This should expire 'a'.
    testee.set("u", "b", "");
    testee.set("u", "d", value);
    testee.set("u", "e", value);

    TS_ASSERT_EQUALS(testee.get("u", "a"), "");
    TS_ASSERT_EQUALS(testee.get("u", "b"), "");
    TS_ASSERT_EQUALS(testee.get("u", "c"), value);
    TS_ASSERT_EQUALS(testee.get("u", "d"), value);
    TS_ASSERT_EQUALS(testee.get("u", "e"), value);
}

/** Test error cases. */
void
TestServerUserUserData::testError()
{
    // Setup
    server::common::NumericalIdGenerator gen;
    server::user::ClassicEncrypter enc("foo");
    afl::net::redis::InternalDatabase db;
    server::user::Configuration config;
    config.userDataMaxKeySize = 10;
    config.userDataMaxValueSize = 20;
    server::user::Root root(db, gen, enc, config);
    server::user::UserData testee(root);

    // Base case (valid)
    TS_ASSERT_THROWS_NOTHING(testee.set("u", "aaaaaaaaaa", "bbbbbbbbbbbbbbbbbbbb"));

    // Invalid keys
    TS_ASSERT_THROWS(testee.set("u", "", ""), std::exception);
    TS_ASSERT_THROWS(testee.set("u", "\x81", ""), std::exception);
    TS_ASSERT_THROWS(testee.set("u", "\n", ""), std::exception);
    TS_ASSERT_THROWS(testee.set("u", "aaaaaaaaaaa", ""), std::exception);

    // Invalid values
    TS_ASSERT_THROWS(testee.set("u", "a", "xxxxxxxxxxxxxxxxxxxxx"), std::exception);
}

/** Test inconsistent data case.
    The server used to hang if the stored size was much larger than the actual data size,
    because it would fail to free up the amount of space it thinks it can free. */
void
TestServerUserUserData::testExpireInconsistent()
{
    // Setup
    server::common::NumericalIdGenerator gen;
    server::user::ClassicEncrypter enc("foo");
    afl::net::redis::InternalDatabase db;
    server::user::Configuration config;
    config.userDataMaxTotalSize = 100;
    server::user::Root root(db, gen, enc, config);
    server::user::UserData testee(root);

    // Inconsistent status: size is set but data is empty; GC therefore will fail
    const String_t userId = "ui";
    server::user::User(root, userId).userData().intKey("size").set(1000);

    // Set a value, must succeed
    testee.set(userId, "k", "v");
    testee.set(userId, "k2", "v2");
    TS_ASSERT_EQUALS(testee.get(userId, "k"), "v");
    TS_ASSERT_EQUALS(testee.get(userId, "k2"), "v2");
}

