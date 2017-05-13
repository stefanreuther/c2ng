/**
  *  \file u/t_server_talk_topic.cpp
  *  \brief Test for server::talk::Topic
  */

#include "server/talk/topic.hpp"

#include "t_server_talk.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "server/talk/root.hpp"
#include "server/talk/forum.hpp"
#include "server/talk/message.hpp"
#include "afl/net/redis/sortoperation.hpp"

/** Simple test. */
void
TestServerTalkTopic::testSimple()
{
    // Infrastructure
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, mq, server::talk::Configuration());

    // Topic
    server::talk::Topic testee(root, 38);
    TS_ASSERT(!testee.exists());
    TS_ASSERT_EQUALS(testee.getId(), 38);

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

    TS_ASSERT(testee.exists());
    TS_ASSERT_EQUALS(testee.subject().get(), "subj");
    TS_ASSERT_EQUALS(testee.forumId().get(), 9);
    TS_ASSERT_EQUALS(testee.firstPostingId().get(), 120);
    TS_ASSERT_EQUALS(testee.readPermissions().get(), "all");
    TS_ASSERT_EQUALS(testee.answerPermissions().get(), "u:a");
    TS_ASSERT_EQUALS(testee.lastPostId().get(), 121);
    TS_ASSERT_EQUALS(testee.lastTime().get(), 191919);
    TS_ASSERT(testee.messages().contains(120));
    TS_ASSERT(testee.messages().contains(121));
    TS_ASSERT(testee.watchers().contains("x"));

    // Forum
    server::talk::Forum f(testee.forum(root));
    TS_ASSERT_EQUALS(f.getId(), 9);
    f.stickyTopics().add(testee.getId());

    // Verify stickyness behaviour
    TS_ASSERT(!testee.isSticky());
    testee.setSticky(root, true);
    TS_ASSERT(testee.isSticky());
    TS_ASSERT(!f.topics().contains(testee.getId()));
    TS_ASSERT(f.stickyTopics().contains(testee.getId()));

    testee.setSticky(root, false);
    TS_ASSERT(!testee.isSticky());
    TS_ASSERT(f.topics().contains(testee.getId()));
    TS_ASSERT(!f.stickyTopics().contains(testee.getId()));

    testee.setSticky(root, false); // no-op
    TS_ASSERT(!testee.isSticky());
    TS_ASSERT(f.topics().contains(testee.getId()));
    TS_ASSERT(!f.stickyTopics().contains(testee.getId()));

    // Describe
    server::interface::TalkThread::Info i = testee.describe();
    TS_ASSERT_EQUALS(i.subject, "subj");
    TS_ASSERT_EQUALS(i.forumId, 9);
    TS_ASSERT_EQUALS(i.firstPostId, 120);
    TS_ASSERT_EQUALS(i.lastPostId, 121);
    TS_ASSERT_EQUALS(i.lastTime, 191919);
    TS_ASSERT_EQUALS(i.isSticky, false);

        // class TopicSorter : public Sorter {
        //  public:
        //     TopicSorter(Root& root);
        //     virtual void applySortKey(afl::net::redis::SortOperation& op, const String_t& keyName) const;
        //  private:
        //     Root& m_root;
        // };

}

/** Test removal. */
void
TestServerTalkTopic::testRemove()
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
        TS_ASSERT(!f.topics().contains(TOPIC_ID));
        TS_ASSERT(!f.stickyTopics().contains(TOPIC_ID));
        TS_ASSERT(!f.messages().contains(MESSAGE1_ID));
        TS_ASSERT(!f.messages().contains(MESSAGE2_ID));
        TS_ASSERT(!t.exists());
        TS_ASSERT(!m1.exists());
        TS_ASSERT(!m2.exists());
    }
}

/** Test sorting. */
void
TestServerTalkTopic::testSort()
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
        TS_ASSERT_EQUALS(result.size(), 5U);
        TS_ASSERT_EQUALS(result[0], 100);
    }
    {
        IntegerList_t result;
        SortOperation op(key.sort());
        server::talk::Topic::TopicSorter(root).applySortKey(op, "FIRSTPOST");
        op.getResult(result);
        TS_ASSERT_EQUALS(result.size(), 5U);
        TS_ASSERT_EQUALS(result[0], 101);
    }
    {
        IntegerList_t result;
        SortOperation op(key.sort());
        server::talk::Topic::TopicSorter(root).applySortKey(op, "LASTPOST");
        op.getResult(result);
        TS_ASSERT_EQUALS(result.size(), 5U);
        TS_ASSERT_EQUALS(result[0], 102);
    }
    {
        IntegerList_t result;
        SortOperation op(key.sort());
        server::talk::Topic::TopicSorter(root).applySortKey(op, "FORUM");
        op.getResult(result);
        TS_ASSERT_EQUALS(result.size(), 5U);
        TS_ASSERT_EQUALS(result[0], 103);
    }
    {
        IntegerList_t result;
        SortOperation op(key.sort());
        server::talk::Topic::TopicSorter(root).applySortKey(op, "LASTTIME");
        op.getResult(result);
        TS_ASSERT_EQUALS(result.size(), 5U);
        TS_ASSERT_EQUALS(result[0], 104);
    }

    // Error cases
    {
        SortOperation op(key.sort());
        TS_ASSERT_THROWS(server::talk::Topic::TopicSorter(root).applySortKey(op, "lasttime"), std::exception);
        TS_ASSERT_THROWS(server::talk::Topic::TopicSorter(root).applySortKey(op, ""), std::exception);
        TS_ASSERT_THROWS(server::talk::Topic::TopicSorter(root).applySortKey(op, "WHATEVER"), std::exception);
    }
}

