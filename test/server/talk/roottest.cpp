/**
  *  \file test/server/talk/roottest.cpp
  *  \brief Test for server::talk::Root
  */

#include "server/talk/root.hpp"

#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/test/testrunner.hpp"
#include <memory>

/** Test checkUserPermission(). */
AFL_TEST("server.talk.Root:checkUserPermission", a)
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
    a.check("01",  testee.checkUserPermission("all", "1003"));
    a.check("02", !testee.checkUserPermission("-all", "1003"));

    a.check("11",  testee.checkUserPermission("p:defProfile1", "1003"));
    a.check("12", !testee.checkUserPermission("-p:defProfile1", "1003"));
    a.check("13", !testee.checkUserPermission("p:defProfile0", "1003"));
    a.check("14", !testee.checkUserPermission("-p:defProfile0", "1003"));

    a.check("21",  testee.checkUserPermission("p:userProfile1", "1003"));
    a.check("22", !testee.checkUserPermission("-p:userProfile1", "1003"));
    a.check("23", !testee.checkUserPermission("p:userProfile0", "1003"));
    a.check("24", !testee.checkUserPermission("-p:userProfile0", "1003"));

    a.check("31",  testee.checkUserPermission("p:bothProfile1", "1003"));
    a.check("32", !testee.checkUserPermission("-p:bothProfile1", "1003"));
    a.check("33", !testee.checkUserPermission("p:bothProfile0", "1003"));
    a.check("34", !testee.checkUserPermission("-p:bothProfile0", "1003"));

    a.check("41",  testee.checkUserPermission("g:42", "1003"));
    a.check("42", !testee.checkUserPermission("-g:42", "1003"));
    a.check("43",  testee.checkUserPermission("g:42", "1004"));
    a.check("44", !testee.checkUserPermission("-g:42", "1004"));
    a.check("45", !testee.checkUserPermission("g:44", "1003"));
    a.check("46", !testee.checkUserPermission("-g:44", "1003"));

    a.check("51",  testee.checkUserPermission("u:1003", "1003"));
    a.check("52", !testee.checkUserPermission("u:1003", "1004"));
    a.check("53", !testee.checkUserPermission("-u:1003", "1003"));
    a.check("54", !testee.checkUserPermission("-u:1003", "1004"));

    // Combinations
    // - first hit decides
    a.check("61", !testee.checkUserPermission("-all,all", "1003"));
    a.check("62", !testee.checkUserPermission("-p:defProfile1,all", "1003"));
    a.check("63", !testee.checkUserPermission("-all,u:1003", "1003"));
    a.check("64", !testee.checkUserPermission("-all,u:1003", "1004"));
    a.check("65",  testee.checkUserPermission("u:1003,-all", "1003"));

    // - first is mismatch, second decides
    a.check("71",  testee.checkUserPermission("-p:defProfile0,all", "1003"));
    a.check("72",  testee.checkUserPermission("-p:bothProfile0,all", "1003"));
    a.check("73", !testee.checkUserPermission("u:1003,-all", "1004"));
    a.check("74",  testee.checkUserPermission("u:1003,all", "1004"));

    // - no match
    a.check("81", !testee.checkUserPermission("p:userProfile0,p:bothProfile0", "1003"));

    // Undefined is skipped
    a.check("91", !testee.checkUserPermission("whatever", "1003"));
    a.check("92",  testee.checkUserPermission("-p:defProfile0,whatever,all", "1003"));

    // Border cases
    a.check("101", !testee.checkUserPermission("", "1003"));
    a.check("102", !testee.checkUserPermission("-", "1003"));
}

/** Test getUserIdFromLogin(). */
AFL_TEST("server.talk.Root:getUserIdFromLogin", a)
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

    a.checkEqual("01", testee.getUserIdFromLogin(""), "");
    a.checkEqual("02", testee.getUserIdFromLogin("0"), "");
    a.checkEqual("03", testee.getUserIdFromLogin("1001"), "");
    a.checkEqual("04", testee.getUserIdFromLogin("admin"), "");
    a.checkEqual("05", testee.getUserIdFromLogin("ADMIN"), "");
    a.checkEqual("06", testee.getUserIdFromLogin("Admin"), "");
    a.checkEqual("07", testee.getUserIdFromLogin("ab"), "");

    a.checkEqual("11", testee.getUserIdFromLogin("foo"), "1001");
    a.checkEqual("12", testee.getUserIdFromLogin("FOO"), "1001");
    a.checkEqual("13", testee.getUserIdFromLogin("-foo-"), "1001");

    a.checkEqual("21", testee.getUserIdFromLogin("a-b"), "1002");
    a.checkEqual("22", testee.getUserIdFromLogin("a_b"), "1002");
}
