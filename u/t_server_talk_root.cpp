/**
  *  \file u/t_server_talk_root.cpp
  *  \brief Test for server::talk::Root
  */

#include <memory>
#include "server/talk/root.hpp"

#include "t_server_talk.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/nullcommandhandler.hpp"

/** Test checkUserPermission(). */
void
TestServerTalkRoot::testCheckUserPermission()
{
    // Create a preloaded internal database
    using afl::data::Segment;
    afl::net::redis::InternalDatabase db;
    db.callVoid(Segment().pushBackString("hset").pushBackString("default:profile").pushBackString("defProfile1").pushBackString("1"));
    db.callVoid(Segment().pushBackString("hset").pushBackString("default:profile").pushBackString("defProfile0").pushBackString("0"));
    db.callVoid(Segment().pushBackString("hset").pushBackString("default:profile").pushBackString("bothProfile1").pushBackString("0")); // note reversed value
    db.callVoid(Segment().pushBackString("hset").pushBackString("default:profile").pushBackString("bothProfile0").pushBackString("1")); // note reversed value
    db.callVoid(Segment().pushBackString("hset").pushBackString("user:1003:profile").pushBackString("userProfile1").pushBackString("1"));
    db.callVoid(Segment().pushBackString("hset").pushBackString("user:1003:profile").pushBackString("userProfile0").pushBackString("0"));
    db.callVoid(Segment().pushBackString("hset").pushBackString("user:1003:profile").pushBackString("bothProfile1").pushBackString("1"));
    db.callVoid(Segment().pushBackString("hset").pushBackString("user:1003:profile").pushBackString("bothProfile0").pushBackString("0"));
    db.callVoid(Segment().pushBackString("hset").pushBackString("game:42:users").pushBackString("1003").pushBackString("0"));
    db.callVoid(Segment().pushBackString("hset").pushBackString("game:42:users").pushBackString("1004").pushBackString("1"));

    // Test
    afl::net::NullCommandHandler null;
    server::talk::Root testee(db, null, server::talk::Configuration());
    TS_ASSERT( testee.checkUserPermission("all", "1003"));
    TS_ASSERT(!testee.checkUserPermission("-all", "1003"));

    TS_ASSERT( testee.checkUserPermission("p:defProfile1", "1003"));
    TS_ASSERT(!testee.checkUserPermission("-p:defProfile1", "1003"));
    TS_ASSERT(!testee.checkUserPermission("p:defProfile0", "1003"));
    TS_ASSERT(!testee.checkUserPermission("-p:defProfile0", "1003"));

    TS_ASSERT( testee.checkUserPermission("p:userProfile1", "1003"));
    TS_ASSERT(!testee.checkUserPermission("-p:userProfile1", "1003"));
    TS_ASSERT(!testee.checkUserPermission("p:userProfile0", "1003"));
    TS_ASSERT(!testee.checkUserPermission("-p:userProfile0", "1003"));

    TS_ASSERT( testee.checkUserPermission("p:bothProfile1", "1003"));
    TS_ASSERT(!testee.checkUserPermission("-p:bothProfile1", "1003"));
    TS_ASSERT(!testee.checkUserPermission("p:bothProfile0", "1003"));
    TS_ASSERT(!testee.checkUserPermission("-p:bothProfile0", "1003"));

    TS_ASSERT( testee.checkUserPermission("g:42", "1003"));
    TS_ASSERT(!testee.checkUserPermission("-g:42", "1003"));
    TS_ASSERT( testee.checkUserPermission("g:42", "1004"));
    TS_ASSERT(!testee.checkUserPermission("-g:42", "1004"));
    TS_ASSERT(!testee.checkUserPermission("g:44", "1003"));
    TS_ASSERT(!testee.checkUserPermission("-g:44", "1003"));

    // Combinations
    // - first hit decides
    TS_ASSERT(!testee.checkUserPermission("-all,all", "1003"));
    TS_ASSERT(!testee.checkUserPermission("-p:defProfile1,all", "1003"));

    // - first is mismatch, second decides
    TS_ASSERT( testee.checkUserPermission("-p:defProfile0,all", "1003"));
    TS_ASSERT( testee.checkUserPermission("-p:bothProfile0,all", "1003"));

    // - no match
    TS_ASSERT(!testee.checkUserPermission("p:userProfile0,p:bothProfile0", "1003"));

    // Undefined is skipped
    TS_ASSERT(!testee.checkUserPermission("whatever", "1003"));
    TS_ASSERT( testee.checkUserPermission("-p:defProfile0,whatever,all", "1003"));

    // Border cases
    TS_ASSERT(!testee.checkUserPermission("", "1003"));
    TS_ASSERT(!testee.checkUserPermission("-", "1003"));
}

/** Test getUserIdFromLogin(). */
void
TestServerTalkRoot::testGetUserIdFromLogin()
{
    // Create a preloaded internal database
    using afl::data::Segment;
    afl::net::redis::InternalDatabase db;
    db.callVoid(Segment().pushBackString("set").pushBackString("uid:admin").pushBackString("0"));     // blocked account
    db.callVoid(Segment().pushBackString("set").pushBackString("uid:foo").pushBackString("1001"));    // regular account
    db.callVoid(Segment().pushBackString("set").pushBackString("user:1001:name").pushBackString("foo"));
    db.callVoid(Segment().pushBackString("set").pushBackString("uid:a_b").pushBackString("1002"));    // regular account
    db.callVoid(Segment().pushBackString("set").pushBackString("user:1002:name").pushBackString("a_b"));

    // Test
    afl::net::NullCommandHandler null;
    server::talk::Root testee(db, null, server::talk::Configuration());

    TS_ASSERT_EQUALS(testee.getUserIdFromLogin(""), "");
    TS_ASSERT_EQUALS(testee.getUserIdFromLogin("0"), "");
    TS_ASSERT_EQUALS(testee.getUserIdFromLogin("1001"), "");
    TS_ASSERT_EQUALS(testee.getUserIdFromLogin("admin"), "");
    TS_ASSERT_EQUALS(testee.getUserIdFromLogin("ADMIN"), "");
    TS_ASSERT_EQUALS(testee.getUserIdFromLogin("Admin"), "");
    TS_ASSERT_EQUALS(testee.getUserIdFromLogin("ab"), "");

    TS_ASSERT_EQUALS(testee.getUserIdFromLogin("foo"), "1001");
    TS_ASSERT_EQUALS(testee.getUserIdFromLogin("FOO"), "1001");
    TS_ASSERT_EQUALS(testee.getUserIdFromLogin("-foo-"), "1001");

    TS_ASSERT_EQUALS(testee.getUserIdFromLogin("a-b"), "1002");
    TS_ASSERT_EQUALS(testee.getUserIdFromLogin("a_b"), "1002");
}
