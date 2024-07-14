/**
  *  \file test/server/talk/notifytest.cpp
  *  \brief Test for server::talk::Notify
  */

#include "server/talk/notify.hpp"

#include "afl/test/testrunner.hpp"
#include "server/talk/root.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "server/test/mailmock.hpp"
#include "server/talk/configuration.hpp"
#include "server/talk/message.hpp"
#include "server/talk/topic.hpp"
#include "server/talk/forum.hpp"
#include "server/talk/user.hpp"
#include "server/interface/mailqueueserver.hpp"

using afl::net::redis::InternalDatabase;
using server::talk::Configuration;
using server::talk::Forum;
using server::talk::Message;
using server::talk::Root;
using server::talk::Topic;
using server::test::MailMock;
using server::talk::User;
using server::interface::MailQueueServer;

/** Notify message, initial message (topic creation). */
AFL_TEST("server.talk.Notify:notifyMessage:initial", a)
{
    InternalDatabase db;
    MailMock mq(a);
    MailQueueServer mail(mq);
    Root root(db, mail, Configuration());

    // IDs
    const int32_t forumId = 99;
    const int32_t topicId = 42;
    const int32_t postId = 123;

    // Users
    User postUser(root, "p");
    postUser.profile().intField("talkwatchindividual").set(0);
    postUser.watchedForums().add(forumId);
    root.userRoot().stringSetKey("all").add("p");

    User bulkUser(root, "b");
    bulkUser.profile().intField("talkwatchindividual").set(0);
    bulkUser.watchedForums().add(forumId);
    root.userRoot().stringSetKey("all").add("b");

    User singleUser(root, "s");
    singleUser.profile().intField("talkwatchindividual").set(1);
    singleUser.watchedForums().add(forumId);
    root.userRoot().stringSetKey("all").add("s");

    // Create forum
    Forum forum(root, forumId);
    root.allForums().add(forumId);
    forum.creationTime().set(1);
    forum.header().stringField("name").set("Forum");
    forum.watchers().add("p");
    forum.watchers().add("b");
    forum.watchers().add("s");
    forum.readPermissions().set("all");

    // Topic
    Topic topic(root, topicId);
    forum.topics().add(topicId);
    topic.subject().set("topic sub");
    topic.forumId().set(forumId);
    topic.firstPostingId().set(postId);

    // Post
    Message post(root, postId);
    topic.messages().add(postId);
    forum.messages().add(postId);
    post.topicId().set(topicId);
    post.author().set("p");
    post.text().set("forum:text");
    post.subject().set("post sub");

    // Test it
    notifyMessage(post, topic, forum, root);

    // Verify
    MailMock::Message* msg;

    // - user 'p' must not have got a message
    msg = mq.extract("user:p");
    a.checkNull("01. p", msg);

    // - user 'b' must have got a message without text
    msg = mq.extract("user:b");
    a.checkNonNull("11. b", msg);
    a.checkEqual("12. b template", msg->templateName, "talk-forum");
    a.checkEqual("13. b subject",  msg->parameters.at("subject"), "post sub");

    // - user 's' must have got a message with text
    msg = mq.extract("user:s");
    a.checkNonNull("21. s", msg);
    a.checkEqual("22. s template", msg->templateName, "talk-forum-message");
    a.checkEqual("23. s subject",  msg->parameters.at("subject"), "post sub");
    a.checkEqual("24. s message",  msg->parameters.at("message"), "text\n");

    a.check("99. empty", mq.empty());
}

/** Notify message, reply. */
AFL_TEST("server.talk.Notify:notifyMessage:reply", a)
{
    InternalDatabase db;
    MailMock mq(a);
    MailQueueServer mail(mq);
    Root root(db, mail, Configuration());

    // IDs
    const int32_t forumId = 99;
    const int32_t topicId = 42;
    const int32_t postId = 123;

    // Users
    User postUser(root, "p");
    postUser.profile().intField("talkwatchindividual").set(0);
    postUser.watchedForums().add(forumId);
    postUser.watchedTopics().add(topicId);
    root.userRoot().stringSetKey("all").add("p");

    User bulkUser(root, "b");
    bulkUser.profile().intField("talkwatchindividual").set(0);
    bulkUser.watchedForums().add(forumId);
    bulkUser.watchedTopics().add(topicId);
    root.userRoot().stringSetKey("all").add("b");

    User otherUser(root, "o");
    otherUser.profile().intField("talkwatchindividual").set(0);
    otherUser.watchedForums().add(forumId);
    otherUser.watchedTopics().add(topicId);
    root.userRoot().stringSetKey("all").add("o");

    User singleUser(root, "s");
    singleUser.profile().intField("talkwatchindividual").set(1);
    singleUser.watchedForums().add(forumId);
    singleUser.watchedTopics().add(topicId);
    root.userRoot().stringSetKey("all").add("s");

    // Create forum
    Forum forum(root, forumId);
    root.allForums().add(forumId);
    forum.creationTime().set(1);
    forum.header().stringField("name").set("Forum");
    forum.watchers().add("p");
    forum.watchers().add("b");
    forum.watchers().add("s");
    forum.watchers().add("o");
    forum.readPermissions().set("all");

    // Topic
    Topic topic(root, topicId);
    forum.topics().add(topicId);
    topic.subject().set("topic sub");
    topic.forumId().set(forumId);
    topic.firstPostingId().set(postId);
    topic.watchers().add("p");
    topic.watchers().add("b");
    topic.watchers().add("s");

    // Post
    Message post(root, postId);
    topic.messages().add(postId);
    forum.messages().add(postId);
    post.topicId().set(topicId);
    post.author().set("p");
    post.text().set("forum:[quote]text[/quote]\nmore text");
    post.subject().set("post sub");

    // Test it
    notifyMessage(post, topic, forum, root);

    // Verify
    MailMock::Message* msg;

    // - user 'p' must not have got a message
    msg = mq.extract("user:p");
    a.checkNull("01. p", msg);

    // - user 'b' must have got a message without text
    msg = mq.extract("user:b");
    a.checkNonNull("11. b", msg);
    a.checkEqual("12. b template", msg->templateName, "talk-topic");
    a.checkEqual("13. b subject",  msg->parameters.at("subject"), "post sub");

    // - user 's' must have got a message with text
    msg = mq.extract("user:s");
    a.checkNonNull("21. s", msg);
    a.checkEqual("22. s template", msg->templateName, "talk-topic-message");
    a.checkEqual("23. s subject",  msg->parameters.at("subject"), "post sub");
    a.checkEqual("24. s message",  msg->parameters.at("message"), "> text\n\nmore text\n");

    // - user 'o' must have got forum notification
    msg = mq.extract("user:o");
    a.checkNonNull("31. b", msg);
    a.checkEqual("32. o template", msg->templateName, "talk-forum");
    a.checkEqual("33. o subject",  msg->parameters.at("subject"), "post sub");

    a.check("99. empty", mq.empty());
}
