/**
  *  \file test/server/talk/notificationthreadtest.cpp
  *  \brief Test for server::talk::NotificationThread
  */

#include "server/talk/notificationthread.hpp"

#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/sys/thread.hpp"
#include "afl/test/commandhandler.hpp"
#include "afl/test/testrunner.hpp"
#include "server/interface/mailqueueclient.hpp"
#include "server/talk/configuration.hpp"
#include "server/talk/forum.hpp"
#include "server/talk/message.hpp"
#include "server/talk/root.hpp"
#include "server/talk/topic.hpp"
#include "server/talk/user.hpp"
#include "server/talk/userpm.hpp"
#include "server/test/mailmock.hpp"

using afl::net::NullCommandHandler;
using afl::net::redis::InternalDatabase;
using server::interface::MailQueueClient;
using server::talk::Configuration;
using server::talk::Forum;
using server::talk::Message;
using server::talk::NotificationThread;
using server::talk::Root;
using server::talk::Topic;
using server::talk::User;
using server::talk::UserPM;
using server::test::MailMock;

namespace {
    struct ForumEnvironment {
        // IDs
        static const int32_t forumId = 99;
        static const int32_t topicId = 42;
        static const int32_t postId = 123;

        // For this test, only one user.
        User bulkUser;
        Forum forum;
        Topic topic;
        Message post;

        ForumEnvironment(Root& root)
            : bulkUser(root, "b"),
              forum(root, forumId),
              topic(root, topicId),
              post(root, postId)
            {
                bulkUser.profile().intField("talkwatchindividual").set(0);
                bulkUser.watchedForums().add(forumId);
                root.userRoot().stringSetKey("all").add("b");

                // Create forum
                root.allForums().add(forumId);
                forum.creationTime().set(1);
                forum.header().stringField("name").set("Forum");
                forum.watchers().add("p");
                forum.watchers().add("b");
                forum.readPermissions().set("all");

                // Topic
                forum.topics().add(topicId);
                topic.subject().set("topic sub");
                topic.forumId().set(forumId);
                topic.firstPostingId().set(postId);

                // Post
                topic.messages().add(postId);
                forum.messages().add(postId);
                post.topicId().set(topicId);
                post.author().set("p");
                post.text().set("forum:text");
                post.subject().set("post sub");
                post.postTime().set(root.getTime());
            }
    };
}

/** Quick lifecycle test.
    Verifies that we can start-stop a NotificationThread. */
AFL_TEST_NOARG("server.talk.NotificationThread:lifecycle")
{
    NullCommandHandler ch;
    MailQueueClient mq(ch);
    Root root(ch, Configuration());
    NotificationThread t(root, mq);
}

/** Test notification for a forum message. */
AFL_TEST("server.talk.NotificationThread:notifyMessage:fast", a)
{
    // Derived from "server.talk.Notify:notifyMessage:initial"
    InternalDatabase db;
    MailMock mq(a);
    Configuration fig;
    fig.notificationDelay = 0;
    Root root(db, fig);

    // Environment
    ForumEnvironment env(root);

    // Test it
    {
        NotificationThread nt(root, mq);
        nt.notifyMessage(env.post);
        afl::sys::Thread::sleep(100);
    }

    // Verify
    MailMock::Message* msg = mq.extract("user:b");
    a.checkNonNull("01. b", msg);
    a.checkEqual("02. b template", msg->templateName, "talk-forum");
    a.checkEqual("03. b subject",  msg->parameters.at("subject"), "post sub");

    a.check("99. empty", mq.empty());
}

/** Test notification for a forum message with delay. */
AFL_TEST("server.talk.NotificationThread:notifyMessage:slow", a)
{
    InternalDatabase db;
    MailMock mq(a);
    Configuration fig;
    fig.notificationDelay = 10;
    Root root(db, fig);

    // Environment
    ForumEnvironment env(root);

    // Test it
    {
        NotificationThread nt(root, mq);
        nt.notifyMessage(env.post);
        afl::sys::Thread::sleep(100);
    }

    // Verify: No messages have been sent so far.
    a.check("99. empty", mq.empty());
}

/** Test notification for a user PM. */
AFL_TEST("server.talk.NotificationThread:notifyPM", a)
{
    InternalDatabase db;
    MailMock mq(a);
    Configuration fig;
    fig.notificationDelay = 10;
    Root root(db, fig);

    afl::data::StringList_t notifyIndividual;
    notifyIndividual.push_back("user:i");

    afl::data::StringList_t notifyGroup;
    notifyGroup.push_back("user:g");

    const int32_t pmid = 66;
    UserPM pm(root, pmid);
    pm.author().set("a");
    pm.receivers().set("whatever");
    pm.subject().set("pm subj");
    pm.text().set("forum:pm text");

    // Test it
    {
        NotificationThread nt(root, mq);
        nt.notifyPM(pm, notifyIndividual, notifyGroup);
        afl::sys::Thread::sleep(100);
    }

    // Verify
    MailMock::Message* msg;

    msg = mq.extract("user:i");
    a.checkNonNull("01. i", msg);
    a.checkEqual("02. i template", msg->templateName, "talk-pm-message");
    a.checkEqual("03. i subject",  msg->parameters.at("subject"), "pm subj");
    a.checkEqual("04. i message",  msg->parameters.at("message"), "pm text\n");

    msg = mq.extract("user:g");
    a.checkNonNull("11. g", msg);
    a.checkEqual("12. g template", msg->templateName, "talk-pm");

    a.check("99. empty", mq.empty());
}
