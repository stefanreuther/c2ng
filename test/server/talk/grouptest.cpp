/**
  *  \file test/server/talk/grouptest.cpp
  *  \brief Test for server::talk::Group
  */

#include "server/talk/group.hpp"

#include "afl/data/segment.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/test/testrunner.hpp"

/** Simple basic test. */
AFL_TEST("server.talk.Group", a)
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
    a.check("01. exists", t.exists());
    a.checkEqual("02", t.name().get(), "All forums");
    a.checkEqual("03", t.description().get(), "text:All forums");
    a.checkEqual("04", t.key().get(), "root");
    a.checkEqual("05. getParent", t.getParent(), "");

    a.checkEqual("11", t.forums().size(), 2);
    a.check("12", t.forums().contains(1));
    a.check("13", !t.forums().contains(5));

    a.checkEqual("21", t.subgroups().size(), 1);
    a.check("22", t.subgroups().contains("active"));

    // Description
    {
        server::talk::render::Context ctx("u");
        server::talk::render::Options opts;
        opts.setFormat("html");
        server::interface::TalkGroup::Description desc = t.describe(ctx, opts, root);
        a.checkEqual("31. name", desc.name.orElse("fail"), "All forums");
        a.checkEqual("32. description", desc.description.orElse("fail"), "<p>All forums</p>\n");
        a.checkEqual("33. parentGroup", desc.parentGroup.orElse("fail"), "");
        a.checkEqual("34. unlisted", desc.unlisted.orElse(true), false);
    }

    // Test another
    a.check("41", !server::talk::Group(root, "foo").exists());
}

/** Test setParent. */
AFL_TEST("server.talk.Group:setParent", a)
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
    a.checkEqual("01. getParent", t.getParent(), "r1");
    a.checkEqual("02. group r1", db.callInt(Segment().pushBackString("scard").pushBackString("group:r1:groups")), 1);
    a.checkEqual("03. group r2", db.callInt(Segment().pushBackString("scard").pushBackString("group:r2:groups")), 0);

    // Move
    t.setParent("r2", root);
    a.checkEqual("11. getParent", t.getParent(), "r2");
    a.checkEqual("12. group r1", db.callInt(Segment().pushBackString("scard").pushBackString("group:r1:groups")), 0);
    a.checkEqual("13. group r2", db.callInt(Segment().pushBackString("scard").pushBackString("group:r2:groups")), 1);

    // Move again with no change
    t.setParent("r2", root);
    a.checkEqual("21. getParent", t.getParent(), "r2");
    a.checkEqual("22. group r1", db.callInt(Segment().pushBackString("scard").pushBackString("group:r1:groups")), 0);
    a.checkEqual("23. group r2", db.callInt(Segment().pushBackString("scard").pushBackString("group:r2:groups")), 1);

    // Move out of groups
    t.setParent("", root);
    a.checkEqual("31. getParent", t.getParent(), "");
    a.checkEqual("32. group r1", db.callInt(Segment().pushBackString("scard").pushBackString("group:r1:groups")), 0);
    a.checkEqual("33. group r2", db.callInt(Segment().pushBackString("scard").pushBackString("group:r2:groups")), 0);

    // Move back into a group
    t.setParent("r1", root);
    a.checkEqual("41. getParent", t.getParent(), "r1");
    a.checkEqual("42. group r1", db.callInt(Segment().pushBackString("scard").pushBackString("group:r1:groups")), 1);
    a.checkEqual("43. group r2", db.callInt(Segment().pushBackString("scard").pushBackString("group:r2:groups")), 0);
}
