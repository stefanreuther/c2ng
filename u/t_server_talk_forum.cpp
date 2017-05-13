/**
  *  \file u/t_server_talk_forum.cpp
  *  \brief Test for server::talk::Forum
  */

#include "server/talk/forum.hpp"

#include "t_server_talk.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "server/talk/root.hpp"
#include "server/talk/session.hpp"

/** Simple test. */
void
TestServerTalkForum::testIt()
{
    // Infrastructure
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, mq, server::talk::Configuration());

    // Forum
    server::talk::Forum testee(root, 3);
    TS_ASSERT_EQUALS(testee.getId(), 3);

    // Create it
    TS_ASSERT(!testee.exists(root));
    root.allForums().add(3);
    TS_ASSERT(testee.exists(root));

    // Attributes
    testee.name().set("F");
    TS_ASSERT_EQUALS(testee.name().get(), "F");

    testee.description().set("text:dd");
    TS_ASSERT_EQUALS(testee.description().get(), "text:dd");

    testee.readPermissions().set("all");
    TS_ASSERT_EQUALS(testee.readPermissions().get(), "all");

    testee.writePermissions().set("-all");
    TS_ASSERT_EQUALS(testee.writePermissions().get(), "-all");

    testee.answerPermissions().set("-all");
    TS_ASSERT_EQUALS(testee.answerPermissions().get(), "-all");

    testee.deletePermissions().set("u:1001");
    TS_ASSERT_EQUALS(testee.deletePermissions().get(), "u:1001");

    testee.key().set("001");
    TS_ASSERT_EQUALS(testee.key().get(), "001");

    TS_ASSERT_EQUALS(testee.lastMessageSequenceNumber().get(), 0);
    testee.lastMessageSequenceNumber().set(9);
    TS_ASSERT_EQUALS(testee.lastMessageSequenceNumber().get(), 9);

    testee.creationTime().set(10001);
    TS_ASSERT_EQUALS(testee.creationTime().get(), 10001);

    testee.lastPostId().set(42);
    TS_ASSERT_EQUALS(testee.lastPostId().get(), 42);

    testee.lastTime().set(10002);
    TS_ASSERT_EQUALS(testee.lastTime().get(), 10002);

    // Sets
    testee.messages().add(3);
    TS_ASSERT(testee.messages().contains(3));

    testee.topics().add(33);
    TS_ASSERT(testee.topics().contains(33));

    testee.stickyTopics().add(333);
    TS_ASSERT(testee.stickyTopics().contains(333));

    // Parenting
    TS_ASSERT_EQUALS(testee.getParent(), "");
    testee.setParent("p1", root);
    TS_ASSERT(root.groupRoot().subtree("p1").intSetKey("forums").contains(3));
    testee.setParent("p2", root);
    TS_ASSERT(root.groupRoot().subtree("p2").intSetKey("forums").contains(3));
    TS_ASSERT(!root.groupRoot().subtree("p1").intSetKey("forums").contains(3));
    testee.setParent("", root);
    TS_ASSERT(!root.groupRoot().subtree("p2").intSetKey("forums").contains(3));
    TS_ASSERT(!root.groupRoot().subtree("p1").intSetKey("forums").contains(3));
    testee.setParent("p2", root);

    // Newsgroup
    testee.setNewsgroup("g.n", root);
    TS_ASSERT_EQUALS(testee.getNewsgroup(), "g.n");

    // Describe
    server::talk::render::Context ctx("u");
    server::talk::render::Options opts;
    opts.setFormat("text");

    server::interface::TalkForum::Info fi = testee.describe(ctx, opts, root);
    TS_ASSERT_EQUALS(fi.name, "F");
    TS_ASSERT_EQUALS(fi.parentGroup, "p2");
    TS_ASSERT_EQUALS(fi.description, "dd");
    TS_ASSERT_EQUALS(fi.newsgroupName, "g.n");

    server::talk::Session session;
    session.setUser("u");
    server::interface::TalkNNTP::Info gi = testee.describeAsNewsgroup(ctx, opts, root, session);
    TS_ASSERT_EQUALS(gi.forumId, 3);
    TS_ASSERT_EQUALS(gi.newsgroupName, "g.n");
    TS_ASSERT_EQUALS(gi.firstSequenceNumber, 1);
    TS_ASSERT_EQUALS(gi.lastSequenceNumber, 9);
    TS_ASSERT_EQUALS(gi.writeAllowed, false);
    TS_ASSERT_EQUALS(gi.description, "dd");
}

/** Test newsgroup behaviour. */
void
TestServerTalkForum::testNewsgroup()
{
    // Infrastructure
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, mq, server::talk::Configuration());

    // 2 forums
    server::talk::Forum a(root, 3);
    server::talk::Forum b(root, 4);

    // Create them
    root.allForums().add(3);
    root.allForums().add(4);
    TS_ASSERT(a.exists(root));
    TS_ASSERT(b.exists(root));

    // Initial state
    TS_ASSERT_EQUALS(a.getNewsgroup(), "");
    TS_ASSERT_EQUALS(b.getNewsgroup(), "");

    // Make a newsgroup
    a.setNewsgroup("n.g", root);
    TS_ASSERT_EQUALS(a.getNewsgroup(), "n.g");
    TS_ASSERT_EQUALS(b.getNewsgroup(), "");
    TS_ASSERT_EQUALS(root.newsgroupMap().intField("n.g").get(), 3);

    // Make b the same newsgroup
    b.setNewsgroup("n.g", root);
    TS_ASSERT_EQUALS(a.getNewsgroup(), "");
    TS_ASSERT_EQUALS(b.getNewsgroup(), "n.g");
    TS_ASSERT_EQUALS(root.newsgroupMap().intField("n.g").get(), 4);

    // Make a another group
    a.setNewsgroup("n.a", root);
    TS_ASSERT_EQUALS(a.getNewsgroup(), "n.a");
    TS_ASSERT_EQUALS(b.getNewsgroup(), "n.g");
    TS_ASSERT_EQUALS(root.newsgroupMap().intField("n.a").get(), 3);
    TS_ASSERT_EQUALS(root.newsgroupMap().intField("n.g").get(), 4);

    // Rename a group
    b.setNewsgroup("n.b", root);
    TS_ASSERT_EQUALS(a.getNewsgroup(), "n.a");
    TS_ASSERT_EQUALS(b.getNewsgroup(), "n.b");
    TS_ASSERT_EQUALS(root.newsgroupMap().intField("n.a").get(), 3);
    TS_ASSERT_EQUALS(root.newsgroupMap().intField("n.b").get(), 4);
    TS_ASSERT_EQUALS(root.newsgroupMap().intField("n.g").get(), 0);

    // Rename and overwrite in one stpe
    b.setNewsgroup("n.a", root);
    TS_ASSERT_EQUALS(a.getNewsgroup(), "");
    TS_ASSERT_EQUALS(b.getNewsgroup(), "n.a");
    TS_ASSERT_EQUALS(root.newsgroupMap().intField("n.a").get(), 4);
    TS_ASSERT_EQUALS(root.newsgroupMap().intField("n.b").get(), 0);
    TS_ASSERT_EQUALS(root.newsgroupMap().intField("n.g").get(), 0);
}

/** Test ForumSorter. */
void
TestServerTalkForum::testSort()
{
    // Infrastructure
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, mq, server::talk::Configuration());

    // Create three forums
    server::talk::Forum a(root, 3);
    server::talk::Forum b(root, 4);
    server::talk::Forum c(root, 5);

    root.allForums().add(3);
    root.allForums().add(4);
    root.allForums().add(5);

    a.key().set("eins");
    b.key().set("zwo");
    c.key().set("drei");

    a.lastPostId().set(900);
    b.lastPostId().set(902);
    c.lastPostId().set(901);

    a.lastTime().set(10001);
    b.lastTime().set(9999);
    c.lastTime().set(10002);

    a.name().set("first");
    b.name().set("second");
    c.name().set("third");

    // Try sorting
    // - key: drei,eins,zwo
    {
        afl::data::IntegerList_t result;
        afl::net::redis::SortOperation op = root.allForums().sort().get();
        server::talk::Forum::ForumSorter(root).applySortKey(op, "KEY");
        op.getResult(result);
        TS_ASSERT_EQUALS(result.size(), 3U);
        TS_ASSERT_EQUALS(result[0], 5);
        TS_ASSERT_EQUALS(result[1], 3);
        TS_ASSERT_EQUALS(result[2], 4);
    }
    // - lastPost: 900,901,902
    {
        afl::data::IntegerList_t result;
        afl::net::redis::SortOperation op = root.allForums().sort().get();
        server::talk::Forum::ForumSorter(root).applySortKey(op, "LASTPOST");
        op.getResult(result);
        TS_ASSERT_EQUALS(result.size(), 3U);
        TS_ASSERT_EQUALS(result[0], 3);
        TS_ASSERT_EQUALS(result[1], 5);
        TS_ASSERT_EQUALS(result[2], 4);
    }
    // - lastTime: 9999,10001,10002
    {
        afl::data::IntegerList_t result;
        afl::net::redis::SortOperation op = root.allForums().sort().get();
        server::talk::Forum::ForumSorter(root).applySortKey(op, "LASTTIME");
        op.getResult(result);
        TS_ASSERT_EQUALS(result.size(), 3U);
        TS_ASSERT_EQUALS(result[0], 4);
        TS_ASSERT_EQUALS(result[1], 3);
        TS_ASSERT_EQUALS(result[2], 5);
    }
    // - name: first,second,third
    {
        afl::data::IntegerList_t result;
        afl::net::redis::SortOperation op = root.allForums().sort().get();
        server::talk::Forum::ForumSorter(root).applySortKey(op, "NAME");
        op.getResult(result);
        TS_ASSERT_EQUALS(result.size(), 3U);
        TS_ASSERT_EQUALS(result[0], 3);
        TS_ASSERT_EQUALS(result[1], 4);
        TS_ASSERT_EQUALS(result[2], 5);
    }
    // - error case
    {
        afl::data::IntegerList_t result;
        afl::net::redis::SortOperation op = root.allForums().sort().get();
        TS_ASSERT_THROWS(server::talk::Forum::ForumSorter(root).applySortKey(op, "name"), std::runtime_error);
        TS_ASSERT_THROWS(server::talk::Forum::ForumSorter(root).applySortKey(op, "OTHER"), std::runtime_error);
        TS_ASSERT_THROWS(server::talk::Forum::ForumSorter(root).applySortKey(op, ""), std::runtime_error);
    }
}

