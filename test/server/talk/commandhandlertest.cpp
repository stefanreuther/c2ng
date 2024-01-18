/**
  *  \file test/server/talk/commandhandlertest.cpp
  *  \brief Test for server::talk::CommandHandler
  */

#include "server/talk/commandhandler.hpp"

#include "afl/data/access.hpp"
#include "afl/data/segment.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/test/testrunner.hpp"
#include "server/talk/root.hpp"
#include "server/talk/session.hpp"

using afl::data::Segment;
using afl::data::Access;

/** Simple test.
    Call once into every child element to make sure command routing works. */
AFL_TEST("server.talk.CommandHandler", a)
{
    // Infrastructure
    afl::net::redis::InternalDatabase db;
    afl::net::NullCommandHandler mq;
    server::talk::Root root(db, mq, server::talk::Configuration());
    server::talk::Session session;

    // Preload
    root.keywordTable().add("KEYWORD", "Info");
    root.groupRoot().subtree("g").hashKey("header").stringField("name").set("gn");

    // Testee
    server::talk::CommandHandler testee(root, session);

    // - Basic commands
    a.checkEqual("01. ping", testee.callString(Segment().pushBackString("PING")), "PONG");
    a.check("02. help", testee.callString(Segment().pushBackString("HELP")).size() > 20U);

    // - Syntax
    a.checkEqual("11. syntaxget", testee.callString(Segment().pushBackString("SYNTAXGET").pushBackString("KEYWORD")), "Info");
    a.checkEqual("12. syntaxget", testee.callString(Segment().pushBackString("syntaxget").pushBackString("KEYWORD")), "Info");

    // - Render
    a.checkEqual("21. render", testee.callString(Segment().pushBackString("RENDER").pushBackString("text:x").pushBackString("FORMAT").pushBackString("html")), "<p>x</p>\n");
    a.checkEqual("22. render", testee.callString(Segment().pushBackString("render").pushBackString("text:x").pushBackString("format").pushBackString("html")), "<p>x</p>\n");

    // - Group
    a.checkEqual("31. groupget", testee.callString(Segment().pushBackString("GROUPGET").pushBackString("g").pushBackString("name")), "gn");

    // - Forum
    a.checkEqual("41. forumadd", testee.callInt(Segment().pushBackString("FORUMADD").pushBackString("name").pushBackString("f").pushBackString("readperm").pushBackString("all")), 1);

    // - Post
    a.checkEqual("51. postnew", testee.callInt(Segment().pushBackString("POSTNEW").pushBackInteger(1).pushBackString("title").pushBackString("text").pushBackString("USER").pushBackString("a")), 1);

    // - Thread
    std::auto_ptr<afl::data::Value> p;
    AFL_CHECK_SUCCEEDS(a("61. threadstat"), p.reset(testee.call(Segment().pushBackString("THREADSTAT").pushBackInteger(1))));
    a.checkEqual("62. subject", Access(p)("subject").toString(), "title");

    // - User
    AFL_CHECK_SUCCEEDS(a("71. userlsposted"), p.reset(testee.call(Segment().pushBackString("USERLSPOSTED").pushBackString("a"))));
    a.checkEqual("72. result size", Access(p).getArraySize(), 1U);
    a.checkEqual("73. result value", Access(p)[0].toInteger(), 1);

    // - Change user context. Required for Folder/PM.
    testee.callVoid(Segment().pushBackString("USER").pushBackString("1009"));
    a.checkEqual("81. getUser", session.getUser(), "1009");

    // - Folder
    a.checkEqual("91. foldernew", testee.callInt(Segment().pushBackString("FOLDERNEW").pushBackString("fn")), 100);

    // - PM
    a.checkEqual("101. pmnew", testee.callInt(Segment().pushBackString("PMNEW").pushBackString("u:b").pushBackString("pmsubj").pushBackString("pmtext")), 1);

    // - NNTP
    AFL_CHECK_SUCCEEDS(a("111. nntpposthead"), p.reset(testee.call(Segment().pushBackString("NNTPPOSTHEAD").pushBackInteger(1))));
    a.checkEqual("112. result header", Access(p)("Subject").toString(), "title");

    // Some errors
    Segment empty;
    AFL_CHECK_THROWS(a("121. bad verb"), testee.callVoid(Segment().pushBackString("NNTPWHATEVER")), std::exception);
    AFL_CHECK_THROWS(a("122. bad verb"), testee.callVoid(Segment().pushBackString("huh?")), std::exception);
    AFL_CHECK_THROWS(a("123. bad verb"), testee.callVoid(Segment().pushBackString("huh?")), std::exception);
    AFL_CHECK_THROWS(a("124. no verb"), testee.callVoid(empty), std::exception);
}
