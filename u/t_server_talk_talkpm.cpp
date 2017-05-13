/**
  *  \file u/t_server_talk_talkpm.cpp
  *  \brief Test for server::talk::TalkPM
  */

#include "server/talk/talkpm.hpp"

#include "t_server_talk.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/integersetkey.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "afl/net/redis/subtree.hpp"
#include "server/talk/root.hpp"
#include "server/talk/session.hpp"
#include "server/talk/userpm.hpp"
#include "server/talk/userfolder.hpp"
#include "server/talk/user.hpp"

/** Test rendering (bug #336). */
void
TestServerTalkTalkPM::testRender()
{
    // Infrastructure
    afl::net::redis::InternalDatabase db;
    afl::net::NullCommandHandler mq;
    server::talk::Root root(db, mq, server::talk::Configuration());
    server::talk::Session session;

    // Configure db - just what is needed
    root.userRoot().subtree("1001").intSetKey("pm:folder:1:messages").add(10);
    root.userRoot().subtree("1001").stringKey("name").set("streu");
    root.userRoot().subtree("1003").stringKey("name").set("b");
    root.pmRoot().subtree("10").hashKey("header").stringField("author").set("1003");
    root.pmRoot().subtree("10").stringKey("text").set("forum:let's test this");

    // Configure session
    session.setUser("1001");
    session.renderOptions().setFormat("quote:forum");

    // Test it
    server::talk::TalkPM testee(session, root);
    const char EXPECT[] = "[quote=b]\nlet's test this[/quote]";

    TS_ASSERT_EQUALS(testee.render(1, 10, server::interface::TalkPM::Options()), EXPECT);

    afl::container::PtrVector<String_t> out;
    static const int32_t in[] = { 10 };
    testee.render(1, in, out);
    TS_ASSERT_EQUALS(out.size(), 1U);
    TS_ASSERT(out[0] != 0);
    TS_ASSERT_EQUALS(*out[0], EXPECT);
}

/** Command tests. */
void
TestServerTalkTalkPM::testIt()
{
    using server::talk::TalkPM;
    using server::talk::Session;

    // Infrastructure
    afl::net::redis::InternalDatabase db;
    afl::net::NullCommandHandler mq;
    server::talk::Root root(db, mq, server::talk::Configuration());

    Session aSession;
    Session bSession;
    aSession.setUser("a");
    bSession.setUser("b");

    // Make two system folders
    root.defaultFolderRoot().subtree("1").hashKey("header").stringField("name").set("Inbox");
    root.defaultFolderRoot().subtree("1").hashKey("header").stringField("description").set("Incoming messages");
    root.defaultFolderRoot().subtree("2").hashKey("header").stringField("name").set("Outbox");
    root.defaultFolderRoot().subtree("2").hashKey("header").stringField("description").set("Sent messages");
    root.defaultFolderRoot().intSetKey("all").add(1);
    root.defaultFolderRoot().intSetKey("all").add(2);

    // Send a message from A to B
    {
        int32_t n = TalkPM(aSession, root).create("u:b", "subj", "text:text", afl::base::Nothing);
        TS_ASSERT_EQUALS(n, 1);
    }

    // Send a reply
    {
        int32_t n = TalkPM(bSession, root).create("u:a", "re: subj", "text:wtf", 1);
        TS_ASSERT_EQUALS(n, 2);
    }

    // Get info on #1. It's in A's outbox and B's inbox
    {
        TalkPM::Info i = TalkPM(aSession, root).getInfo(2, 1);
        TS_ASSERT_EQUALS(i.author, "a");
        TS_ASSERT_EQUALS(i.receivers, "u:b");
        TS_ASSERT_EQUALS(i.subject, "subj");
        TS_ASSERT_EQUALS(i.flags, 1);            // we sent it, that counts as if it is read

        TS_ASSERT_THROWS(TalkPM(aSession, root).getInfo(1, 1), std::exception);
    }
    {
        TalkPM::Info i = TalkPM(bSession, root).getInfo(1, 1);
        TS_ASSERT_EQUALS(i.author, "a");
        TS_ASSERT_EQUALS(i.receivers, "u:b");
        TS_ASSERT_EQUALS(i.subject, "subj");
        TS_ASSERT_EQUALS(i.flags, 0);

        TS_ASSERT_THROWS(TalkPM(bSession, root).getInfo(2, 1), std::exception);
    }

    // Copy. Message #1 is in A's outbox, #2 is in his inbox. Copy #2 into outbox as well.
    {
        static const int32_t mids[] = {1,2,9};

        // Result is number of messages copied. Only #2 is in inbox.
        TS_ASSERT_EQUALS(TalkPM(aSession, root).copy(1, 2, mids), 1);

        // Copying again does not change the result.
        TS_ASSERT_EQUALS(TalkPM(aSession, root).copy(1, 2, mids), 1);

        // Self-copy: both messages are in source.
        TS_ASSERT_EQUALS(TalkPM(aSession, root).copy(2, 2, mids), 2);

        // Verify that refcount is not broken.
        // Message #1 is in A's outbox and B's inbox.
        // Message #2 is in A's in+outbox and B's outbox.
        TS_ASSERT_EQUALS(server::talk::UserPM(root, 1).referenceCounter().get(), 2);
        TS_ASSERT_EQUALS(server::talk::UserPM(root, 2).referenceCounter().get(), 3);
    }

    // Multi-get
    {
        static const int32_t mids[] = {1,2,9};
        afl::container::PtrVector<TalkPM::Info> result;
        TalkPM(aSession, root).getInfo(2, mids, result);
        TS_ASSERT_EQUALS(result.size(), 3U);
        TS_ASSERT(result[0] != 0);
        TS_ASSERT(result[1] != 0);
        TS_ASSERT(result[2] == 0);
        TS_ASSERT_EQUALS(result[0]->author, "a");
        TS_ASSERT_EQUALS(result[1]->author, "b");
    }

    // Move.
    {
        static const int32_t mids[] = {1,2,9};

        // Result is number of messages moved. Only #2 is in A's inbox.
        TS_ASSERT_EQUALS(TalkPM(aSession, root).move(1, 2, mids), 1);

        // Move again. Inbox now empty, so result is 0.
        TS_ASSERT_EQUALS(TalkPM(aSession, root).move(1, 2, mids), 0);

        // Verify that refcount is not broken.
        // Message #1 is in A's outbox and B's inbox.
        // Message #2 is in A's outbox and B's outbox.
        TS_ASSERT_EQUALS(server::talk::UserPM(root, 1).referenceCounter().get(), 2);
        TS_ASSERT_EQUALS(server::talk::UserPM(root, 2).referenceCounter().get(), 2);

        // Self-move is a no-op.
        TS_ASSERT_EQUALS(TalkPM(aSession, root).copy(2, 2, mids), 2);
        TS_ASSERT_EQUALS(server::talk::UserPM(root, 1).referenceCounter().get(), 2);
        TS_ASSERT_EQUALS(server::talk::UserPM(root, 2).referenceCounter().get(), 2);
    }

    // Remove
    {
        static const int32_t mids[] = {1,7};

        // Message #1 is in A's outbox and B's inbox.
        TS_ASSERT_EQUALS(TalkPM(aSession, root).remove(1, mids), 0);
        TS_ASSERT_EQUALS(TalkPM(aSession, root).remove(2, mids), 1);
        TS_ASSERT_EQUALS(TalkPM(bSession, root).remove(1, mids), 1);
        TS_ASSERT_EQUALS(TalkPM(bSession, root).remove(2, mids), 0);
        TS_ASSERT_EQUALS(server::talk::UserPM(root, 1).referenceCounter().get(), 0);
    }

    // Render
    {
        TalkPM::Options opts;
        opts.format = "html";
        TS_ASSERT_EQUALS(TalkPM(aSession, root).render(2, 2, opts), "<p>wtf</p>\n");
        TS_ASSERT_EQUALS(TalkPM(bSession, root).render(2, 2, opts), "<p>wtf</p>\n");
        TS_ASSERT_THROWS(TalkPM(bSession, root).render(1, 2, opts), std::exception);
    }
    {
        static const int32_t mids[] = {5,2};
        afl::container::PtrVector<String_t> result;
        TS_ASSERT_THROWS_NOTHING(TalkPM(aSession, root).render(2, mids, result));
        TS_ASSERT_EQUALS(result.size(), 2U);
        TS_ASSERT(result[0] == 0);
        TS_ASSERT(result[1] != 0);
        TS_ASSERT_EQUALS(*result[1], "text:wtf");  // default state is type "raw"
    }

    // Flags
    {
        // Verify initial state
        TS_ASSERT_EQUALS(TalkPM(aSession, root).getInfo(2, 2).flags, 0);
        TS_ASSERT_EQUALS(TalkPM(bSession, root).getInfo(2, 2).flags, 1);

        // Change flags
        static const int32_t mids[] = {2};
        TS_ASSERT_EQUALS(TalkPM(aSession, root).changeFlags(2, 1, 4, mids), 1);  // A's outbox
        TS_ASSERT_EQUALS(TalkPM(bSession, root).changeFlags(2, 0, 8, mids), 1);  // B's outbox
        TS_ASSERT_EQUALS(TalkPM(bSession, root).changeFlags(1, 0, 8, mids), 0);  // wrong folder

        // Verify initial state
        TS_ASSERT_EQUALS(TalkPM(aSession, root).getInfo(2, 2).flags, 4);
        TS_ASSERT_EQUALS(TalkPM(bSession, root).getInfo(2, 2).flags, 9);
    }
}

/** Command tests for root. Must all fail. */
void
TestServerTalkTalkPM::testRoot()
{
    using server::talk::TalkPM;

    // Infrastructure
    afl::net::redis::InternalDatabase db;
    afl::net::NullCommandHandler mq;
    server::talk::Root root(db, mq, server::talk::Configuration());
    server::talk::Session session;

    // Make a system folders (not required, commands hopefully fail before looking here)
    root.defaultFolderRoot().subtree("1").hashKey("header").stringField("name").set("Inbox");
    root.defaultFolderRoot().intSetKey("all").add(1);

    // Testee
    TalkPM testee(session, root);

    static const int32_t pmids[] = { 1, 3, 5 };
    TS_ASSERT_THROWS(testee.create("u:a", "subj", "text:text", afl::base::Nothing), std::exception);
    TS_ASSERT_THROWS(testee.getInfo(1, 42), std::exception);
    {
        afl::container::PtrVector<TalkPM::Info> result;
        TS_ASSERT_THROWS(testee.getInfo(1, pmids, result), std::exception);
    }
    TS_ASSERT_THROWS(testee.copy(1, 2, pmids), std::exception);
    TS_ASSERT_THROWS(testee.move(1, 2, pmids), std::exception);
    TS_ASSERT_THROWS(testee.remove(1, pmids), std::exception);
    TS_ASSERT_THROWS(testee.render(1, 42, TalkPM::Options()), std::exception);
    {
        afl::container::PtrVector<String_t> result;
        TS_ASSERT_THROWS(testee.render(1, pmids, result), std::exception);
    }
    TS_ASSERT_THROWS(testee.changeFlags(1, 4, 8, pmids), std::exception);
}

/** Test receiver handling. */
void
TestServerTalkTalkPM::testReceivers()
{
    using server::talk::UserFolder;

    // Infrastructure
    afl::net::redis::InternalDatabase db;
    afl::net::NullCommandHandler mq;
    server::talk::Root root(db, mq, server::talk::Configuration());
    server::talk::Session session;
    session.setUser("a");
    server::talk::TalkPM testee(session, root);
    server::talk::User ua(root, "a");
    server::talk::User ub(root, "b");
    server::talk::User uc(root, "c");
    server::talk::User ud(root, "d");

    // Preload database
    // - users b,c,d are on game 3
    root.gameRoot().intSetKey("all").add(3);
    root.gameRoot().subtree("3").hashKey("users").intField("b").set(1);
    root.gameRoot().subtree("3").hashKey("users").intField("c").set(1);
    root.gameRoot().subtree("3").hashKey("users").intField("d").set(1);

    // - user b is fed, c is robot together with b
    root.gameRoot().subtree("3").subtree("player").subtree("1").stringListKey("users").pushBack("b");
    root.gameRoot().subtree("3").subtree("player").subtree("9").stringListKey("users").pushBack("c");
    root.gameRoot().subtree("3").subtree("player").subtree("9").stringListKey("users").pushBack("b");

    // Sending mails, successful cases
    TS_ASSERT_EQUALS(testee.create("u:b", "subj", "text:text", afl::base::Nothing), 1);
    TS_ASSERT_EQUALS(testee.create("g:3", "subj", "text:text", afl::base::Nothing), 2);
    TS_ASSERT_EQUALS(testee.create("g:3:1", "subj", "text:text", afl::base::Nothing), 3);
    TS_ASSERT_EQUALS(testee.create("g:3:9", "subj", "text:text", afl::base::Nothing), 4);
    TS_ASSERT_EQUALS(testee.create("g:3:9,u:d", "subj", "text:text", afl::base::Nothing), 5);
    TS_ASSERT_EQUALS(testee.create("u:b,u:a", "subj", "text:text", afl::base::Nothing), 6);

    // Verify mails
    // - a has everything in their outbox, and one in their inbox
    TS_ASSERT(!UserFolder(ua, 1).messages().contains(1));
    TS_ASSERT(!UserFolder(ua, 1).messages().contains(2));
    TS_ASSERT(!UserFolder(ua, 1).messages().contains(3));
    TS_ASSERT(!UserFolder(ua, 1).messages().contains(4));
    TS_ASSERT(!UserFolder(ua, 1).messages().contains(5));
    TS_ASSERT( UserFolder(ua, 1).messages().contains(6));
    TS_ASSERT( UserFolder(ua, 2).messages().contains(1));
    TS_ASSERT( UserFolder(ua, 2).messages().contains(2));
    TS_ASSERT( UserFolder(ua, 2).messages().contains(3));
    TS_ASSERT( UserFolder(ua, 2).messages().contains(4));
    TS_ASSERT( UserFolder(ua, 2).messages().contains(5));
    TS_ASSERT( UserFolder(ua, 2).messages().contains(6));

    // - b has everything in their inbox
    TS_ASSERT( UserFolder(ub, 1).messages().contains(1));
    TS_ASSERT( UserFolder(ub, 1).messages().contains(2));
    TS_ASSERT( UserFolder(ub, 1).messages().contains(3));
    TS_ASSERT( UserFolder(ub, 1).messages().contains(4));
    TS_ASSERT( UserFolder(ub, 1).messages().contains(5));
    TS_ASSERT( UserFolder(ub, 1).messages().contains(6));

    // - c has just messages 2, 4, 5
    TS_ASSERT(!UserFolder(uc, 1).messages().contains(1));
    TS_ASSERT( UserFolder(uc, 1).messages().contains(2));
    TS_ASSERT(!UserFolder(uc, 1).messages().contains(3));
    TS_ASSERT( UserFolder(uc, 1).messages().contains(4));
    TS_ASSERT( UserFolder(uc, 1).messages().contains(5));
    TS_ASSERT(!UserFolder(uc, 1).messages().contains(6));

    // - c has just messages 2, 5
    TS_ASSERT(!UserFolder(ud, 1).messages().contains(1));
    TS_ASSERT( UserFolder(ud, 1).messages().contains(2));
    TS_ASSERT(!UserFolder(ud, 1).messages().contains(3));
    TS_ASSERT(!UserFolder(ud, 1).messages().contains(4));
    TS_ASSERT( UserFolder(ud, 1).messages().contains(5));
    TS_ASSERT(!UserFolder(ud, 1).messages().contains(6));
}

/** Test receiver errors. */
void
TestServerTalkTalkPM::testReceiverErrors()
{
    using server::talk::UserFolder;

    // Infrastructure
    afl::net::redis::InternalDatabase db;
    afl::net::NullCommandHandler mq;
    server::talk::Root root(db, mq, server::talk::Configuration());
    server::talk::Session session;
    session.setUser("a");
    server::talk::TalkPM testee(session, root);

    // Preload database
    root.gameRoot().intSetKey("all").add(3);

    // Failure: expands to no users
    TS_ASSERT_THROWS(testee.create("g:3", "subj", "text:text", afl::base::Nothing), std::exception);
    TS_ASSERT_THROWS(testee.create("g:3:1", "subj", "text:text", afl::base::Nothing), std::exception);

    // Failure: range error
    TS_ASSERT_THROWS(testee.create("g:9", "subj", "text:text", afl::base::Nothing), std::exception);
    TS_ASSERT_THROWS(testee.create("g:0", "subj", "text:text", afl::base::Nothing), std::exception);
    TS_ASSERT_THROWS(testee.create("g:3:0", "subj", "text:text", afl::base::Nothing), std::exception);
    TS_ASSERT_THROWS(testee.create("g:3:20", "subj", "text:text", afl::base::Nothing), std::exception);

    // Failure: parse error
    TS_ASSERT_THROWS(testee.create("", "subj", "text:text", afl::base::Nothing), std::exception);
    TS_ASSERT_THROWS(testee.create("u:a,", "subj", "text:text", afl::base::Nothing), std::exception);
    TS_ASSERT_THROWS(testee.create("u:a, u:b", "subj", "text:text", afl::base::Nothing), std::exception);
    TS_ASSERT_THROWS(testee.create("u:a,,u:b", "subj", "text:text", afl::base::Nothing), std::exception);
    TS_ASSERT_THROWS(testee.create("x:1", "subj", "text:text", afl::base::Nothing), std::exception);
}

