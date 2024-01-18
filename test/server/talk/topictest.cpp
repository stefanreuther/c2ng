/**
  *  \file test/server/talk/topictest.cpp
  *  \brief Test for server::talk::Topic
  */

#include "server/talk/topic.hpp"

#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/sortoperation.hpp"
#include "afl/test/testrunner.hpp"
#include "server/talk/forum.hpp"
#include "server/talk/message.hpp"
#include "server/talk/root.hpp"

/** Simple test. */
AFL_TEST("server.talk.Topic:basics", a)
{
    // Infrastructure
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, mq, server::talk::Configuration());

    // Topic
    server::talk::Topic testee(root, 38);
    a.check("01. exists", !testee.exists());
    a.checkEqual("02. getId", testee.getId(), 38);

    // Create and verify it by accessing header fields
    testee.subject().set("subj");
    testee.forumId().set(9);
    testee.firstPostingId().set(120);
    testee.readPermissions().set("all");
    testee.answerPermissions().set("u:a");
    testee.lastPostId().set(121);
    testee.lastTime().set(191919);
    testee.messages().add(120);
    testee.messages().add(121);
    testee.watchers().add("x");

    a.check     ("11. exists",            testee.exists());
    a.checkEqual("12. subject",           testee.subject().get(), "subj");
    a.checkEqual("13. forumId",           testee.forumId().get(), 9);
    a.checkEqual("14. firstPostingId",    testee.firstPostingId().get(), 120);
    a.checkEqual("15. readPermissions",   testee.readPermissions().get(), "all");
    a.checkEqual("16. answerPermissions", testee.answerPermissions().get(), "u:a");
    a.checkEqual("17. lastPostId",        testee.lastPostId().get(), 121);
    a.checkEqual("18. lastTime",          testee.lastTime().get(), 191919);
    a.check     ("19. messages",          testee.messages().contains(120));
    a.check     ("20. messages",          testee.messages().contains(121));
    a.check     ("21. watchers",          testee.watchers().contains("x"));

    // Forum
    server::talk::Forum f(testee.forum(root));
    a.checkEqual("31. getId", f.getId(), 9);
    f.stickyTopics().add(testee.getId());

    // Verify stickyness behaviour
    a.check("41. isSticky", !testee.isSticky());
    testee.setSticky(root, true);
    a.check("42. isSticky", testee.isSticky());
    a.check("43. topics", !f.topics().contains(testee.getId()));
    a.check("44. stickyTopics", f.stickyTopics().contains(testee.getId()));

    testee.setSticky(root, false);
    a.check("51. isSticky", !testee.isSticky());
    a.check("52. topics", f.topics().contains(testee.getId()));
    a.check("53. stickyTopics", !f.stickyTopics().contains(testee.getId()));

    testee.setSticky(root, false); // no-op
    a.check("61. isSticky", !testee.isSticky());
    a.check("62. topics", f.topics().contains(testee.getId()));
    a.check("63. stickyTopics", !f.stickyTopics().contains(testee.getId()));

    // Describe
    server::interface::TalkThread::Info i = testee.describe();
    a.checkEqual("71. subject",     i.subject, "subj");
    a.checkEqual("72. forumId",     i.forumId, 9);
    a.checkEqual("73. firstPostId", i.firstPostId, 120);
    a.checkEqual("74. lastPostId",  i.lastPostId, 121);
    a.checkEqual("75. lastTime",    i.lastTime, 191919);
    a.checkEqual("76. isSticky",    i.isSticky, false);
}

/** Test removal. */
AFL_TEST("server.talk.Topic:remove", a)
{
    for (int sticky = 0; sticky < 2; ++sticky) {
        // Infrastructure
        afl::net::NullCommandHandler mq;
        afl::net::redis::InternalDatabase db;
        server::talk::Root root(db, mq, server::talk::Configuration());

        const int FORUM_ID = 12;
        const int TOPIC_ID = 55;
        const int MESSAGE1_ID = 150;
        const int MESSAGE2_ID = 152;

        // Forum
        server::talk::Forum f(root, FORUM_ID);
        f.name().set("f");
        f.topics().add(TOPIC_ID);
        f.messages().add(MESSAGE1_ID);
        f.messages().add(MESSAGE2_ID);

        // Topic
        server::talk::Topic t(root, TOPIC_ID);
        t.forumId().set(FORUM_ID);
        t.subject().set("s");
        t.messages().add(MESSAGE1_ID);
        t.messages().add(MESSAGE2_ID);

        // Messages
        server::talk::Message m1(root, MESSAGE1_ID);
        m1.topicId().set(TOPIC_ID);
        m1.author().set("a");

        server::talk::Message m2(root, MESSAGE2_ID);
        m2.topicId().set(TOPIC_ID);
        m2.author().set("a");

        // Stickyness!
        if (sticky != 0) {
            t.setSticky(root, true);
        }

        // Remove
        t.remove(root);

        // Must be gone!
        a.check("01. topics",       !f.topics().contains(TOPIC_ID));
        a.check("02. stickyTopics", !f.stickyTopics().contains(TOPIC_ID));
        a.check("03. messages",     !f.messages().contains(MESSAGE1_ID));
        a.check("04. messages",     !f.messages().contains(MESSAGE2_ID));
        a.check("05. exists",       !t.exists());
        a.check("06. exists",       !m1.exists());
        a.check("07. exists",       !m2.exists());
    }
}

/** Test sorting. */
AFL_TEST("server.talk.Topic:sort", a)
{
    // Infrastructure
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, mq, server::talk::Configuration());

    // Preloaded database
    const int N = 5;
    struct Data {
        const char* subject;
        int firstPost;
        int lastPost;
        int forum;
        int lastTime;
    };
    static const Data d[N] = {
        { "a",  100, 120,  17,   20000 },    // #100: first subject
        { "b",   90, 105,  18,   20100 },    // #101: first firstPost
        { "c",   95,  96,  20,   30000 },    // #102: first lastPost
        { "d",  107, 111,   8,   42000 },    // #103: first forum
        { "e",  121, 122,  16,    9000 },    // #104: first time
    };
    afl::net::redis::IntegerSetKey key(db, "some_key");
    for (int i = 0; i < N; ++i) {
        server::talk::Topic t(root, 100+i);
        t.subject().set(d[i].subject);
        t.firstPostingId().set(d[i].firstPost);
        t.lastPostId().set(d[i].lastPost);
        t.forumId().set(d[i].forum);
        t.lastTime().set(d[i].lastTime);
        key.add(t.getId());
    }

    // Check it
    using afl::net::redis::SortOperation;
    using afl::data::IntegerList_t;
    {
        IntegerList_t result;
        SortOperation op(key.sort());
        server::talk::Topic::TopicSorter(root).applySortKey(op, "SUBJECT");
        op.getResult(result);
        a.checkEqual("01. size", result.size(), 5U);
        a.checkEqual("02. result", result[0], 100);
    }
    {
        IntegerList_t result;
        SortOperation op(key.sort());
        server::talk::Topic::TopicSorter(root).applySortKey(op, "FIRSTPOST");
        op.getResult(result);
        a.checkEqual("03. size", result.size(), 5U);
        a.checkEqual("04. result", result[0], 101);
    }
    {
        IntegerList_t result;
        SortOperation op(key.sort());
        server::talk::Topic::TopicSorter(root).applySortKey(op, "LASTPOST");
        op.getResult(result);
        a.checkEqual("05. size", result.size(), 5U);
        a.checkEqual("06. result", result[0], 102);
    }
    {
        IntegerList_t result;
        SortOperation op(key.sort());
        server::talk::Topic::TopicSorter(root).applySortKey(op, "FORUM");
        op.getResult(result);
        a.checkEqual("07. size", result.size(), 5U);
        a.checkEqual("08. result", result[0], 103);
    }
    {
        IntegerList_t result;
        SortOperation op(key.sort());
        server::talk::Topic::TopicSorter(root).applySortKey(op, "LASTTIME");
        op.getResult(result);
        a.checkEqual("09. size", result.size(), 5U);
        a.checkEqual("10. result", result[0], 104);
    }

    // Error cases
    {
        SortOperation op(key.sort());
        AFL_CHECK_THROWS(a("11. bad key"), server::talk::Topic::TopicSorter(root).applySortKey(op, "lasttime"), std::exception);
        AFL_CHECK_THROWS(a("12. bad key"), server::talk::Topic::TopicSorter(root).applySortKey(op, ""), std::exception);
        AFL_CHECK_THROWS(a("13. bad key"), server::talk::Topic::TopicSorter(root).applySortKey(op, "WHATEVER"), std::exception);
    }
}
