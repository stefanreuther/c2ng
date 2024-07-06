/**
  *  \file test/server/talk/talkpmtest.cpp
  *  \brief Test for server::talk::TalkPM
  */

#include "server/talk/talkpm.hpp"

#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/integersetkey.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "afl/net/redis/subtree.hpp"
#include "afl/test/testrunner.hpp"
#include "server/talk/root.hpp"
#include "server/talk/session.hpp"
#include "server/talk/talkfolder.hpp"
#include "server/talk/user.hpp"
#include "server/talk/userfolder.hpp"
#include "server/talk/userpm.hpp"

using afl::net::NullCommandHandler;
using afl::net::redis::InternalDatabase;
using server::talk::Configuration;
using server::talk::Root;
using server::talk::Session;
using server::talk::TalkPM;
using server::talk::User;
using server::talk::UserFolder;
using server::talk::UserPM;

/** Test rendering (bug #336). */
AFL_TEST("server.talk.TalkPM:render", a)
{
    // Infrastructure
    InternalDatabase db;
    NullCommandHandler mq;
    Root root(db, mq, Configuration());
    Session session;

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
    TalkPM testee(session, root);
    const char EXPECT[] = "[quote=b]\nlet's test this[/quote]";

    a.checkEqual("01. render", testee.render(1, 10, server::interface::TalkPM::Options()), EXPECT);

    afl::container::PtrVector<String_t> out;
    static const int32_t in[] = { 10 };
    testee.render(1, in, out);
    a.checkEqual  ("11. size", out.size(), 1U);
    a.checkNonNull("12. result", out[0]);
    a.checkEqual  ("13. result", *out[0], EXPECT);
}

/** Command tests. */
AFL_TEST("server.talk.TalkPM:basics", a)
{
    // Infrastructure
    InternalDatabase db;
    NullCommandHandler mq;
    Root root(db, mq, Configuration());

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
        a.checkEqual("01. create", n, 1);
    }

    // Send a reply
    {
        int32_t n = TalkPM(bSession, root).create("u:a", "re: subj", "text:wtf", 1);
        a.checkEqual("11. create", n, 2);
    }

    // Get info on #1. It's in A's outbox and B's inbox
    {
        TalkPM::Info i = TalkPM(aSession, root).getInfo(2, 1);
        a.checkEqual("21. author",    i.author, "a");
        a.checkEqual("22. receivers", i.receivers, "u:b");
        a.checkEqual("23. subject",   i.subject, "subj");
        a.checkEqual("24. flags",     i.flags, 1);            // we sent it, that counts as if it is read

        AFL_CHECK_THROWS(a("31. getInfo admin"), TalkPM(aSession, root).getInfo(1, 1), std::exception);
    }
    {
        TalkPM::Info i = TalkPM(bSession, root).getInfo(1, 1);
        a.checkEqual("32. author",    i.author, "a");
        a.checkEqual("33. receivers", i.receivers, "u:b");
        a.checkEqual("34. subject",   i.subject, "subj");
        a.checkEqual("35. flags",     i.flags, 0);

        AFL_CHECK_THROWS(a("41. getInfo admin"), TalkPM(bSession, root).getInfo(2, 1), std::exception);
    }

    // Get info on #2. It's in A's inbox; should suggest linking with previous message in outbox.
    {
        TalkPM::Info i = TalkPM(aSession, root).getInfo(1, 2);
        a.checkEqual("51. author",           i.author, "b");
        a.checkEqual("52. receivers",        i.receivers, "u:a");
        a.checkEqual("53. subject",          i.subject, "re: subj");
        a.checkEqual("54. flags",            i.flags, 0);
        a.checkEqual("55. parent",           i.parent.orElse(-1), 1);
        a.checkEqual("56. parentFolder",     i.parentFolder.orElse(-1), 2);
        a.checkEqual("57. parentSubject",    i.parentSubject.orElse(""), "subj");
        a.checkEqual("58. parentFolderName", i.parentFolderName.orElse(""), "Outbox");
    }

    // Copy. Message #1 is in A's outbox, #2 is in his inbox. Copy #2 into outbox as well.
    {
        static const int32_t mids[] = {1,2,9};

        // Result is number of messages copied. Only #2 is in inbox.
        a.checkEqual("61. copy", TalkPM(aSession, root).copy(1, 2, mids), 1);

        // Copying again does not change the result.
        a.checkEqual("71. copy", TalkPM(aSession, root).copy(1, 2, mids), 1);

        // Self-copy: both messages are in source.
        a.checkEqual("81. copy", TalkPM(aSession, root).copy(2, 2, mids), 2);

        // Verify that refcount is not broken.
        // Message #1 is in A's outbox and B's inbox.
        // Message #2 is in A's in+outbox and B's outbox.
        a.checkEqual("91. referenceCounter", UserPM(root, 1).referenceCounter().get(), 2);
        a.checkEqual("92. referenceCounter", UserPM(root, 2).referenceCounter().get(), 3);
    }

    // Multi-get
    {
        static const int32_t mids[] = {1,2,9};
        afl::container::PtrVector<TalkPM::Info> result;
        TalkPM(aSession, root).getInfo(2, mids, result);
        a.checkEqual  ("101. size", result.size(), 3U);
        a.checkNonNull("102. result", result[0]);
        a.checkNonNull("103. result", result[1]);
        a.checkNull   ("104. result", result[2]);
        a.checkEqual  ("105. author", result[0]->author, "a");
        a.checkEqual  ("106. author", result[1]->author, "b");
    }

    // Move.
    {
        static const int32_t mids[] = {1,2,9};

        // Result is number of messages moved. Only #2 is in A's inbox.
        a.checkEqual("111. move", TalkPM(aSession, root).move(1, 2, mids), 1);

        // Move again. Inbox now empty, so result is 0.
        a.checkEqual("121. move", TalkPM(aSession, root).move(1, 2, mids), 0);

        // Verify that refcount is not broken.
        // Message #1 is in A's outbox and B's inbox.
        // Message #2 is in A's outbox and B's outbox.
        a.checkEqual("131. referenceCounter", UserPM(root, 1).referenceCounter().get(), 2);
        a.checkEqual("132. referenceCounter", UserPM(root, 2).referenceCounter().get(), 2);

        // Self-move is a no-op.
        a.checkEqual("141. copy", TalkPM(aSession, root).copy(2, 2, mids), 2);
        a.checkEqual("142. referenceCounter", UserPM(root, 1).referenceCounter().get(), 2);
        a.checkEqual("143. referenceCounter", UserPM(root, 2).referenceCounter().get(), 2);
    }

    // Remove
    {
        static const int32_t mids[] = {1,7};

        // Message #1 is in A's outbox and B's inbox.
        a.checkEqual("151. remove", TalkPM(aSession, root).remove(1, mids), 0);
        a.checkEqual("152. remove", TalkPM(aSession, root).remove(2, mids), 1);
        a.checkEqual("153. remove", TalkPM(bSession, root).remove(1, mids), 1);
        a.checkEqual("154. remove", TalkPM(bSession, root).remove(2, mids), 0);
        a.checkEqual("155. referenceCounter", UserPM(root, 1).referenceCounter().get(), 0);
    }

    // Render
    {
        TalkPM::Options opts;
        opts.format = "html";
        a.checkEqual("161. render", TalkPM(aSession, root).render(2, 2, opts), "<p>wtf</p>\n");
        a.checkEqual("162. render", TalkPM(bSession, root).render(2, 2, opts), "<p>wtf</p>\n");
        AFL_CHECK_THROWS(a("163. render"), TalkPM(bSession, root).render(1, 2, opts), std::exception);
    }
    {
        static const int32_t mids[] = {5,2};
        afl::container::PtrVector<String_t> result;
        AFL_CHECK_SUCCEEDS(a("164. render"), TalkPM(aSession, root).render(2, mids, result));
        a.checkEqual  ("165. size", result.size(), 2U);
        a.checkNull   ("166. result", result[0]);
        a.checkNonNull("167. result", result[1]);
        a.checkEqual  ("168. result", *result[1], "text:wtf");  // default state is type "raw"
    }

    // Flags
    {
        // Verify initial state
        a.checkEqual("171. getInfo", TalkPM(aSession, root).getInfo(2, 2).flags, 0);
        a.checkEqual("172. getInfo", TalkPM(bSession, root).getInfo(2, 2).flags, 1);

        // Change flags
        static const int32_t mids[] = {2};
        a.checkEqual("181. changeFlags", TalkPM(aSession, root).changeFlags(2, 1, 4, mids), 1);  // A's outbox
        a.checkEqual("182. changeFlags", TalkPM(bSession, root).changeFlags(2, 0, 8, mids), 1);  // B's outbox
        a.checkEqual("183. changeFlags", TalkPM(bSession, root).changeFlags(1, 0, 8, mids), 0);  // wrong folder

        // Verify initial state
        a.checkEqual("191. getInfo", TalkPM(aSession, root).getInfo(2, 2).flags, 4);
        a.checkEqual("192. getInfo", TalkPM(bSession, root).getInfo(2, 2).flags, 9);
    }
}

/** Command tests for root. Must all fail. */
AFL_TEST("server.talk.TalkPM:admin", a)
{
    // Infrastructure
    InternalDatabase db;
    NullCommandHandler mq;
    Root root(db, mq, Configuration());
    Session session;

    // Make a system folders (not required, commands hopefully fail before looking here)
    root.defaultFolderRoot().subtree("1").hashKey("header").stringField("name").set("Inbox");
    root.defaultFolderRoot().intSetKey("all").add(1);

    // Testee
    TalkPM testee(session, root);

    static const int32_t pmids[] = { 1, 3, 5 };
    AFL_CHECK_THROWS(a("01. create"), testee.create("u:a", "subj", "text:text", afl::base::Nothing), std::exception);
    AFL_CHECK_THROWS(a("02. getInfo"), testee.getInfo(1, 42), std::exception);
    {
        afl::container::PtrVector<TalkPM::Info> result;
        AFL_CHECK_THROWS(a("03. getInfo"), testee.getInfo(1, pmids, result), std::exception);
    }
    AFL_CHECK_THROWS(a("04. copy"), testee.copy(1, 2, pmids), std::exception);
    AFL_CHECK_THROWS(a("05. move"), testee.move(1, 2, pmids), std::exception);
    AFL_CHECK_THROWS(a("06. remove"), testee.remove(1, pmids), std::exception);
    AFL_CHECK_THROWS(a("07. render"), testee.render(1, 42, TalkPM::Options()), std::exception);
    {
        afl::container::PtrVector<String_t> result;
        AFL_CHECK_THROWS(a("08. render"), testee.render(1, pmids, result), std::exception);
    }
    AFL_CHECK_THROWS(a("09. changeFlags"), testee.changeFlags(1, 4, 8, pmids), std::exception);
}

/** Test receiver handling. */
AFL_TEST("server.talk.TalkPM:receivers", a)
{
    // Infrastructure
    InternalDatabase db;
    NullCommandHandler mq;
    Root root(db, mq, Configuration());
    Session session;
    session.setUser("a");
    TalkPM testee(session, root);
    User ua(root, "a");
    User ub(root, "b");
    User uc(root, "c");
    User ud(root, "d");

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
    a.checkEqual("01. create", testee.create("u:b", "subj", "text:text", afl::base::Nothing), 1);
    a.checkEqual("02. create", testee.create("g:3", "subj", "text:text", afl::base::Nothing), 2);
    a.checkEqual("03. create", testee.create("g:3:1", "subj", "text:text", afl::base::Nothing), 3);
    a.checkEqual("04. create", testee.create("g:3:9", "subj", "text:text", afl::base::Nothing), 4);
    a.checkEqual("05. create", testee.create("g:3:9,u:d", "subj", "text:text", afl::base::Nothing), 5);
    a.checkEqual("06. create", testee.create("u:b,u:a", "subj", "text:text", afl::base::Nothing), 6);

    // Verify mails
    // - a has everything in their outbox, and one in their inbox
    a.check("11", !UserFolder(ua, 1).messages().contains(1));
    a.check("12", !UserFolder(ua, 1).messages().contains(2));
    a.check("13", !UserFolder(ua, 1).messages().contains(3));
    a.check("14", !UserFolder(ua, 1).messages().contains(4));
    a.check("15", !UserFolder(ua, 1).messages().contains(5));
    a.check("16",  UserFolder(ua, 1).messages().contains(6));
    a.check("17",  UserFolder(ua, 2).messages().contains(1));
    a.check("18",  UserFolder(ua, 2).messages().contains(2));
    a.check("19",  UserFolder(ua, 2).messages().contains(3));
    a.check("20",  UserFolder(ua, 2).messages().contains(4));
    a.check("21",  UserFolder(ua, 2).messages().contains(5));
    a.check("22",  UserFolder(ua, 2).messages().contains(6));

    // - b has everything in their inbox
    a.check("31",  UserFolder(ub, 1).messages().contains(1));
    a.check("32",  UserFolder(ub, 1).messages().contains(2));
    a.check("33",  UserFolder(ub, 1).messages().contains(3));
    a.check("34",  UserFolder(ub, 1).messages().contains(4));
    a.check("35",  UserFolder(ub, 1).messages().contains(5));
    a.check("36",  UserFolder(ub, 1).messages().contains(6));

    // - c has just messages 2, 4, 5
    a.check("41", !UserFolder(uc, 1).messages().contains(1));
    a.check("42",  UserFolder(uc, 1).messages().contains(2));
    a.check("43", !UserFolder(uc, 1).messages().contains(3));
    a.check("44",  UserFolder(uc, 1).messages().contains(4));
    a.check("45",  UserFolder(uc, 1).messages().contains(5));
    a.check("46", !UserFolder(uc, 1).messages().contains(6));

    // - c has just messages 2, 5
    a.check("51", !UserFolder(ud, 1).messages().contains(1));
    a.check("52",  UserFolder(ud, 1).messages().contains(2));
    a.check("53", !UserFolder(ud, 1).messages().contains(3));
    a.check("54", !UserFolder(ud, 1).messages().contains(4));
    a.check("55",  UserFolder(ud, 1).messages().contains(5));
    a.check("56", !UserFolder(ud, 1).messages().contains(6));
}

/** Test receiver errors. */
AFL_TEST("server.talk.TalkPM:receivers:error", a)
{
    // Infrastructure
    InternalDatabase db;
    NullCommandHandler mq;
    Root root(db, mq, Configuration());
    Session session;
    session.setUser("a");
    TalkPM testee(session, root);

    // Preload database
    root.gameRoot().intSetKey("all").add(3);

    // Failure: expands to no users
    AFL_CHECK_THROWS(a("01. empty group"), testee.create("g:3", "subj", "text:text", afl::base::Nothing), std::exception);
    AFL_CHECK_THROWS(a("02. empty group"), testee.create("g:3:1", "subj", "text:text", afl::base::Nothing), std::exception);

    // Failure: range error
    AFL_CHECK_THROWS(a("11. range error"), testee.create("g:9", "subj", "text:text", afl::base::Nothing), std::exception);
    AFL_CHECK_THROWS(a("12. range error"), testee.create("g:0", "subj", "text:text", afl::base::Nothing), std::exception);
    AFL_CHECK_THROWS(a("13. range error"), testee.create("g:3:0", "subj", "text:text", afl::base::Nothing), std::exception);
    AFL_CHECK_THROWS(a("14. range error"), testee.create("g:3:20", "subj", "text:text", afl::base::Nothing), std::exception);

    // Failure: parse error
    AFL_CHECK_THROWS(a("21. syntax error"), testee.create("", "subj", "text:text", afl::base::Nothing), std::exception);
    AFL_CHECK_THROWS(a("22. syntax error"), testee.create("u:a,", "subj", "text:text", afl::base::Nothing), std::exception);
    AFL_CHECK_THROWS(a("23. syntax error"), testee.create("u:a, u:b", "subj", "text:text", afl::base::Nothing), std::exception);
    AFL_CHECK_THROWS(a("24. syntax error"), testee.create("u:a,,u:b", "subj", "text:text", afl::base::Nothing), std::exception);
    AFL_CHECK_THROWS(a("25. syntax error"), testee.create("x:1", "subj", "text:text", afl::base::Nothing), std::exception);
}

/** Test suggested folders. */
AFL_TEST("server.talk.TalkPM:suggestedFolder", a)
{
    // Infrastructure
    InternalDatabase db;
    NullCommandHandler mq;
    Root root(db, mq, Configuration());

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

    // Make a user folder; use TalkFolder for simplicity
    int32_t folderId = server::talk::TalkFolder(aSession, root).create("User", afl::base::Nothing);

    // Create messages
    int32_t ma = TalkPM(aSession, root).create("u:b", "subj", "one", afl::base::Nothing);
    int32_t mb = TalkPM(bSession, root).create("u:a", "re: subj", "two", ma);
    int32_t mc = TalkPM(aSession, root).create("u:b", "re: re: subj", "two", mb);

    // Move a into folder
    const int32_t pmids[] = {ma};
    int32_t n = TalkPM(aSession, root).move(2, folderId, pmids);
    a.checkEqual("01. move", n, 1);

    // Verify
    TalkPM::Info i = TalkPM(aSession, root).getInfo(2, mc);
    a.checkEqual("11. author",              i.author, "a");
    a.checkEqual("12. receivers",           i.receivers, "u:b");
    a.checkEqual("13. subject",             i.subject, "re: re: subj");
    a.checkEqual("14. flags",               i.flags, 1);
    a.checkEqual("15. parent",              i.parent.orElse(-1), mb);
    a.checkEqual("16. parentFolder",        i.parentFolder.orElse(-1), 1);
    a.checkEqual("17. parentSubject",       i.parentSubject.orElse(""), "re: subj");
    a.checkEqual("18. parentFolderName",    i.parentFolderName.orElse(""), "Inbox");
    a.checkEqual("19. suggestedFolder",     i.suggestedFolder.orElse(-1), folderId);
    a.checkEqual("20. suggestedFolderName", i.suggestedFolderName.orElse(""), "User");
}

/** Test permission to post: default (success) case. */
AFL_TEST("server.talk.TalkPM:perm:success", a)
{
    // Infrastructure
    InternalDatabase db;
    NullCommandHandler mq;
    Root root(db, mq, Configuration());
    Session session;
    session.setUser("a");
    TalkPM testee(session, root);

    a.checkDifferent("", testee.create("u:b", "subj", "text", afl::base::Nothing), 0);
}

/** Test permission to post: disabled for user. */
AFL_TEST("server.talk.TalkPM:perm:disabled:user", a)
{
    // Infrastructure
    InternalDatabase db;
    NullCommandHandler mq;
    Root root(db, mq, Configuration());
    Session session;
    session.setUser("a");
    TalkPM testee(session, root);

    User(root, "a").profile().intField("allowpm").set(0);

    AFL_CHECK_THROWS(a, testee.create("u:b", "subj", "text", afl::base::Nothing), std::runtime_error);
}

/** Test permission to post: disabled globally. */
AFL_TEST("server.talk.TalkPM:perm:disabled:global", a)
{
    // Infrastructure
    InternalDatabase db;
    NullCommandHandler mq;
    Root root(db, mq, Configuration());
    Session session;
    session.setUser("a");
    TalkPM testee(session, root);

    root.defaultProfile().intField("allowpm").set(0);

    AFL_CHECK_THROWS(a, testee.create("u:b", "subj", "text", afl::base::Nothing), std::runtime_error);
}

/** Test permission to post: disabled globally, but enabled per-user. */
AFL_TEST("server.talk.TalkPM:perm:enabled:re-enabled", a)
{
    // Infrastructure
    InternalDatabase db;
    NullCommandHandler mq;
    Root root(db, mq, Configuration());
    Session session;
    session.setUser("a");
    TalkPM testee(session, root);

    root.defaultProfile().intField("allowpm").set(0);
    User(root, "a").profile().intField("allowpm").set(1);

    a.checkDifferent("", testee.create("u:b", "subj", "text", afl::base::Nothing), 0);
}

/** Test permission to post: explicitly enabled. */
AFL_TEST("server.talk.TalkPM:perm:enabled:explicit", a)
{
    // Infrastructure
    InternalDatabase db;
    NullCommandHandler mq;
    Root root(db, mq, Configuration());
    Session session;
    session.setUser("a");
    TalkPM testee(session, root);

    User(root, "a").profile().intField("allowpm").set(1);

    a.checkDifferent("", testee.create("u:b", "subj", "text", afl::base::Nothing), 0);
}

/** Test rate limiting: a fresh user can send at least 10 messages, but not more than 50.
    Actual limit as of 20240706: 24 with default config. */
AFL_TEST("server.talk.TalkPM:ratelimit", a)
{
    // Infrastructure
    InternalDatabase db;
    NullCommandHandler mq;
    Root root(db, mq, Configuration());
    Session session;
    session.setUser("a");
    TalkPM testee(session, root);

    int i = 0;
    while (i < 100) {
        try {
            testee.create("u:b", "subj", "text", afl::base::Nothing);
        }
        catch (std::runtime_error& e) {
            break;
        }
        ++i;
    }

    a.checkGreaterEqual("count", i, 10);
    a.checkGreaterEqual("count", 50, i);
}
