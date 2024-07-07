/**
  *  \file test/server/talk/talkposttest.cpp
  *  \brief Test for server::talk::TalkPost
  */

#include "server/talk/talkpost.hpp"

#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/test/commandhandler.hpp"
#include "afl/test/testrunner.hpp"
#include "server/talk/forum.hpp"
#include "server/talk/message.hpp"
#include "server/talk/root.hpp"
#include "server/talk/session.hpp"
#include "server/talk/topic.hpp"
#include "server/talk/user.hpp"

#include "spamtest.hpp"

using afl::net::NullCommandHandler;
using afl::net::redis::InternalDatabase;
using server::talk::Configuration;
using server::talk::Forum;
using server::talk::Root;
using server::talk::Session;
using server::talk::TalkPost;
using server::talk::Topic;
using server::talk::User;

/** Test create(), regular case, including notification. */
AFL_TEST("server.talk.TalkPost:create", a)
{
    // Infrastructure
    afl::test::CommandHandler mq(a);
    InternalDatabase db;
    Root root(db, mq, Configuration());
    Session session;

    // Set up database
    // - make a forum
    const int32_t FORUM_ID = 42;
    root.allForums().add(FORUM_ID);
    Forum f(root, FORUM_ID);
    f.name().set("Foorum");
    f.writePermissions().set("all");
    f.readPermissions().set("all");

    // - make a user who watches the forum
    User userA(root, "a");
    userA.watchedForums().add(FORUM_ID);
    f.watchers().add("a");

    // - make another user who watches the forum
    User userB(root, "b");
    userB.watchedForums().add(FORUM_ID);
    f.watchers().add("b");

    // - finally a user user who watches the forum but was already notified
    User userC(root, "c");
    userC.watchedForums().add(FORUM_ID);
    userC.notifiedForums().add(FORUM_ID);
    userC.profile().intField("talkwatchindividual").set(0);
    f.watchers().add("c");

    // Write a posting as user "b".
    // This must create a message to "a" (because b is the author and c is already notified).
    mq.expectCall("MAIL, talk-forum");
    mq.provideNewResult(0);
    mq.expectCall("PARAM, forum, Foorum");
    mq.provideNewResult(0);
    mq.expectCall("PARAM, subject, subj");
    mq.provideNewResult(0);
    mq.expectCall("PARAM, posturl, talk/thread.cgi/1-subj#p1");
    mq.provideNewResult(0);
    mq.expectCall("SEND, user:a");
    mq.provideNewResult(0);

    session.setUser("b");
    TalkPost testee(session, root);
    int32_t i = testee.create(FORUM_ID, "subj", "forum:text", TalkPost::CreateOptions());

    a.checkDifferent("01. create", i, 0);
    server::talk::Message msg(root, i);
    a.checkEqual("02. subj", msg.subject().get(), "subj");
    a.checkEqual("03. text", msg.text().get(), "forum:text");

    mq.checkFinish();
}

/** Test create(), error cases. */
AFL_TEST("server.talk.TalkPost:create:error", a)
{
    // Infrastructure
    NullCommandHandler mq;
    InternalDatabase db;
    Root root(db, mq, Configuration());
    Session session;

    // Set up database
    // - make a forum
    const int32_t FORUM_ID = 42;
    root.allForums().add(FORUM_ID);
    Forum f(root, FORUM_ID);
    f.name().set("Foorum");
    f.writePermissions().set("all");
    f.readPermissions().set("all");

    // Testee
    TalkPost testee(session, root);

    // Error: posting from admin context without USER
    AFL_CHECK_THROWS(a("01. no user"), testee.create(FORUM_ID, "subj", "text", TalkPost::CreateOptions()), std::exception);

    // Error: posting from user context with USER
    session.setUser("a");
    {
        TalkPost::CreateOptions opts;
        opts.userId = "u";
        AFL_CHECK_THROWS(a("11. user change"), testee.create(FORUM_ID, "subj", "text", opts), std::exception);
    }

    // Error: posting into nonexistant forum
    session.setUser("a");
    AFL_CHECK_THROWS(a("21. bad forum"), testee.create(FORUM_ID+1, "subj", "text", TalkPost::CreateOptions()), std::exception);
}

/** Test create(), forbidden. */
AFL_TEST("server.talk.TalkPost:create:forbidden", a)
{
    // Infrastructure
    NullCommandHandler mq;
    InternalDatabase db;
    Root root(db, mq, Configuration());
    Session session;

    // Set up database
    // - make a forum
    const int32_t FORUM_ID = 42;
    root.allForums().add(FORUM_ID);
    Forum f(root, FORUM_ID);
    f.name().set("Foorum");
    f.writePermissions().set("all");
    f.readPermissions().set("all");

    // Make a user
    User user(root, "b");
    user.profile().intField("allowpost").set(0);

    session.setUser("b");
    TalkPost testee(session, root);
    AFL_CHECK_THROWS(a("01. create"), testee.create(FORUM_ID, "subj", "forum:text", TalkPost::CreateOptions()), std::exception);
}

/** Test create(), globally forbidden. */
AFL_TEST("server.talk.TalkPost:create:forbidden:global", a)
{
    // Infrastructure
    NullCommandHandler mq;
    InternalDatabase db;
    Root root(db, mq, Configuration());
    Session session;

    // Set up database
    // - make a forum
    const int32_t FORUM_ID = 42;
    root.allForums().add(FORUM_ID);
    Forum f(root, FORUM_ID);
    f.name().set("Foorum");
    f.writePermissions().set("all");
    f.readPermissions().set("all");

    // Make a user
    User user(root, "b");
    root.defaultProfile().intField("allowpost").set(0);

    session.setUser("b");
    TalkPost testee(session, root);
    AFL_CHECK_THROWS(a("01. create"), testee.create(FORUM_ID, "subj", "forum:text", TalkPost::CreateOptions()), std::exception);
}

/** Test create(), spam case. */
AFL_TEST("server.talk.TalkPost:create:spam", a)
{
    // Infrastructure
    NullCommandHandler mq;
    InternalDatabase db;
    Root root(db, mq, Configuration());
    Session session;

    // Set up database
    // - make a forum
    const int32_t FORUM_ID = 42;
    root.allForums().add(FORUM_ID);
    Forum f(root, FORUM_ID);
    f.name().set("Foorum");
    f.writePermissions().set("all");
    f.readPermissions().set("all");

    // - make a user
    User u(root, "a");
    u.profile().stringField("createacceptlanguage").set("zh_ZH");
    u.profile().intField("createtime").set(60*root.getTime() - 1);                 // seconds, not minutes in this field!

    // Testee
    TalkPost testee(session, root);
    session.setUser("a");
    int32_t id = testee.create(FORUM_ID, "subj", server::talk::SPAM_MESSAGE, TalkPost::CreateOptions());

    // Verify
    a.check("01. create", id > 0);
    int32_t topicId = server::talk::Message(root, id).topicId().get();
    a.checkEqual("02. spam", User(root, "a").profile().intField("spam").get(), 1);
    a.checkEqual("03. perm", Topic(root, topicId).readPermissions().get(), "p:spam");
}

/** Test reply(), forbidden. */
AFL_TEST("server.talk.TalkPost:reply:forbidden", a)
{
    // Infrastructure
    NullCommandHandler mq;
    InternalDatabase db;
    Root root(db, mq, Configuration());

    // Set up database
    // - make a forum
    const int32_t FORUM_ID = 42;
    root.allForums().add(FORUM_ID);
    Forum f(root, FORUM_ID);
    f.name().set("Foorum");
    f.writePermissions().set("all");
    f.readPermissions().set("all");

    // Create a posting
    int32_t postId;
    {
        Session session;
        session.setUser("a");
        TalkPost testee(session, root);
        postId = testee.create(FORUM_ID, "subj", "forum:text", TalkPost::CreateOptions());
    }

    // Make a user that is forbidden
    User user(root, "b");
    user.profile().intField("allowpost").set(0);

    {
        Session session;
        session.setUser("b");
        TalkPost testee(session, root);
        AFL_CHECK_THROWS(a("01. reply"), testee.reply(postId, "reply", "forum:reply text", TalkPost::ReplyOptions()), std::exception);
    }
}

/** Test reply(), forbidden globally. */
AFL_TEST("server.talk.TalkPost:reply:forbidden:global", a)
{
    // Infrastructure
    NullCommandHandler mq;
    InternalDatabase db;
    Root root(db, mq, Configuration());

    // Set up database
    // - make a forum
    const int32_t FORUM_ID = 42;
    root.allForums().add(FORUM_ID);
    Forum f(root, FORUM_ID);
    f.name().set("Foorum");
    f.writePermissions().set("all");
    f.readPermissions().set("all");

    // Create a posting
    int32_t postId;
    {
        Session session;
        session.setUser("a");
        TalkPost testee(session, root);
        postId = testee.create(FORUM_ID, "subj", "forum:text", TalkPost::CreateOptions());
    }

    // Make a user that is forbidden
    User user(root, "b");
    root.defaultProfile().intField("allowpost").set(0);

    {
        Session session;
        session.setUser("b");
        TalkPost testee(session, root);
        AFL_CHECK_THROWS(a("01. reply"), testee.reply(postId, "reply", "forum:reply text", TalkPost::ReplyOptions()), std::exception);
    }
}

/** Test permissions in create(), reply(), edit(). */
AFL_TEST("server.talk.TalkPost:permissions", a)
{
    // Infrastructure
    NullCommandHandler mq;
    InternalDatabase db;
    Root root(db, mq, Configuration());

    // Set up database
    // - make a forum
    const int32_t FORUM_ID = 42;
    root.allForums().add(FORUM_ID);
    Forum f(root, FORUM_ID);
    f.name().set("Foorum");
    f.writePermissions().set("-u:b,all");
    f.readPermissions().set("all");

    // - Plain create fails because we didn't set a user yet
    {
        Session session;
        TalkPost testee(session, root);
        AFL_CHECK_THROWS(a("01. create"), testee.create(FORUM_ID, "subj", "text", TalkPost::CreateOptions()), std::runtime_error);
    }

    // - Normal posting (#1)
    {
        Session session;
        TalkPost testee(session, root);
        TalkPost::CreateOptions opts;
        opts.userId = "a";
        int32_t topicId = testee.create(FORUM_ID, "subj", "text:text", opts);
        a.checkEqual("11. create", topicId, 1);
        a.checkEqual("12. firstPostingId", Topic(root, topicId).firstPostingId().get(), topicId);
    }

    // - Normal posting with permissions (#2)
    {
        Session session;
        TalkPost testee(session, root);
        TalkPost::CreateOptions opts;
        opts.userId = "a";
        opts.answerPermissions = "all";
        int32_t topicId = testee.create(FORUM_ID, "subj", "text:text", opts);
        a.checkEqual("21. create", topicId, 2);
        a.checkEqual("22. firstPostingId", Topic(root, topicId).firstPostingId().get(), topicId);
    }

    // - Posting with implicit user permission (#3)
    {
        Session session;
        TalkPost testee(session, root);
        TalkPost::CreateOptions opts;
        session.setUser("a");
        int32_t topicId = testee.create(FORUM_ID, "subj", "text:text", opts);
        a.checkEqual("31. create", topicId, 3);
        a.checkEqual("32. firstPostingId", Topic(root, topicId).firstPostingId().get(), topicId);
    }

    // - Posting with conflicting user permission
    {
        Session session;
        TalkPost testee(session, root);
        TalkPost::CreateOptions opts;
        session.setUser("a");
        opts.userId = "b";
        AFL_CHECK_THROWS(a("41. perm"), testee.create(FORUM_ID, "subj", "text:text", opts), std::exception);
    }

    // - Posting with conflicting matching permission (#4)
    {
        Session session;
        TalkPost testee(session, root);
        TalkPost::CreateOptions opts;
        session.setUser("a");
        opts.userId = "a";
        int32_t topicId = testee.create(FORUM_ID, "subj", "text:text", opts);
        a.checkEqual("51. create", topicId, 4);
        a.checkEqual("52. firstPostingId", Topic(root, topicId).firstPostingId().get(), topicId);
    }

    // - Posting with disallowed user
    {
        Session session;
        TalkPost testee(session, root);
        TalkPost::CreateOptions opts;
        session.setUser("b");
        AFL_CHECK_THROWS(a("61. blocked"), testee.create(FORUM_ID, "subj", "text:text", opts), std::exception);
    }

    // - Posting with root permissions as disallowed user (#5): succeeds
    {
        Session session;
        TalkPost testee(session, root);
        TalkPost::CreateOptions opts;
        opts.userId = "b";
        int32_t topicId = testee.create(FORUM_ID, "subj", "text:text", opts);
        a.checkEqual("71. create", topicId, 5);
        a.checkEqual("72. firstPostingId", Topic(root, topicId).firstPostingId().get(), topicId);
    }

    /*
     *  At this point we have four postings authored by a and one authored by b.
     *  #2 has answer permissions set.
     */

    // - Reply to #1 as b (should fail)
    {
        Session session;
        TalkPost testee(session, root);
        TalkPost::ReplyOptions opts;
        session.setUser("b");
        AFL_CHECK_THROWS(a("81. reply"), testee.reply(1, "reply", "text:text", opts), std::exception);
    }

    // - Reply to #2 as b (should succeed due to thread permissions)
    {
        Session session;
        TalkPost testee(session, root);
        TalkPost::ReplyOptions opts;
        session.setUser("b");
        int32_t postId = testee.reply(2, "reply", "text:text", opts);
        a.checkEqual("91. reply", postId, 6);
    }

    // - Reply to #1 as b with root permissions (should work, root can do anything)
    {
        Session session;
        TalkPost testee(session, root);
        TalkPost::ReplyOptions opts;
        opts.userId = "b";
        int32_t postId = testee.reply(1, "reply", "text:text", opts);
        a.checkEqual("101. reply", postId, 7);
    }

    // - Reply to #1 as b with implicit+explicit permissions (should fail)
    {
        Session session;
        TalkPost testee(session, root);
        TalkPost::ReplyOptions opts;
        session.setUser("b");
        opts.userId = "b";
        AFL_CHECK_THROWS(a("111. reply"), testee.reply(1, "reply", "text:text", opts), std::exception);
    }

    // - Reply to #2 as b with different permissions (should fail)
    {
        Session session;
        TalkPost testee(session, root);
        TalkPost::ReplyOptions opts;
        session.setUser("b");
        opts.userId = "a";
        AFL_CHECK_THROWS(a("121. reply"), testee.reply(2, "reply", "text:text", opts), std::exception);
    }

    // - Reply to #1 with empty subject
    {
        Session session;
        TalkPost testee(session, root);
        TalkPost::ReplyOptions opts;
        opts.userId = "b";
        int32_t postId = testee.reply(1, "", "text:text", opts);
        a.checkEqual("131. reply", postId, 8);
        a.checkEqual("132. subj", server::talk::Message(root, postId).subject().get(), "subj");
    }

    // - Message not found
    {
        Session session;
        TalkPost testee(session, root);
        TalkPost::ReplyOptions opts;
        opts.userId = "b";
        AFL_CHECK_THROWS(a("141. reply"), testee.reply(999, "reply", "text:text", opts), std::exception);
    }

    // - No user context
    {
        Session session;
        TalkPost testee(session, root);
        TalkPost::ReplyOptions opts;
        AFL_CHECK_THROWS(a("151. reply"), testee.reply(1, "reply", "text:text", opts), std::exception);
    }

    /*
     *  Edit
     */

    // - Edit #1 as root (should succeed)
    {
        Session session;
        TalkPost testee(session, root);
        AFL_CHECK_SUCCEEDS(a("161. edit"), testee.edit(1, "reply", "text:text2"));
    }

    // - Edit #1 as a (should succeed)
    {
        Session session;
        TalkPost testee(session, root);
        session.setUser("a");
        AFL_CHECK_SUCCEEDS(a("171. edit"), testee.edit(1, "reply", "text:text3"));
    }

    // - Edit #1 as b (should fail)
    {
        Session session;
        TalkPost testee(session, root);
        session.setUser("b");
        AFL_CHECK_THROWS(a("181. edit"), testee.edit(1, "reply", "text:text4"), std::exception);
    }

    // - Message not found
    {
        Session session;
        TalkPost testee(session, root);
        AFL_CHECK_THROWS(a("191. edit"), testee.edit(999, "reply", "text:text4"), std::exception);
    }
}

/** Test rendering. */
AFL_TEST("server.talk.TalkPost:render", a)
{
    // Infrastructure
    NullCommandHandler mq;
    InternalDatabase db;
    Root root(db, mq, Configuration());

    // Set up database
    // - make a forum
    const int32_t FORUM_ID = 42;
    root.allForums().add(FORUM_ID);
    Forum f(root, FORUM_ID);
    f.name().set("Foorum");
    f.writePermissions().set("all");
    f.readPermissions().set("-u:b,all");

    // Initial postings
    {
        Session session;
        TalkPost testee(session, root);
        TalkPost::CreateOptions opts;
        session.setUser("a");
        testee.create(FORUM_ID, "subj", "text:text", opts);
    }
    {
        Session session;
        TalkPost testee(session, root);
        TalkPost::CreateOptions opts;
        opts.readPermissions = "all";
        session.setUser("a");
        testee.create(FORUM_ID, "subj", "text:text2", opts);
    }

    // Render as root
    {
        Session session;
        TalkPost testee(session, root);
        session.renderOptions().setFormat("html");
        a.checkEqual("01. admin", testee.render(1, server::interface::TalkRender::Options()), "<p>text</p>\n");
    }

    // Render as user a, as HTML
    {
        Session session;
        TalkPost testee(session, root);
        session.setUser("a");
        session.renderOptions().setFormat("html");
        a.checkEqual("11. user html", testee.render(1, server::interface::TalkRender::Options()), "<p>text</p>\n");
    }

    // Render as user a, as plain-text with per-operation override
    {
        Session session;
        TalkPost testee(session, root);
        server::interface::TalkRender::Options opts;
        session.setUser("a");
        session.renderOptions().setFormat("html");
        opts.format = "text";
        a.checkEqual("21. user text", testee.render(1, opts), "text");
        a.checkEqual("22. option", session.renderOptions().getFormat(), "html");
    }

    // Render as user b, as HTML (permission denied)
    {
        Session session;
        TalkPost testee(session, root);
        session.setUser("b");
        session.renderOptions().setFormat("html");
        AFL_CHECK_THROWS(a("31. error"), testee.render(1, server::interface::TalkRender::Options()), std::runtime_error);
    }

    // Render as user b, as HTML (succeeds due to per-thread permissions)
    {
        Session session;
        TalkPost testee(session, root);
        session.setUser("b");
        session.renderOptions().setFormat("html");
        a.checkEqual("41. html", testee.render(2, server::interface::TalkRender::Options()), "<p>text2</p>\n");
    }

    // Render non-existant
    {
        Session session;
        TalkPost testee(session, root);
        session.renderOptions().setFormat("html");
        AFL_CHECK_THROWS(a("51. error"), testee.render(999, server::interface::TalkRender::Options()), std::runtime_error);
    }

    // Multi-render as a
    {
        Session session;
        TalkPost testee(session, root);
        session.setUser("a");
        session.renderOptions().setFormat("html");
        static const int32_t IDs[] = {1,2};
        afl::data::StringList_t result;
        AFL_CHECK_SUCCEEDS(a("61. render"), testee.render(IDs, result));
        a.checkEqual("62. size", result.size(), 2U);
        a.checkEqual("63. result", result[0], "<p>text</p>\n");
        a.checkEqual("64. result", result[1], "<p>text2</p>\n");
    }

    // Multi-render as b
    {
        Session session;
        TalkPost testee(session, root);
        session.setUser("b");
        session.renderOptions().setFormat("html");
        static const int32_t IDs[] = {1,2};
        afl::data::StringList_t result;
        AFL_CHECK_SUCCEEDS(a("71. render"), testee.render(IDs, result));
        a.checkEqual("72. size", result.size(), 2U);
        a.checkEqual("73. result", result[0], "");                 // inaccessible
        a.checkEqual("74. result", result[1], "<p>text2</p>\n");
    }

    // Multi-render nonexistant as root
    {
        Session session;
        TalkPost testee(session, root);
        session.setUser("a");
        session.renderOptions().setFormat("html");
        static const int32_t IDs[] = {1,4,2,3};
        afl::data::StringList_t result;
        AFL_CHECK_SUCCEEDS(a("81. render"), testee.render(IDs, result));
        a.checkEqual("82. size", result.size(), 4U);
        a.checkEqual("83. result", result[0], "<p>text</p>\n");
        a.checkEqual("84. result", result[1], "");
        a.checkEqual("85. result", result[2], "<p>text2</p>\n");
        a.checkEqual("86. result", result[3], "");
    }
}

/** Test getInfo. */
AFL_TEST("server.talk.TalkPost:getInfo", a)
{
    // Infrastructure
    NullCommandHandler mq;
    InternalDatabase db;
    Root root(db, mq, Configuration());

    // Set up database
    // - make a forum
    const int32_t FORUM_ID = 42;
    root.allForums().add(FORUM_ID);
    Forum f(root, FORUM_ID);
    f.name().set("Foorum");
    f.writePermissions().set("all");
    f.readPermissions().set("-u:b,all");

    // Initial postings
    {
        Session session;
        TalkPost testee(session, root);
        TalkPost::CreateOptions opts;
        session.setUser("a");
        testee.create(FORUM_ID, "subj", "text:text", opts);
    }
    {
        Session session;
        TalkPost testee(session, root);
        TalkPost::CreateOptions opts;
        opts.readPermissions = "all";
        session.setUser("a");
        testee.create(FORUM_ID, "subj", "text:text2", opts);
    }

    // Get information as root
    {
        Session session;
        TalkPost testee(session, root);
        TalkPost::Info i = testee.getInfo(1);
        a.checkEqual("01. threadId",     i.threadId, 1);
        a.checkEqual("02. parentPostId", i.parentPostId, 0);
        a.checkEqual("03. author",       i.author, "a");
        a.checkEqual("04. subject",      i.subject, "subj");
    }

    // Get information as "a"
    {
        Session session;
        TalkPost testee(session, root);
        session.setUser("a");
        TalkPost::Info i = testee.getInfo(1);
        a.checkEqual("11. threadId",     i.threadId, 1);
        a.checkEqual("12. parentPostId", i.parentPostId, 0);
        a.checkEqual("13. author",       i.author, "a");
        a.checkEqual("14. subject",      i.subject, "subj");
    }

    // Get information as "b"
    {
        Session session;
        TalkPost testee(session, root);
        session.setUser("b");
        AFL_CHECK_THROWS(a("21. getInfo"), testee.getInfo(1), std::exception);
    }

    // Get information as "b" for post 2
    {
        Session session;
        TalkPost testee(session, root);
        session.setUser("b");
        TalkPost::Info i = testee.getInfo(2);
        a.checkEqual("31. threadId",     i.threadId, 2);
        a.checkEqual("32. parentPostId", i.parentPostId, 0);
        a.checkEqual("33. author",       i.author, "a");
        a.checkEqual("34. subject",      i.subject, "subj");
    }

    // Multi-get information as a
    {
        Session session;
        TalkPost testee(session, root);
        session.setUser("a");
        static const int32_t IDs[] = {1,2};
        afl::container::PtrVector<TalkPost::Info> infos;
        AFL_CHECK_SUCCEEDS(a("41. getInfo"), testee.getInfo(IDs, infos));
        a.checkEqual  ("42. size", infos.size(), 2U);
        a.checkNonNull("43. result", infos[0]);
        a.checkNonNull("44. result", infos[1]);
        a.checkEqual  ("45. threadId", infos[0]->threadId, 1);
        a.checkEqual  ("46. threadId", infos[1]->threadId, 2);
    }

    // Multi-get information as b
    {
        Session session;
        TalkPost testee(session, root);
        session.setUser("b");
        static const int32_t IDs[] = {1,3,2};
        afl::container::PtrVector<TalkPost::Info> infos;
        AFL_CHECK_SUCCEEDS(a("51. getInfo"), testee.getInfo(IDs, infos));
        a.checkEqual  ("52. size", infos.size(), 3U);
        a.checkNull   ("53. result", infos[0]);
        a.checkNull   ("54. result", infos[1]);
        a.checkNonNull("55. result", infos[2]);
        a.checkEqual  ("56. threadId", infos[2]->threadId, 2);
    }

    // Multi-get information as root
    {
        Session session;
        TalkPost testee(session, root);
        static const int32_t IDs[] = {1,2};
        afl::container::PtrVector<TalkPost::Info> infos;
        AFL_CHECK_SUCCEEDS(a("61. render"), testee.getInfo(IDs, infos));
        a.checkEqual  ("62. size", infos.size(), 2U);
        a.checkNonNull("63. result", infos[0]);
        a.checkNonNull("64. result", infos[1]);
    }

    // Get information for nonexistant
    {
        Session session;
        TalkPost testee(session, root);
        AFL_CHECK_THROWS(a("71. error"), testee.getInfo(99), std::exception);
    }
}

/** Test getNewest. */
AFL_TEST("server.talk.TalkPost:getNewest", a)
{
    // Infrastructure
    NullCommandHandler mq;
    InternalDatabase db;
    Configuration config;
    config.rateCostPerPost = 0;
    Root root(db, mq, config);

    // Set up database
    // - make a forum
    const int32_t FORUM_ID = 42;
    root.allForums().add(FORUM_ID);
    Forum f(root, FORUM_ID);
    f.name().set("Foorum");
    f.writePermissions().set("all");
    f.readPermissions().set("-u:b,all");

    // Initial postings
    for (int i = 0; i < 100; ++i) {
        // 1, 3, 5, 7, ...., 199: public
        // 2,4,6,8, ..., 200: non-public
        Session session;
        TalkPost testee(session, root);
        {
            TalkPost::CreateOptions opts;
            opts.readPermissions = "all";
            opts.userId = "a";
            testee.create(FORUM_ID, "subj", "text:text", opts);
        }
        {
            TalkPost::CreateOptions opts;
            opts.userId = "a";
            testee.create(FORUM_ID, "subj", "text:text", opts);
        }
    }

    // List as root
    {
        Session session;
        TalkPost testee(session, root);
        afl::data::IntegerList_t result;
        testee.getNewest(5, result);
        a.checkEqual("01. size", result.size(), 5U);
        a.checkEqual("02. result", result[0], 200);
        a.checkEqual("03. result", result[1], 199);
        a.checkEqual("04. result", result[2], 198);
        a.checkEqual("05. result", result[3], 197);
        a.checkEqual("06. result", result[4], 196);
    }

    // List as 'b' who sees only the odd ones
    {
        Session session;
        TalkPost testee(session, root);
        session.setUser("b");
        afl::data::IntegerList_t result;
        testee.getNewest(5, result);
        a.checkEqual("11. size", result.size(), 5U);
        a.checkEqual("12. result", result[0], 199);
        a.checkEqual("13. result", result[1], 197);
        a.checkEqual("14. result", result[2], 195);
        a.checkEqual("15. result", result[3], 193);
        a.checkEqual("16. result", result[4], 191);
    }
}

/** Test getNewest, with limit config. */
AFL_TEST("server.talk.TalkPost:getNewest:limit", a)
{
    // Infrastructure
    NullCommandHandler mq;
    InternalDatabase db;
    Configuration config;
    config.rateCostPerPost = 0;
    config.getNewestLimit = 5;
    Root root(db, mq, config);

    // Set up database
    // - make a forum
    const int32_t FORUM_ID = 42;
    root.allForums().add(FORUM_ID);
    Forum f(root, FORUM_ID);
    f.name().set("Foorum");
    f.writePermissions().set("all");
    f.readPermissions().set("-u:b,all");

    // Initial postings
    for (int i = 0; i < 100; ++i) {
        // 1, 3, 5, 7, ...., 199: public
        // 2,4,6,8, ..., 200: non-public
        Session session;
        TalkPost testee(session, root);
        {
            TalkPost::CreateOptions opts;
            opts.readPermissions = "all";
            opts.userId = "a";
            testee.create(FORUM_ID, "subj", "text:text", opts);
        }
        {
            TalkPost::CreateOptions opts;
            opts.userId = "a";
            testee.create(FORUM_ID, "subj", "text:text", opts);
        }
    }

    // List as 'b' who sees only the odd ones
    // Limit is set to 5, so we check 196..200.
    {
        Session session;
        TalkPost testee(session, root);
        session.setUser("b");
        afl::data::IntegerList_t result;
        testee.getNewest(5, result);
        a.checkEqual("01. size", result.size(), 2U);
        a.checkEqual("02. result", result[0], 199);
        a.checkEqual("03. result", result[1], 197);
    }
}

/** Test getNewest() for a user who cannot see anything. */
AFL_TEST("server.talk.TalkPost:getNewest:invisible", a)
{
    // Infrastructure
    NullCommandHandler mq;
    InternalDatabase db;
    Configuration config;
    config.rateCostPerPost = 0;
    Root root(db, mq, config);

    // Set up database
    // - make a forum
    const int32_t FORUM_ID = 42;
    root.allForums().add(FORUM_ID);
    Forum f(root, FORUM_ID);
    f.name().set("Foorum");
    f.readPermissions().set("u:a");

    // Initial postings
    for (int i = 0; i < 1000; ++i) {
        Session session;
        TalkPost testee(session, root);
        TalkPost::CreateOptions opts;
        opts.userId = "b";
        testee.create(FORUM_ID, "subj", "text:text", opts);
    }

    // List as root
    {
        Session session;
        TalkPost testee(session, root);
        afl::data::IntegerList_t result;
        testee.getNewest(5, result);
        a.checkEqual("01. size", result.size(), 5U);
    }

    // List as 'a' who can see everything because he can read the forum
    {
        Session session;
        TalkPost testee(session, root);
        session.setUser("a");
        afl::data::IntegerList_t result;
        testee.getNewest(5, result);
        a.checkEqual("11. size", result.size(), 5U);
    }

    // List as 'b' who can see everything because he wrote it
    {
        Session session;
        TalkPost testee(session, root);
        session.setUser("a");
        afl::data::IntegerList_t result;
        testee.getNewest(5, result);
        a.checkEqual("21. size", result.size(), 5U);
    }

    // List as 'c' who cannot see anything
    {
        Session session;
        TalkPost testee(session, root);
        session.setUser("c");
        afl::data::IntegerList_t result;
        testee.getNewest(5, result);
        a.checkEqual("31. size", result.size(), 0U);
    }
}

/** Test getHeaderField(). */
AFL_TEST("server.talk.TalkPost:getHeaderField", a)
{
    // Infrastructure
    NullCommandHandler mq;
    InternalDatabase db;
    Configuration config;
    config.messageIdSuffix = "@suf";
    Root root(db, mq, config);

    // Set up database
    // - make a forum
    const int32_t FORUM_ID = 42;
    root.allForums().add(FORUM_ID);
    Forum f(root, FORUM_ID);
    f.name().set("Foorum");
    f.writePermissions().set("all");
    f.readPermissions().set("-u:b,all");

    // A posting and a reply
    {
        Session session;
        TalkPost testee(session, root);
        session.setUser("a");
        int32_t postId = testee.create(FORUM_ID, "subj", "text:text", TalkPost::CreateOptions());
        a.checkEqual("01. create", postId, 1);

        session.setUser("b");
        int32_t replyId = testee.reply(1, "reply", "text:text2", TalkPost::ReplyOptions());
        a.checkEqual("11. reply", replyId, 2);
    }

    // Tests as root
    {
        Session session;
        TalkPost testee(session, root);
        a.checkEqual("21. thread",   testee.getHeaderField(1, "thread"), "1");
        a.checkEqual("22. subject",  testee.getHeaderField(1, "subject"), "subj");
        a.checkEqual("23. author",   testee.getHeaderField(1, "author"), "a");
        a.checkEqual("24. rfcmsgid", testee.getHeaderField(1, "rfcmsgid"), "1.1@suf");

        a.checkEqual("31. thread",   testee.getHeaderField(2, "thread"), "1");
        a.checkEqual("32. subject",  testee.getHeaderField(2, "subject"), "reply");
        a.checkEqual("33. author",   testee.getHeaderField(2, "author"), "b");
        a.checkEqual("34. rfcmsgid", testee.getHeaderField(2, "rfcmsgid"), "2.2@suf");

        AFL_CHECK_THROWS(a("41. thread"), testee.getHeaderField(99, "thread"), std::exception);
    }

    // Tests as 'b': can only see post 2
    {
        Session session;
        TalkPost testee(session, root);
        session.setUser("b");
        AFL_CHECK_THROWS(a("51. thread"),   testee.getHeaderField(1, "thread"), std::exception);
        AFL_CHECK_THROWS(a("52. rfcmsgid"), testee.getHeaderField(1, "rfcmsgid"), std::exception);

        a.checkEqual("61. thread",   testee.getHeaderField(2, "thread"), "1");
        a.checkEqual("62. subject",  testee.getHeaderField(2, "subject"), "reply");
        a.checkEqual("63. author",   testee.getHeaderField(2, "author"), "b");
        a.checkEqual("64. rfcmsgid", testee.getHeaderField(2, "rfcmsgid"), "2.2@suf");

        AFL_CHECK_THROWS(a("71. thread"), testee.getHeaderField(99, "thread"), std::exception);
    }
}

/** Test remove(). */
AFL_TEST("server.talk.TalkPost:remove", a)
{
    // Infrastructure
    NullCommandHandler mq;
    InternalDatabase db;
    Root root(db, mq, Configuration());

    // Set up database
    // - make a forum
    const int32_t FORUM_ID = 42;
    root.allForums().add(FORUM_ID);
    Forum f(root, FORUM_ID);
    f.name().set("Foorum");
    f.writePermissions().set("all");

    // A posting and a reply
    {
        Session session;
        TalkPost testee(session, root);
        session.setUser("a");
        int32_t postId = testee.create(FORUM_ID, "subj", "text:text", TalkPost::CreateOptions());
        a.checkEqual("01. create", postId, 1);

        session.setUser("b");
        int32_t replyId = testee.reply(1, "reply", "text:text2", TalkPost::ReplyOptions());
        a.checkEqual("11. reply", replyId, 2);
    }

    // Remove first posting as root
    {
        Session session;
        TalkPost testee(session, root);
        a.checkEqual("21. remove", testee.remove(1), 1);
        a.check("22", !server::talk::Message(root, 1).exists());
        a.check("23",  Topic(root, 1).exists());
        a.check("24", !Topic(root, 1).messages().contains(1));
        a.check("25",  Topic(root, 1).messages().contains(2));
        a.check("26", !Forum(root, FORUM_ID).messages().contains(1));
        a.check("27",  Forum(root, FORUM_ID).messages().contains(2));
    }

    // Try to remove second posting as 'a': should fail
    {
        Session session;
        TalkPost testee(session, root);
        session.setUser("a");
        AFL_CHECK_THROWS(a("31. remove"), testee.remove(2), std::exception);
        a.check("32",  server::talk::Message(root, 2).exists());
        a.check("33",  Topic(root, 1).exists());
        a.check("34", !Topic(root, 1).messages().contains(1));
        a.check("35",  Topic(root, 1).messages().contains(2));
        a.check("36", !Forum(root, FORUM_ID).messages().contains(1));
        a.check("37",  Forum(root, FORUM_ID).messages().contains(2));
    }

    // Try to remove second posting as 'b' (=owner)
    {
        Session session;
        TalkPost testee(session, root);
        session.setUser("b");
        a.checkEqual("41. remove", testee.remove(2), 1);
        a.check("42", !server::talk::Message(root, 2).exists());
        a.check("43", !Topic(root, 1).exists());
        a.check("44", !Forum(root, FORUM_ID).messages().contains(1));
        a.check("45", !Forum(root, FORUM_ID).messages().contains(2));
    }

    // Remove nonexistant
    {
        Session session;
        TalkPost testee(session, root);
        a.checkEqual("51. remove", testee.remove(1), 0);
        a.checkEqual("52. remove", testee.remove(100), 0);
    }
}

/** Test rate limiting: a fresh user can send at least 5 messages, but not more than 50.
    Actual limit as of 20240706: 9 with default config. */
AFL_TEST("server.talk.TalkPM:ratelimit:create", a)
{
    // Infrastructure
    NullCommandHandler mq;
    InternalDatabase db;
    Root root(db, mq, Configuration());
    Session session;

    // Set up database with a forum
    const int32_t FORUM_ID = 42;
    root.allForums().add(FORUM_ID);
    Forum f(root, FORUM_ID);
    f.name().set("Foorum");
    f.writePermissions().set("all");
    f.readPermissions().set("all");

    session.setUser("a");
    TalkPost testee(session, root);

    int i = 0;
    while (i < 100) {
        try {
            testee.create(FORUM_ID, "subj", "text", TalkPost::CreateOptions());
        }
        catch (std::runtime_error& e) {
            break;
        }
        ++i;
    }

    a.checkGreaterEqual("count", i, 5);
    a.checkGreaterEqual("count", 50, i);
}

/** Test rate limiting: same thing, using reply(). */
AFL_TEST("server.talk.TalkPM:ratelimit:reply", a)
{
    // Infrastructure
    NullCommandHandler mq;
    InternalDatabase db;
    Root root(db, mq, Configuration());
    Session session;

    // Set up database with a forum
    const int32_t FORUM_ID = 42;
    root.allForums().add(FORUM_ID);
    Forum f(root, FORUM_ID);
    f.name().set("Foorum");
    f.writePermissions().set("all");
    f.readPermissions().set("all");

    session.setUser("a");
    TalkPost testee(session, root);

    int32_t msg = testee.create(FORUM_ID, "subj", "text", TalkPost::CreateOptions());

    int i = 1;
    while (i < 100) {
        try {
            testee.reply(msg, "subj", "text", TalkPost::ReplyOptions());
        }
        catch (std::runtime_error& e) {
            break;
        }
        ++i;
    }

    a.checkGreaterEqual("count", i, 5);
    a.checkGreaterEqual("count", 50, i);
}
