/**
  *  \file test/server/talk/forumtest.cpp
  *  \brief Test for server::talk::Forum
  */

#include "server/talk/forum.hpp"

#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/test/testrunner.hpp"
#include "server/talk/root.hpp"
#include "server/talk/session.hpp"

/** Simple test. */
AFL_TEST("server.talk.Forum:basics", a)
{
    // Infrastructure
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, mq, server::talk::Configuration());

    // Forum
    server::talk::Forum testee(root, 3);
    a.checkEqual("01. getId", testee.getId(), 3);

    // Create it
    a.check("11. exists", !testee.exists(root));
    root.allForums().add(3);
    a.check("12. exists", testee.exists(root));

    // Attributes
    testee.name().set("F");
    a.checkEqual("21. name", testee.name().get(), "F");

    testee.description().set("text:dd");
    a.checkEqual("31. description", testee.description().get(), "text:dd");

    testee.readPermissions().set("all");
    a.checkEqual("41. readPermissions", testee.readPermissions().get(), "all");

    testee.writePermissions().set("-all");
    a.checkEqual("51. writePermissions", testee.writePermissions().get(), "-all");

    testee.answerPermissions().set("-all");
    a.checkEqual("61. answerPermissions", testee.answerPermissions().get(), "-all");

    testee.deletePermissions().set("u:1001");
    a.checkEqual("71. deletePermissions", testee.deletePermissions().get(), "u:1001");

    testee.key().set("001");
    a.checkEqual("81. key", testee.key().get(), "001");

    a.checkEqual("91. lastMessageSequenceNumber", testee.lastMessageSequenceNumber().get(), 0);
    testee.lastMessageSequenceNumber().set(9);
    a.checkEqual("92. lastMessageSequenceNumber", testee.lastMessageSequenceNumber().get(), 9);

    testee.creationTime().set(10001);
    a.checkEqual("101. creationTime", testee.creationTime().get(), 10001);

    testee.lastPostId().set(42);
    a.checkEqual("111. lastPostId", testee.lastPostId().get(), 42);

    testee.lastTime().set(10002);
    a.checkEqual("121. lastTime", testee.lastTime().get(), 10002);

    // Sets
    testee.messages().add(3);
    a.check("131. messages", testee.messages().contains(3));

    testee.topics().add(33);
    a.check("141. topics", testee.topics().contains(33));

    testee.stickyTopics().add(333);
    a.check("151. stickyTopics", testee.stickyTopics().contains(333));

    // Parenting
    a.checkEqual("161. getParent", testee.getParent(), "");
    testee.setParent("p1", root);
    a.check("162. groupRoot", root.groupRoot().subtree("p1").intSetKey("forums").contains(3));
    testee.setParent("p2", root);
    a.check("163. groupRoot", root.groupRoot().subtree("p2").intSetKey("forums").contains(3));
    a.check("164. groupRoot", !root.groupRoot().subtree("p1").intSetKey("forums").contains(3));
    testee.setParent("", root);
    a.check("165. groupRoot", !root.groupRoot().subtree("p2").intSetKey("forums").contains(3));
    a.check("166. groupRoot", !root.groupRoot().subtree("p1").intSetKey("forums").contains(3));
    testee.setParent("p2", root);

    // Newsgroup
    testee.setNewsgroup("g.n", root);
    a.checkEqual("171. getNewsgroup", testee.getNewsgroup(), "g.n");

    // Describe
    server::talk::render::Context ctx("u");
    server::talk::render::Options opts;
    opts.setFormat("text");

    server::interface::TalkForum::Info fi = testee.describe(ctx, opts, root);
    a.checkEqual("181. name",          fi.name, "F");
    a.checkEqual("182. parentGroup",   fi.parentGroup, "p2");
    a.checkEqual("183. description",   fi.description, "dd");
    a.checkEqual("184. newsgroupName", fi.newsgroupName, "g.n");

    server::talk::Session session;
    session.setUser("u");
    server::interface::TalkNNTP::Info gi = testee.describeAsNewsgroup(ctx, opts, root, session);
    a.checkEqual("191. forumId",             gi.forumId, 3);
    a.checkEqual("192. newsgroupName",       gi.newsgroupName, "g.n");
    a.checkEqual("193. firstSequenceNumber", gi.firstSequenceNumber, 1);
    a.checkEqual("194. lastSequenceNumber",  gi.lastSequenceNumber, 9);
    a.checkEqual("195. writeAllowed",        gi.writeAllowed, false);
    a.checkEqual("196. description",         gi.description, "dd");
}

/** Test newsgroup behaviour. */
AFL_TEST("server.talk.Forum:newsgroup", a)
{
    // Infrastructure
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, mq, server::talk::Configuration());

    // 2 forums
    server::talk::Forum fa(root, 3);
    server::talk::Forum fb(root, 4);

    // Create them
    root.allForums().add(3);
    root.allForums().add(4);
    a.check("01. exists", fa.exists(root));
    a.check("02. exists", fb.exists(root));

    // Initial state
    a.checkEqual("11. getNewsgroup", fa.getNewsgroup(), "");
    a.checkEqual("12. getNewsgroup", fb.getNewsgroup(), "");

    // Make a newsgroup
    fa.setNewsgroup("n.g", root);
    a.checkEqual("21. getNewsgroup", fa.getNewsgroup(), "n.g");
    a.checkEqual("22. getNewsgroup", fb.getNewsgroup(), "");
    a.checkEqual("23. newsgroupMap", root.newsgroupMap().intField("n.g").get(), 3);

    // Make b the same newsgroup
    fb.setNewsgroup("n.g", root);
    a.checkEqual("31. getNewsgroup", fa.getNewsgroup(), "");
    a.checkEqual("32. getNewsgroup", fb.getNewsgroup(), "n.g");
    a.checkEqual("33. newsgroupMap", root.newsgroupMap().intField("n.g").get(), 4);

    // Make a another group
    fa.setNewsgroup("n.a", root);
    a.checkEqual("41. getNewsgroup", fa.getNewsgroup(), "n.a");
    a.checkEqual("42. getNewsgroup", fb.getNewsgroup(), "n.g");
    a.checkEqual("43. newsgroupMap", root.newsgroupMap().intField("n.a").get(), 3);
    a.checkEqual("44. newsgroupMap", root.newsgroupMap().intField("n.g").get(), 4);

    // Rename a group
    fb.setNewsgroup("n.b", root);
    a.checkEqual("51. getNewsgroup", fa.getNewsgroup(), "n.a");
    a.checkEqual("52. getNewsgroup", fb.getNewsgroup(), "n.b");
    a.checkEqual("53. newsgroupMap", root.newsgroupMap().intField("n.a").get(), 3);
    a.checkEqual("54. newsgroupMap", root.newsgroupMap().intField("n.b").get(), 4);
    a.checkEqual("55. newsgroupMap", root.newsgroupMap().intField("n.g").get(), 0);

    // Rename and overwrite in one stpe
    fb.setNewsgroup("n.a", root);
    a.checkEqual("61. getNewsgroup", fa.getNewsgroup(), "");
    a.checkEqual("62. getNewsgroup", fb.getNewsgroup(), "n.a");
    a.checkEqual("63. newsgroupMap", root.newsgroupMap().intField("n.a").get(), 4);
    a.checkEqual("64. newsgroupMap", root.newsgroupMap().intField("n.b").get(), 0);
    a.checkEqual("65. newsgroupMap", root.newsgroupMap().intField("n.g").get(), 0);
}

/** Test ForumSorter. */
AFL_TEST("server.talk.Forum:sort", a)
{
    // Infrastructure
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, mq, server::talk::Configuration());

    // Create three forums
    server::talk::Forum fa(root, 3);
    server::talk::Forum fb(root, 4);
    server::talk::Forum fc(root, 5);

    root.allForums().add(3);
    root.allForums().add(4);
    root.allForums().add(5);

    fa.key().set("eins");
    fb.key().set("zwo");
    fc.key().set("drei");

    fa.lastPostId().set(900);
    fb.lastPostId().set(902);
    fc.lastPostId().set(901);

    fa.lastTime().set(10001);
    fb.lastTime().set(9999);
    fc.lastTime().set(10002);

    fa.name().set("first");
    fb.name().set("second");
    fc.name().set("third");

    // Try sorting
    // - key: drei,eins,zwo
    {
        afl::data::IntegerList_t result;
        afl::net::redis::SortOperation op = root.allForums().sort().get();
        server::talk::Forum::ForumSorter(root).applySortKey(op, "KEY");
        op.getResult(result);
        a.checkEqual("01. size", result.size(), 3U);
        a.checkEqual("02. result", result[0], 5);
        a.checkEqual("03. result", result[1], 3);
        a.checkEqual("04. result", result[2], 4);
    }
    // - lastPost: 900,901,902
    {
        afl::data::IntegerList_t result;
        afl::net::redis::SortOperation op = root.allForums().sort().get();
        server::talk::Forum::ForumSorter(root).applySortKey(op, "LASTPOST");
        op.getResult(result);
        a.checkEqual("05. size", result.size(), 3U);
        a.checkEqual("06. result", result[0], 3);
        a.checkEqual("07. result", result[1], 5);
        a.checkEqual("08. result", result[2], 4);
    }
    // - lastTime: 9999,10001,10002
    {
        afl::data::IntegerList_t result;
        afl::net::redis::SortOperation op = root.allForums().sort().get();
        server::talk::Forum::ForumSorter(root).applySortKey(op, "LASTTIME");
        op.getResult(result);
        a.checkEqual("09. size", result.size(), 3U);
        a.checkEqual("10. result", result[0], 4);
        a.checkEqual("11. result", result[1], 3);
        a.checkEqual("12. result", result[2], 5);
    }
    // - name: first,second,third
    {
        afl::data::IntegerList_t result;
        afl::net::redis::SortOperation op = root.allForums().sort().get();
        server::talk::Forum::ForumSorter(root).applySortKey(op, "NAME");
        op.getResult(result);
        a.checkEqual("13. size", result.size(), 3U);
        a.checkEqual("14. result", result[0], 3);
        a.checkEqual("15. result", result[1], 4);
        a.checkEqual("16. result", result[2], 5);
    }
    // - error case
    {
        afl::data::IntegerList_t result;
        afl::net::redis::SortOperation op = root.allForums().sort().get();
        AFL_CHECK_THROWS(a("17. bad key"), server::talk::Forum::ForumSorter(root).applySortKey(op, "name"), std::runtime_error);
        AFL_CHECK_THROWS(a("18. bad key"), server::talk::Forum::ForumSorter(root).applySortKey(op, "OTHER"), std::runtime_error);
        AFL_CHECK_THROWS(a("19. bad key"), server::talk::Forum::ForumSorter(root).applySortKey(op, ""), std::runtime_error);
    }
}
