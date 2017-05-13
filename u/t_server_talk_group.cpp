/**
  *  \file u/t_server_talk_group.cpp
  *  \brief Test for server::talk::Group
  */

#include "server/talk/group.hpp"

#include "t_server_talk.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/data/segment.hpp"
#include "afl/net/nullcommandhandler.hpp"

/** Simple basic test. */
void
TestServerTalkGroup::testIt()
{
    // Set up some situation
    using afl::data::Segment;
    afl::net::redis::InternalDatabase db;

    // The "root" definition from PlanetsCentral, simplified
    db.callVoid(Segment().pushBackString("sadd").pushBackString("group:root:forums").pushBackString("1").pushBackString("2"));
    db.callVoid(Segment().pushBackString("sadd").pushBackString("group:root:groups").pushBackString("active"));
    db.callVoid(Segment().pushBackString("hmset").pushBackString("group:root:header").
                pushBackString("description").pushBackString("text:All forums").
                pushBackString("key").pushBackString("root").
                pushBackString("name").pushBackString("All forums"));

    afl::net::NullCommandHandler mq;
    server::talk::Root root(db, mq, server::talk::Configuration());

    // Test it
    server::talk::Group t(root, "root");
    TS_ASSERT(t.exists());
    TS_ASSERT_EQUALS(t.name().get(), "All forums");
    TS_ASSERT_EQUALS(t.description().get(), "text:All forums");
    TS_ASSERT_EQUALS(t.key().get(), "root");
    TS_ASSERT_EQUALS(t.getParent(), "");

    TS_ASSERT_EQUALS(t.forums().size(), 2);
    TS_ASSERT(t.forums().contains(1));
    TS_ASSERT(!t.forums().contains(5));

    TS_ASSERT_EQUALS(t.subgroups().size(), 1);
    TS_ASSERT(t.subgroups().contains("active"));

    // Description
    {
        server::talk::render::Context ctx("u");
        server::talk::render::Options opts;
        opts.setFormat("html");
        server::interface::TalkGroup::Description desc = t.describe(ctx, opts, root);
        TS_ASSERT_EQUALS(desc.name.orElse("fail"), "All forums");
        TS_ASSERT_EQUALS(desc.description.orElse("fail"), "<p>All forums</p>\n");
        TS_ASSERT_EQUALS(desc.parentGroup.orElse("fail"), "");
        TS_ASSERT_EQUALS(desc.unlisted.orElse(true), false);
    }

    // Test another
    TS_ASSERT(!server::talk::Group(root, "foo").exists());
}

/** Test setParent. */
void
TestServerTalkGroup::testSetParent()
{
    using afl::data::Segment;
    afl::net::redis::InternalDatabase db;
    afl::net::NullCommandHandler mq;
    server::talk::Root root(db, mq, server::talk::Configuration());

    // Create two root groups
    db.callVoid(Segment().pushBackString("hmset").pushBackString("group:r1:header").pushBackString("name").pushBackString("Root One"));
    db.callVoid(Segment().pushBackString("hmset").pushBackString("group:r2:header").pushBackString("name").pushBackString("Root Two"));

    // Create a child group
    db.callVoid(Segment().pushBackString("hmset").pushBackString("group:ch:header").pushBackString("name").pushBackString("Child")
                .pushBackString("parent").pushBackString("r1"));
    db.callVoid(Segment().pushBackString("sadd").pushBackString("group:r1:groups").pushBackString("ch"));

    // Verify
    server::talk::Group t(root, "ch");
    TS_ASSERT_EQUALS(t.getParent(), "r1");
    TS_ASSERT_EQUALS(db.callInt(Segment().pushBackString("scard").pushBackString("group:r1:groups")), 1);
    TS_ASSERT_EQUALS(db.callInt(Segment().pushBackString("scard").pushBackString("group:r2:groups")), 0);

    // Move
    t.setParent("r2", root);
    TS_ASSERT_EQUALS(t.getParent(), "r2");
    TS_ASSERT_EQUALS(db.callInt(Segment().pushBackString("scard").pushBackString("group:r1:groups")), 0);
    TS_ASSERT_EQUALS(db.callInt(Segment().pushBackString("scard").pushBackString("group:r2:groups")), 1);

    // Move again with no change
    t.setParent("r2", root);
    TS_ASSERT_EQUALS(t.getParent(), "r2");
    TS_ASSERT_EQUALS(db.callInt(Segment().pushBackString("scard").pushBackString("group:r1:groups")), 0);
    TS_ASSERT_EQUALS(db.callInt(Segment().pushBackString("scard").pushBackString("group:r2:groups")), 1);

    // Move out of groups
    t.setParent("", root);
    TS_ASSERT_EQUALS(t.getParent(), "");
    TS_ASSERT_EQUALS(db.callInt(Segment().pushBackString("scard").pushBackString("group:r1:groups")), 0);
    TS_ASSERT_EQUALS(db.callInt(Segment().pushBackString("scard").pushBackString("group:r2:groups")), 0);

    // Move back into a group
    t.setParent("r1", root);
    TS_ASSERT_EQUALS(t.getParent(), "r1");
    TS_ASSERT_EQUALS(db.callInt(Segment().pushBackString("scard").pushBackString("group:r1:groups")), 1);
    TS_ASSERT_EQUALS(db.callInt(Segment().pushBackString("scard").pushBackString("group:r2:groups")), 0);
}
