/**
  *  \file u/t_server_talk_commandhandler.cpp
  *  \brief Test for server::talk::CommandHandler
  */

#include "server/talk/commandhandler.hpp"

#include "t_server_talk.hpp"
#include "server/talk/session.hpp"
#include "server/talk/root.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/data/segment.hpp"
#include "afl/data/access.hpp"
#include "afl/net/redis/stringfield.hpp"

using afl::data::Segment;
using afl::data::Access;

/** Simple test.
    Call once into every child element to make sure command routing works. */
void
TestServerTalkCommandHandler::testIt()
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
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("PING")), "PONG");
    TS_ASSERT(testee.callString(Segment().pushBackString("HELP")).size() > 20U);

    // - Syntax
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("SYNTAXGET").pushBackString("KEYWORD")), "Info");
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("syntaxget").pushBackString("KEYWORD")), "Info");

    // - Render
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("RENDER").pushBackString("text:x").pushBackString("FORMAT").pushBackString("html")), "<p>x</p>\n");
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("render").pushBackString("text:x").pushBackString("format").pushBackString("html")), "<p>x</p>\n");

    // - Group
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("GROUPGET").pushBackString("g").pushBackString("name")), "gn");

    // - Forum
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("FORUMADD").pushBackString("name").pushBackString("f").pushBackString("readperm").pushBackString("all")), 1);

    // - Post
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("POSTNEW").pushBackInteger(1).pushBackString("title").pushBackString("text").pushBackString("USER").pushBackString("a")), 1);

    // - Thread
    std::auto_ptr<afl::data::Value> p;
    TS_ASSERT_THROWS_NOTHING(p.reset(testee.call(Segment().pushBackString("THREADSTAT").pushBackInteger(1))));
    TS_ASSERT_EQUALS(Access(p)("subject").toString(), "title");

    // - User
    TS_ASSERT_THROWS_NOTHING(p.reset(testee.call(Segment().pushBackString("USERLSPOSTED").pushBackString("a"))));
    TS_ASSERT_EQUALS(Access(p).getArraySize(), 1U);
    TS_ASSERT_EQUALS(Access(p)[0].toInteger(), 1);

    // - Change user context. Required for Folder/PM.
    testee.callVoid(Segment().pushBackString("USER").pushBackString("1009"));
    TS_ASSERT_EQUALS(session.getUser(), "1009");

    // - Folder
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("FOLDERNEW").pushBackString("fn")), 100);

    // - PM
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("PMNEW").pushBackString("u:b").pushBackString("pmsubj").pushBackString("pmtext")), 1);

    // - NNTP
    TS_ASSERT_THROWS_NOTHING(p.reset(testee.call(Segment().pushBackString("NNTPPOSTHEAD").pushBackInteger(1))));
    TS_ASSERT_EQUALS(Access(p)("Subject").toString(), "title");

    // Some errors
    Segment empty;
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("NNTPWHATEVER")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("huh?")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("huh?")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(empty), std::exception);
}

