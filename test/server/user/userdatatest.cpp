/**
  *  \file test/server/user/userdatatest.cpp
  *  \brief Test for server::user::UserData
  */

#include "server/user/userdata.hpp"

#include "afl/net/redis/internaldatabase.hpp"
#include "afl/test/testrunner.hpp"
#include "server/common/numericalidgenerator.hpp"
#include "server/user/classicencrypter.hpp"
#include "server/user/root.hpp"
#include "server/user/user.hpp"

/** Basic functionality test. */
AFL_TEST("server.user.UserData:basics", a)
{
    // Setup
    server::common::NumericalIdGenerator gen;
    server::user::ClassicEncrypter enc("foo");
    afl::net::redis::InternalDatabase db;
    server::user::Root root(db, gen, enc, server::user::Configuration());
    server::user::UserData testee(root);

    // No data stored
    a.checkEqual("01. get", testee.get("u", "k"), "");

    // Store some data
    AFL_CHECK_SUCCEEDS(a("11. set"), testee.set("u", "k",  "one"));
    AFL_CHECK_SUCCEEDS(a("12. set"), testee.set("u", "k2", "two"));

    // Retrieve data
    a.checkEqual("21. get", testee.get("u", "k"),  "one");
    a.checkEqual("22. get", testee.get("u", "k2"), "two");
}

/** Test expiration upon exceeded size. */
AFL_TEST("server.user.UserData:expire", a)
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

    a.checkEqual("01. get", testee.get("u", "a"), value);
    a.checkEqual("02. get", testee.get("u", "b"), value);

    // Set another value. This should expire 'a'
    testee.set("u", "c", value);
    a.checkEqual("11. get", testee.get("u", "a"), "");
    a.checkEqual("12. get", testee.get("u", "b"), value);
    a.checkEqual("13. get", testee.get("u", "c"), value);

    // Set 'b' again, then another value. This should expire 'c'
    testee.set("u", "b", value);
    testee.set("u", "d", value);
    a.checkEqual("21. get", testee.get("u", "a"), "");
    a.checkEqual("22. get", testee.get("u", "b"), value);
    a.checkEqual("23. get", testee.get("u", "c"), "");
    a.checkEqual("24. get", testee.get("u", "d"), value);

    // Set value on another user. This should not affect this one.
    testee.set("v", "a", value);
    a.checkEqual("31. get", testee.get("u", "a"), "");
    a.checkEqual("32. get", testee.get("u", "b"), value);
    a.checkEqual("33. get", testee.get("u", "c"), "");
    a.checkEqual("34. get", testee.get("u", "d"), value);
    a.checkEqual("35. get", testee.get("v", "a"), value);
}

/** Test expiration upon exceeded size. */
AFL_TEST("server.user.UserData:expire:2", a)
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

    a.checkEqual("01. get", testee.get("u", "a"), value);
    a.checkEqual("02. get", testee.get("u", "b"), value);
    a.checkEqual("03. get", testee.get("u", "c"), value);

    // Set 'b' to empty, add two values. This should expire 'a'.
    testee.set("u", "b", "");
    testee.set("u", "d", value);
    testee.set("u", "e", value);

    a.checkEqual("11. get", testee.get("u", "a"), "");
    a.checkEqual("12. get", testee.get("u", "b"), "");
    a.checkEqual("13. get", testee.get("u", "c"), value);
    a.checkEqual("14. get", testee.get("u", "d"), value);
    a.checkEqual("15. get", testee.get("u", "e"), value);
}

/** Test error cases. */
AFL_TEST("server.user.UserData:error", a)
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
    AFL_CHECK_SUCCEEDS(a("01. set"), testee.set("u", "aaaaaaaaaa", "bbbbbbbbbbbbbbbbbbbb"));

    // Invalid keys
    AFL_CHECK_THROWS(a("11. no key"), testee.set("u", "", ""), std::exception);
    AFL_CHECK_THROWS(a("12. bad key"), testee.set("u", "\x81", ""), std::exception);
    AFL_CHECK_THROWS(a("13. bad key"), testee.set("u", "\n", ""), std::exception);
    AFL_CHECK_THROWS(a("14. key too long"), testee.set("u", "aaaaaaaaaaa", ""), std::exception);

    // Invalid values
    AFL_CHECK_THROWS(a("21. value too long"), testee.set("u", "a", "xxxxxxxxxxxxxxxxxxxxx"), std::exception);
}

/** Test inconsistent data case.
    The server used to hang if the stored size was much larger than the actual data size,
    because it would fail to free up the amount of space it thinks it can free. */
AFL_TEST("server.user.UserData:expire:inconsistent", a)
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
    a.checkEqual("01. get", testee.get(userId, "k"), "v");
    a.checkEqual("02. get", testee.get(userId, "k2"), "v2");
}
