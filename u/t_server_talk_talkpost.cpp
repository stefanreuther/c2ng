/**
  *  \file u/t_server_talk_talkpost.cpp
  *  \brief Test for server::talk::TalkPost
  */

#include "server/talk/talkpost.hpp"

#include "t_server_talk.hpp"
#include "u/helper/commandhandlermock.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "server/talk/root.hpp"
#include "server/talk/session.hpp"
#include "server/talk/forum.hpp"
#include "server/talk/user.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "server/talk/message.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "server/talk/topic.hpp"

/** Test create(), regular case, including notification. */
void
TestServerTalkTalkPost::testCreate()
{
    // Infrastructure
    CommandHandlerMock mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, mq, server::talk::Configuration());
    server::talk::Session session;

    // Set up database
    // - make a forum
    const int32_t FORUM_ID = 42;
    root.allForums().add(FORUM_ID);
    server::talk::Forum f(root, FORUM_ID);
    f.name().set("Foorum");
    f.writePermissions().set("all");
    f.readPermissions().set("all");

    // - make a user who watches the forum
    server::talk::User userA(root, "a");
    userA.watchedForums().add(FORUM_ID);
    f.watchers().add("a");

    // - make another user who watches the forum
    server::talk::User userB(root, "b");
    userB.watchedForums().add(FORUM_ID);
    f.watchers().add("b");

    // - finally a user user who watches the forum but was already notified
    server::talk::User userC(root, "c");
    userC.watchedForums().add(FORUM_ID);
    userC.notifiedForums().add(FORUM_ID);
    userC.profile().intField("talkwatchindividual").set(0);
    f.watchers().add("c");

    // Write a posting as user "b".
    // This must create a message to "a" (because b is the author and c is already notified).
    mq.expectCall("MAIL|talk-forum");
    mq.provideReturnValue(0);
    mq.expectCall("PARAM|forum|Foorum");
    mq.provideReturnValue(0);
    mq.expectCall("PARAM|subject|subj");
    mq.provideReturnValue(0);
    mq.expectCall("PARAM|posturl|talk/thread.cgi/1-subj#p1");
    mq.provideReturnValue(0);
    mq.expectCall("SEND|user:a");
    mq.provideReturnValue(0);

    session.setUser("b");
    server::talk::TalkPost testee(session, root);
    int32_t i = testee.create(FORUM_ID, "subj", "forum:text", server::talk::TalkPost::CreateOptions());

    TS_ASSERT(i != 0);
    server::talk::Message msg(root, i);
    TS_ASSERT_EQUALS(msg.subject().get(), "subj");
    TS_ASSERT_EQUALS(msg.text().get(), "forum:text");

    mq.checkFinish();
}

/** Test create(), error cases. */
void
TestServerTalkTalkPost::testCreateErrors()
{
    // Infrastructure
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, mq, server::talk::Configuration());
    server::talk::Session session;

    // Set up database
    // - make a forum
    const int32_t FORUM_ID = 42;
    root.allForums().add(FORUM_ID);
    server::talk::Forum f(root, FORUM_ID);
    f.name().set("Foorum");
    f.writePermissions().set("all");
    f.readPermissions().set("all");

    // Testee
    server::talk::TalkPost testee(session, root);

    // Error: posting from admin context without USER
    TS_ASSERT_THROWS(testee.create(FORUM_ID, "subj", "text", server::talk::TalkPost::CreateOptions()), std::exception);

    // Error: posting from user context with USER
    session.setUser("a");
    {
        server::talk::TalkPost::CreateOptions opts;
        opts.userId = "u";
        TS_ASSERT_THROWS(testee.create(FORUM_ID, "subj", "text", opts), std::exception);
    }

    // Error: posting into nonexistant forum
    session.setUser("a");
    TS_ASSERT_THROWS(testee.create(FORUM_ID+1, "subj", "text", server::talk::TalkPost::CreateOptions()), std::exception);
}

/** Test create(), spam case. */
void
TestServerTalkTalkPost::testCreateSpam()
{
    // Infrastructure
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, mq, server::talk::Configuration());
    server::talk::Session session;

    // Set up database
    // - make a forum
    const int32_t FORUM_ID = 42;
    root.allForums().add(FORUM_ID);
    server::talk::Forum f(root, FORUM_ID);
    f.name().set("Foorum");
    f.writePermissions().set("all");
    f.readPermissions().set("all");

    // - make a user
    server::talk::User u(root, "a");
    u.profile().stringField("createacceptlanguage").set("zh_ZH");
    u.profile().intField("createtime").set(60*root.getTime() - 1);                 // seconds, not minutes in this field!

    // Testee
    server::talk::TalkPost testee(session, root);
    session.setUser("a");
    int32_t id = testee.create(FORUM_ID, "subj", TestServerTalkSpam::SPAM_MESSAGE, server::talk::TalkPost::CreateOptions());

    // Verify
    TS_ASSERT(id > 0);
    int32_t topicId = server::talk::Message(root, id).topicId().get();
    TS_ASSERT_EQUALS(server::talk::User(root, "a").profile().intField("spam").get(), 1);
    TS_ASSERT_EQUALS(server::talk::Topic(root, topicId).readPermissions().get(), "p:spam");
}

/** Test permissions in create(), reply(), edit(). */
void
TestServerTalkTalkPost::testPermissions()
{
    // Infrastructure
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, mq, server::talk::Configuration());

    // Set up database
    // - make a forum
    const int32_t FORUM_ID = 42;
    root.allForums().add(FORUM_ID);
    server::talk::Forum f(root, FORUM_ID);
    f.name().set("Foorum");
    f.writePermissions().set("-u:b,all");
    f.readPermissions().set("all");

    // - Plain create fails because we didn't set a user yet
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        TS_ASSERT_THROWS(testee.create(FORUM_ID, "subj", "text", server::talk::TalkPost::CreateOptions()), std::runtime_error);
    }

    // - Normal posting (#1)
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        server::talk::TalkPost::CreateOptions opts;
        opts.userId = "a";
        int32_t topicId = testee.create(FORUM_ID, "subj", "text:text", opts);
        TS_ASSERT_EQUALS(topicId, 1);
        TS_ASSERT_EQUALS(server::talk::Topic(root, topicId).firstPostingId().get(), topicId);
    }

    // - Normal posting with permissions (#2)
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        server::talk::TalkPost::CreateOptions opts;
        opts.userId = "a";
        opts.answerPermissions = "all";
        int32_t topicId = testee.create(FORUM_ID, "subj", "text:text", opts);
        TS_ASSERT_EQUALS(topicId, 2);
        TS_ASSERT_EQUALS(server::talk::Topic(root, topicId).firstPostingId().get(), topicId);
    }

    // - Posting with implicit user permission (#3)
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        server::talk::TalkPost::CreateOptions opts;
        session.setUser("a");
        int32_t topicId = testee.create(FORUM_ID, "subj", "text:text", opts);
        TS_ASSERT_EQUALS(topicId, 3);
        TS_ASSERT_EQUALS(server::talk::Topic(root, topicId).firstPostingId().get(), topicId);
    }

    // - Posting with conflicting user permission
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        server::talk::TalkPost::CreateOptions opts;
        session.setUser("a");
        opts.userId = "b";
        TS_ASSERT_THROWS(testee.create(FORUM_ID, "subj", "text:text", opts), std::exception);
    }

    // - Posting with conflicting matching permission (#4)
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        server::talk::TalkPost::CreateOptions opts;
        session.setUser("a");
        opts.userId = "a";
        int32_t topicId = testee.create(FORUM_ID, "subj", "text:text", opts);
        TS_ASSERT_EQUALS(topicId, 4);
        TS_ASSERT_EQUALS(server::talk::Topic(root, topicId).firstPostingId().get(), topicId);
    }

    // - Posting with disallowed user
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        server::talk::TalkPost::CreateOptions opts;
        session.setUser("b");
        TS_ASSERT_THROWS(testee.create(FORUM_ID, "subj", "text:text", opts), std::exception);
    }

    // - Posting with root permissions as disallowed user (#5): succeeds
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        server::talk::TalkPost::CreateOptions opts;
        opts.userId = "b";
        int32_t topicId = testee.create(FORUM_ID, "subj", "text:text", opts);
        TS_ASSERT_EQUALS(topicId, 5);
        TS_ASSERT_EQUALS(server::talk::Topic(root, topicId).firstPostingId().get(), topicId);
    }

    /*
     *  At this point we have four postings authored by a and one authored by b.
     *  #2 has answer permissions set.
     */

    // - Reply to #1 as b (should fail)
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        server::talk::TalkPost::ReplyOptions opts;
        session.setUser("b");
        TS_ASSERT_THROWS(testee.reply(1, "reply", "text:text", opts), std::exception);
    }

    // - Reply to #2 as b (should succeed due to thread permissions)
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        server::talk::TalkPost::ReplyOptions opts;
        session.setUser("b");
        int32_t postId = testee.reply(2, "reply", "text:text", opts);
        TS_ASSERT_EQUALS(postId, 6);
    }

    // - Reply to #1 as b with root permissions (should work, root can do anything)
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        server::talk::TalkPost::ReplyOptions opts;
        opts.userId = "b";
        int32_t postId = testee.reply(1, "reply", "text:text", opts);
        TS_ASSERT_EQUALS(postId, 7);
    }

    // - Reply to #1 as b with implicit+explicit permissions (should fail)
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        server::talk::TalkPost::ReplyOptions opts;
        session.setUser("b");
        opts.userId = "b";
        TS_ASSERT_THROWS(testee.reply(1, "reply", "text:text", opts), std::exception);
    }

    // - Reply to #2 as b with different permissions (should fail)
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        server::talk::TalkPost::ReplyOptions opts;
        session.setUser("b");
        opts.userId = "a";
        TS_ASSERT_THROWS(testee.reply(2, "reply", "text:text", opts), std::exception);
    }

    // - Reply to #1 with empty subject
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        server::talk::TalkPost::ReplyOptions opts;
        opts.userId = "b";
        int32_t postId = testee.reply(1, "", "text:text", opts);
        TS_ASSERT_EQUALS(postId, 8);
        TS_ASSERT_EQUALS(server::talk::Message(root, postId).subject().get(), "subj");
    }

    // - Message not found
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        server::talk::TalkPost::ReplyOptions opts;
        opts.userId = "b";
        TS_ASSERT_THROWS(testee.reply(999, "reply", "text:text", opts), std::exception);
    }

    // - No user context
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        server::talk::TalkPost::ReplyOptions opts;
        TS_ASSERT_THROWS(testee.reply(1, "reply", "text:text", opts), std::exception);
    }

    /*
     *  Edit
     */

    // - Edit #1 as root (should succeed)
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        TS_ASSERT_THROWS_NOTHING(testee.edit(1, "reply", "text:text2"));
    }

    // - Edit #1 as a (should fail)
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        session.setUser("a");
        TS_ASSERT_THROWS_NOTHING(testee.edit(1, "reply", "text:text3"));
    }

    // - Edit #1 as b (should fail)
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        session.setUser("b");
        TS_ASSERT_THROWS(testee.edit(1, "reply", "text:text4"), std::exception);
    }

    // - Message not found
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        TS_ASSERT_THROWS(testee.edit(999, "reply", "text:text4"), std::exception);
    }
}

/** Test rendering. */
void
TestServerTalkTalkPost::testRender()
{
    // Infrastructure
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, mq, server::talk::Configuration());

    // Set up database
    // - make a forum
    const int32_t FORUM_ID = 42;
    root.allForums().add(FORUM_ID);
    server::talk::Forum f(root, FORUM_ID);
    f.name().set("Foorum");
    f.writePermissions().set("all");
    f.readPermissions().set("-u:b,all");

    // Initial postings
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        server::talk::TalkPost::CreateOptions opts;
        session.setUser("a");
        testee.create(FORUM_ID, "subj", "text:text", opts);
    }
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        server::talk::TalkPost::CreateOptions opts;
        opts.readPermissions = "all";
        session.setUser("a");
        testee.create(FORUM_ID, "subj", "text:text2", opts);
    }

    // Render as root
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        session.renderOptions().setFormat("html");
        TS_ASSERT_EQUALS(testee.render(1, server::interface::TalkRender::Options()), "<p>text</p>\n");
    }

    // Render as user a, as HTML
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        session.setUser("a");
        session.renderOptions().setFormat("html");
        TS_ASSERT_EQUALS(testee.render(1, server::interface::TalkRender::Options()), "<p>text</p>\n");
    }

    // Render as user a, as plain-text with per-operation override
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        server::interface::TalkRender::Options opts;
        session.setUser("a");
        session.renderOptions().setFormat("html");
        opts.format = "text";
        TS_ASSERT_EQUALS(testee.render(1, opts), "text");
        TS_ASSERT_EQUALS(session.renderOptions().getFormat(), "html");
    }

    // Render as user b, as HTML (permission denied)
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        session.setUser("b");
        session.renderOptions().setFormat("html");
        TS_ASSERT_THROWS(testee.render(1, server::interface::TalkRender::Options()), std::runtime_error);
    }

    // Render as user b, as HTML (succeeds due to per-thread permissions)
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        session.setUser("b");
        session.renderOptions().setFormat("html");
        TS_ASSERT_EQUALS(testee.render(2, server::interface::TalkRender::Options()), "<p>text2</p>\n");
    }

    // Render non-existant
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        session.renderOptions().setFormat("html");
        TS_ASSERT_THROWS(testee.render(999, server::interface::TalkRender::Options()), std::runtime_error);
    }

    // Multi-render as a
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        session.setUser("a");
        session.renderOptions().setFormat("html");
        static const int32_t IDs[] = {1,2};
        afl::data::StringList_t result;
        TS_ASSERT_THROWS_NOTHING(testee.render(IDs, result));
        TS_ASSERT_EQUALS(result.size(), 2U);
        TS_ASSERT_EQUALS(result[0], "<p>text</p>\n");
        TS_ASSERT_EQUALS(result[1], "<p>text2</p>\n");
    }

    // Multi-render as b
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        session.setUser("b");
        session.renderOptions().setFormat("html");
        static const int32_t IDs[] = {1,2};
        afl::data::StringList_t result;
        TS_ASSERT_THROWS_NOTHING(testee.render(IDs, result));
        TS_ASSERT_EQUALS(result.size(), 2U);
        TS_ASSERT_EQUALS(result[0], "");                 // inaccessible
        TS_ASSERT_EQUALS(result[1], "<p>text2</p>\n");
    }

    // Multi-render nonexistant as root
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        session.setUser("a");
        session.renderOptions().setFormat("html");
        static const int32_t IDs[] = {1,4,2,3};
        afl::data::StringList_t result;
        TS_ASSERT_THROWS_NOTHING(testee.render(IDs, result));
        TS_ASSERT_EQUALS(result.size(), 4U);
        TS_ASSERT_EQUALS(result[0], "<p>text</p>\n");
        TS_ASSERT_EQUALS(result[1], "");
        TS_ASSERT_EQUALS(result[2], "<p>text2</p>\n");
        TS_ASSERT_EQUALS(result[3], "");
    }
}

/** Test getInfo. */
void
TestServerTalkTalkPost::testGetInfo()
{
    // Infrastructure
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, mq, server::talk::Configuration());

    // Set up database
    // - make a forum
    const int32_t FORUM_ID = 42;
    root.allForums().add(FORUM_ID);
    server::talk::Forum f(root, FORUM_ID);
    f.name().set("Foorum");
    f.writePermissions().set("all");
    f.readPermissions().set("-u:b,all");

    // Initial postings
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        server::talk::TalkPost::CreateOptions opts;
        session.setUser("a");
        testee.create(FORUM_ID, "subj", "text:text", opts);
    }
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        server::talk::TalkPost::CreateOptions opts;
        opts.readPermissions = "all";
        session.setUser("a");
        testee.create(FORUM_ID, "subj", "text:text2", opts);
    }

    // Get information as root
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        server::talk::TalkPost::Info i = testee.getInfo(1);
        TS_ASSERT_EQUALS(i.threadId, 1);
        TS_ASSERT_EQUALS(i.parentPostId, 0);
        TS_ASSERT_EQUALS(i.author, "a");
        TS_ASSERT_EQUALS(i.subject, "subj");
    }

    // Get information as "a"
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        session.setUser("a");
        server::talk::TalkPost::Info i = testee.getInfo(1);
        TS_ASSERT_EQUALS(i.threadId, 1);
        TS_ASSERT_EQUALS(i.parentPostId, 0);
        TS_ASSERT_EQUALS(i.author, "a");
        TS_ASSERT_EQUALS(i.subject, "subj");
    }

    // Get information as "b"
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        session.setUser("b");
        TS_ASSERT_THROWS(testee.getInfo(1), std::exception);
    }

    // Get information as "b" for post 2
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        session.setUser("b");
        server::talk::TalkPost::Info i = testee.getInfo(2);
        TS_ASSERT_EQUALS(i.threadId, 2);
        TS_ASSERT_EQUALS(i.parentPostId, 0);
        TS_ASSERT_EQUALS(i.author, "a");
        TS_ASSERT_EQUALS(i.subject, "subj");
    }

    // Multi-get information as a
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        session.setUser("a");
        static const int32_t IDs[] = {1,2};
        afl::container::PtrVector<server::talk::TalkPost::Info> infos;
        TS_ASSERT_THROWS_NOTHING(testee.getInfo(IDs, infos));
        TS_ASSERT_EQUALS(infos.size(), 2U);
        TS_ASSERT(infos[0] != 0);
        TS_ASSERT(infos[1] != 0);
        TS_ASSERT_EQUALS(infos[0]->threadId, 1);
        TS_ASSERT_EQUALS(infos[1]->threadId, 2);
    }

    // Multi-get information as b
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        session.setUser("b");
        static const int32_t IDs[] = {1,3,2};
        afl::container::PtrVector<server::talk::TalkPost::Info> infos;
        TS_ASSERT_THROWS_NOTHING(testee.getInfo(IDs, infos));
        TS_ASSERT_EQUALS(infos.size(), 3U);
        TS_ASSERT(infos[0] == 0);
        TS_ASSERT(infos[1] == 0);
        TS_ASSERT(infos[2] != 0);
        TS_ASSERT_EQUALS(infos[2]->threadId, 2);
    }

    // Multi-get information as root
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        static const int32_t IDs[] = {1,2};
        afl::container::PtrVector<server::talk::TalkPost::Info> infos;
        TS_ASSERT_THROWS_NOTHING(testee.getInfo(IDs, infos));
        TS_ASSERT_EQUALS(infos.size(), 2U);
        TS_ASSERT(infos[0] != 0);
        TS_ASSERT(infos[1] != 0);
    }

    // Get information for nonexistant
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        TS_ASSERT_THROWS(testee.getInfo(99), std::exception);
    }
}

/** Test getNewest. */
void
TestServerTalkTalkPost::testGetNewest()
{
    // Infrastructure
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, mq, server::talk::Configuration());

    // Set up database
    // - make a forum
    const int32_t FORUM_ID = 42;
    root.allForums().add(FORUM_ID);
    server::talk::Forum f(root, FORUM_ID);
    f.name().set("Foorum");
    f.writePermissions().set("all");
    f.readPermissions().set("-u:b,all");

    // Initial postings
    for (int i = 0; i < 100; ++i) {
        // 1, 3, 5, 7, ...., 199: public
        // 2,4,6,8, ..., 200: non-public
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        {
            server::talk::TalkPost::CreateOptions opts;
            opts.readPermissions = "all";
            opts.userId = "a";
            testee.create(FORUM_ID, "subj", "text:text", opts);
        }
        {
            server::talk::TalkPost::CreateOptions opts;
            opts.userId = "a";
            testee.create(FORUM_ID, "subj", "text:text", opts);
        }
    }

    // List as root
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        afl::data::IntegerList_t result;
        testee.getNewest(5, result);
        TS_ASSERT_EQUALS(result.size(), 5U);
        TS_ASSERT_EQUALS(result[0], 200);
        TS_ASSERT_EQUALS(result[1], 199);
        TS_ASSERT_EQUALS(result[2], 198);
        TS_ASSERT_EQUALS(result[3], 197);
        TS_ASSERT_EQUALS(result[4], 196);
    }

    // List as 'b' who sees only the odd ones
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        session.setUser("b");
        afl::data::IntegerList_t result;
        testee.getNewest(5, result);
        TS_ASSERT_EQUALS(result.size(), 5U);
        TS_ASSERT_EQUALS(result[0], 199);
        TS_ASSERT_EQUALS(result[1], 197);
        TS_ASSERT_EQUALS(result[2], 195);
        TS_ASSERT_EQUALS(result[3], 193);
        TS_ASSERT_EQUALS(result[4], 191);
    }
}

/** Test getNewest() for a user who cannot see anything. */
void
TestServerTalkTalkPost::testGetNewest2()
{
    // Infrastructure
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, mq, server::talk::Configuration());

    // Set up database
    // - make a forum
    const int32_t FORUM_ID = 42;
    root.allForums().add(FORUM_ID);
    server::talk::Forum f(root, FORUM_ID);
    f.name().set("Foorum");
    f.readPermissions().set("u:a");

    // Initial postings
    for (int i = 0; i < 1000; ++i) {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        server::talk::TalkPost::CreateOptions opts;
        opts.userId = "b";
        testee.create(FORUM_ID, "subj", "text:text", opts);
    }

    // List as root
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        afl::data::IntegerList_t result;
        testee.getNewest(5, result);
        TS_ASSERT_EQUALS(result.size(), 5U);
    }

    // List as 'a' who can see everything because he can read the forum
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        session.setUser("a");
        afl::data::IntegerList_t result;
        testee.getNewest(5, result);
        TS_ASSERT_EQUALS(result.size(), 5U);
    }

    // List as 'a' who can see everything because he can wrote it
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        session.setUser("a");
        afl::data::IntegerList_t result;
        testee.getNewest(5, result);
        TS_ASSERT_EQUALS(result.size(), 5U);
    }

    // List as 'c' who cannot see anything
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        session.setUser("c");
        afl::data::IntegerList_t result;
        testee.getNewest(5, result);
        TS_ASSERT_EQUALS(result.size(), 0U);
    }
}

/** Test getHeaderField(). */
void
TestServerTalkTalkPost::testGetHeader()
{
    // Infrastructure
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Configuration config;
    config.messageIdSuffix = "@suf";
    server::talk::Root root(db, mq, config);

    // Set up database
    // - make a forum
    const int32_t FORUM_ID = 42;
    root.allForums().add(FORUM_ID);
    server::talk::Forum f(root, FORUM_ID);
    f.name().set("Foorum");
    f.writePermissions().set("all");
    f.readPermissions().set("-u:b,all");

    // A posting and a reply
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        session.setUser("a");
        int32_t postId = testee.create(FORUM_ID, "subj", "text:text", server::talk::TalkPost::CreateOptions());
        TS_ASSERT_EQUALS(postId, 1);

        session.setUser("b");
        int32_t replyId = testee.reply(1, "reply", "text:text2", server::talk::TalkPost::ReplyOptions());
        TS_ASSERT_EQUALS(replyId, 2);
    }

    // Tests as root
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        TS_ASSERT_EQUALS(testee.getHeaderField(1, "thread"), "1");
        TS_ASSERT_EQUALS(testee.getHeaderField(1, "subject"), "subj");
        TS_ASSERT_EQUALS(testee.getHeaderField(1, "author"), "a");
        TS_ASSERT_EQUALS(testee.getHeaderField(1, "rfcmsgid"), "1.1@suf");

        TS_ASSERT_EQUALS(testee.getHeaderField(2, "thread"), "1");
        TS_ASSERT_EQUALS(testee.getHeaderField(2, "subject"), "reply");
        TS_ASSERT_EQUALS(testee.getHeaderField(2, "author"), "b");
        TS_ASSERT_EQUALS(testee.getHeaderField(2, "rfcmsgid"), "2.2@suf");

        TS_ASSERT_THROWS(testee.getHeaderField(99, "thread"), std::exception);
    }

    // Tests as 'b': can only see post 2
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        session.setUser("b");
        TS_ASSERT_THROWS(testee.getHeaderField(1, "thread"), std::exception);
        TS_ASSERT_THROWS(testee.getHeaderField(1, "rfcmsgid"), std::exception);

        TS_ASSERT_EQUALS(testee.getHeaderField(2, "thread"), "1");
        TS_ASSERT_EQUALS(testee.getHeaderField(2, "subject"), "reply");
        TS_ASSERT_EQUALS(testee.getHeaderField(2, "author"), "b");
        TS_ASSERT_EQUALS(testee.getHeaderField(2, "rfcmsgid"), "2.2@suf");

        TS_ASSERT_THROWS(testee.getHeaderField(99, "thread"), std::exception);
    }
}

/** Test remove(). */
void
TestServerTalkTalkPost::testRemove()
{
    // Infrastructure
    afl::net::NullCommandHandler mq;
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, mq, server::talk::Configuration());

    // Set up database
    // - make a forum
    const int32_t FORUM_ID = 42;
    root.allForums().add(FORUM_ID);
    server::talk::Forum f(root, FORUM_ID);
    f.name().set("Foorum");
    f.writePermissions().set("all");

    // A posting and a reply
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        session.setUser("a");
        int32_t postId = testee.create(FORUM_ID, "subj", "text:text", server::talk::TalkPost::CreateOptions());
        TS_ASSERT_EQUALS(postId, 1);

        session.setUser("b");
        int32_t replyId = testee.reply(1, "reply", "text:text2", server::talk::TalkPost::ReplyOptions());
        TS_ASSERT_EQUALS(replyId, 2);
    }

    // Remove first posting as root
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        TS_ASSERT_EQUALS(testee.remove(1), 1);
        TS_ASSERT(!server::talk::Message(root, 1).exists());
        TS_ASSERT( server::talk::Topic(root, 1).exists());
        TS_ASSERT(!server::talk::Topic(root, 1).messages().contains(1));
        TS_ASSERT( server::talk::Topic(root, 1).messages().contains(2));
        TS_ASSERT(!server::talk::Forum(root, FORUM_ID).messages().contains(1));
        TS_ASSERT( server::talk::Forum(root, FORUM_ID).messages().contains(2));
    }

    // Try to remove second posting as 'a': should fail
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        session.setUser("a");
        TS_ASSERT_THROWS(testee.remove(2), std::exception);
        TS_ASSERT( server::talk::Message(root, 2).exists());
        TS_ASSERT( server::talk::Topic(root, 1).exists());
        TS_ASSERT(!server::talk::Topic(root, 1).messages().contains(1));
        TS_ASSERT( server::talk::Topic(root, 1).messages().contains(2));
        TS_ASSERT(!server::talk::Forum(root, FORUM_ID).messages().contains(1));
        TS_ASSERT( server::talk::Forum(root, FORUM_ID).messages().contains(2));
    }

    // Try to remove second posting as 'b' (=owner)
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        session.setUser("b");
        TS_ASSERT_EQUALS(testee.remove(2), 1);
        TS_ASSERT(!server::talk::Message(root, 2).exists());
        TS_ASSERT(!server::talk::Topic(root, 1).exists());
        TS_ASSERT(!server::talk::Forum(root, FORUM_ID).messages().contains(1));
        TS_ASSERT(!server::talk::Forum(root, FORUM_ID).messages().contains(2));
    }

    // Remove nonexistant
    {
        server::talk::Session session;
        server::talk::TalkPost testee(session, root);
        TS_ASSERT_EQUALS(testee.remove(1), 0);
        TS_ASSERT_EQUALS(testee.remove(100), 0);
    }
}

