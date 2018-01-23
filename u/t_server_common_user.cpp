/**
  *  \file u/t_server_common_user.cpp
  *  \brief Test for server::common::User
  */

#include "server/common/user.hpp"

#include "t_server_common.hpp"
#include "server/common/root.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/subtree.hpp"
#include "afl/net/redis/hashkey.hpp"

/** Test getRealName(). */
void
TestServerCommonUser::testRealName()
{
    // No real name set
    {
        afl::net::redis::InternalDatabase db;
        server::common::Root root(db);
        server::common::User testee(root, "1001");
        TS_ASSERT_EQUALS(testee.getRealName(), "");
    }

    // Real name set, but not enabled
    {
        afl::net::redis::InternalDatabase db;
        server::common::Root root(db);
        root.userRoot().subtree("1001").hashKey("profile").stringField("realname").set("RN");
        server::common::User testee(root, "1001");
        TS_ASSERT_EQUALS(testee.getRealName(), "");
    }

    // Real name set and enabled
    {
        afl::net::redis::InternalDatabase db;
        server::common::Root root(db);
        root.userRoot().subtree("1001").hashKey("profile").stringField("realname").set("RN");
        root.userRoot().subtree("1001").hashKey("profile").intField("inforealnameflag").set(1);
        server::common::User testee(root, "1001");
        TS_ASSERT_EQUALS(testee.getRealName(), "RN");
    }

    // Real name set and enabled in default profile
    {
        afl::net::redis::InternalDatabase db;
        server::common::Root root(db);
        root.userRoot().subtree("1001").hashKey("profile").stringField("realname").set("RN");
        root.defaultProfile().intField("inforealnameflag").set(1);
        server::common::User testee(root, "1001");
        TS_ASSERT_EQUALS(testee.getRealName(), "RN");
    }

    // Real name set and enabled in default profile, but disabled by user
    {
        afl::net::redis::InternalDatabase db;
        server::common::Root root(db);
        root.userRoot().subtree("1001").hashKey("profile").stringField("realname").set("RN");
        root.userRoot().subtree("1001").hashKey("profile").intField("inforealnameflag").set(0);
        root.defaultProfile().intField("inforealnameflag").set(1);
        server::common::User testee(root, "1001");
        TS_ASSERT_EQUALS(testee.getRealName(), "");
    }
}

