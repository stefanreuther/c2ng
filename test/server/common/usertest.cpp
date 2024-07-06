/**
  *  \file test/server/common/usertest.cpp
  *  \brief Test for server::common::User
  */

#include "server/common/user.hpp"

#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/subtree.hpp"
#include "afl/test/testrunner.hpp"
#include "server/common/root.hpp"

/*
 *  getUserId()
 */

// No real name set
AFL_TEST("server.common.User:getUserId", a)
{
    afl::net::redis::InternalDatabase db;
    server::common::Root root(db);
    server::common::User testee(root, "1001");
    a.checkEqual("getUserId", testee.getUserId(), "1001");
}

/*
 *  getRealName()
 */

// No real name set
AFL_TEST("server.common.User:getRealName:not-set", a)
{
    afl::net::redis::InternalDatabase db;
    server::common::Root root(db);
    server::common::User testee(root, "1001");
    a.checkEqual("getRealName", testee.getRealName(), "");
}

// Real name set, but not enabled
AFL_TEST("server.common.User:getRealName:set-but-disabled", a)
{
    afl::net::redis::InternalDatabase db;
    server::common::Root root(db);
    root.userRoot().subtree("1001").hashKey("profile").stringField("realname").set("RN");
    server::common::User testee(root, "1001");
    a.checkEqual("getRealName", testee.getRealName(), "");
}

// Real name set and enabled
AFL_TEST("server.common.User:getRealName:set-and-enabled", a)
{
    afl::net::redis::InternalDatabase db;
    server::common::Root root(db);
    root.userRoot().subtree("1001").hashKey("profile").stringField("realname").set("RN");
    root.userRoot().subtree("1001").hashKey("profile").intField("inforealnameflag").set(1);
    server::common::User testee(root, "1001");
    a.checkEqual("getRealName", testee.getRealName(), "RN");
}

// Real name set and enabled in default profile
AFL_TEST("server.common.User:getRealName:set-and-enabled-by-default", a)
{
    afl::net::redis::InternalDatabase db;
    server::common::Root root(db);
    root.userRoot().subtree("1001").hashKey("profile").stringField("realname").set("RN");
    root.defaultProfile().intField("inforealnameflag").set(1);
    server::common::User testee(root, "1001");
    a.checkEqual("getRealName", testee.getRealName(), "RN");
}

// Real name set and enabled in default profile, but disabled by user
AFL_TEST("server.common.User:getRealName:default-and-disabled-by-user", a)
{
    afl::net::redis::InternalDatabase db;
    server::common::Root root(db);
    root.userRoot().subtree("1001").hashKey("profile").stringField("realname").set("RN");
    root.userRoot().subtree("1001").hashKey("profile").intField("inforealnameflag").set(0);
    root.defaultProfile().intField("inforealnameflag").set(1);
    server::common::User testee(root, "1001");
    a.checkEqual("getRealName", testee.getRealName(), "");
}
