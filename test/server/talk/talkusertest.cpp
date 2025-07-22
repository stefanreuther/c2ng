/**
  *  \file test/server/talk/talkusertest.cpp
  *  \brief Test for server::talk::TalkUser
  */

#include "server/talk/talkuser.hpp"

#include <memory>
#include <stdexcept>
#include "afl/data/access.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/string/format.hpp"
#include "afl/test/testrunner.hpp"
#include "server/talk/forum.hpp"
#include "server/talk/root.hpp"
#include "server/talk/session.hpp"
#include "server/talk/topic.hpp"
#include "server/talk/user.hpp"
#include "server/types.hpp"

using afl::data::Access;
using afl::net::redis::InternalDatabase;
using afl::string::Format;
using server::talk::Configuration;
using server::talk::Forum;
using server::talk::Root;
using server::talk::Session;
using server::talk::TalkUser;
using server::talk::Topic;
using server::talk::User;

/** Test accessNewsrc. */
AFL_TEST("server.talk.TalkUser:accessNewsrc", a)
{
    // Infrastructure
    InternalDatabase db;
    Session session;
    Root root(db, Configuration());
    session.setUser("1004");

    // Prepare database. We only need the message counter to pass limit checks.
    root.lastMessageId().set(200);

    // Messages [0,7] read, [8,15] unread, [16,23] read
    User(root, session.getUser()).newsrc().hashKey("data").stringField("0").set(String_t("\xFF\0\xFF", 3));

    // Testee
    TalkUser testee(session, root);
    std::auto_ptr<afl::data::Value> p;

    // Get single values
    {
        static const int32_t ps[] = {1};
        p.reset(testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetAll, afl::base::Nothing, ps));
        a.checkEqual("01. get", server::toInteger(p.get()), 1);
    }
    {
        static const int32_t ps[] = {7};
        p.reset(testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetAll, afl::base::Nothing, ps));
        a.checkEqual("02. get", server::toInteger(p.get()), 1);
    }
    {
        static const int32_t ps[] = {8};
        p.reset(testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetAll, afl::base::Nothing, ps));
        a.checkEqual("03. get", server::toInteger(p.get()), 0);
    }
    {
        static const int32_t ps[] = {16};
        p.reset(testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetAll, afl::base::Nothing, ps));
        a.checkEqual("04. get", server::toInteger(p.get()), 1);
    }

    // Get multiple values
    {
        static const int32_t ps[] = {5,6,7,8,9};
        p.reset(testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetAll, afl::base::Nothing, ps));
        a.checkEqual("11. get", server::toString(p.get()), "11100");
    }
    {
        static const int32_t ps[] = {5,8,6,7,9};
        p.reset(testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetAll, afl::base::Nothing, ps));
        a.checkEqual("12. get", server::toString(p.get()), "10110");
    }
    {
        static const TalkUser::Selection ss[] = {
            TalkUser::RangeScope, 5, 9
        };
        p.reset(testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetAll, ss, afl::base::Nothing));
        a.checkEqual("13. get", server::toString(p.get()), "11100");
    }

    // Find
    {
        static const int32_t ps[] = {5,6,7,8,9};
        p.reset(testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetFirstRead, afl::base::Nothing, ps));
        a.checkEqual("21. firstRead", server::toInteger(p.get()), 5);
        p.reset(testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetFirstUnread, afl::base::Nothing, ps));
        a.checkEqual("22. firstUnread", server::toInteger(p.get()), 8);
    }
    {
        static const TalkUser::Selection ss[] = {
            TalkUser::RangeScope, 5, 9
        };
        p.reset(testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetFirstRead, ss, afl::base::Nothing));
        a.checkEqual("23. firstRead", server::toInteger(p.get()), 5);
        p.reset(testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetFirstUnread, ss, afl::base::Nothing));
        a.checkEqual("24. firstUnread", server::toInteger(p.get()), 8);
    }
    {
        // Result is first in iteration order, not lowest!
        static const int32_t ps[] = {8,7,6,5,9};
        p.reset(testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetFirstRead, afl::base::Nothing, ps));
        a.checkEqual("25. firstRead", server::toInteger(p.get()), 7);
        p.reset(testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetFirstUnread, afl::base::Nothing, ps));
        a.checkEqual("26. firstUnread", server::toInteger(p.get()), 8);
    }
    {
        static const TalkUser::Selection ss[] = {
            TalkUser::RangeScope, 8, 12
        };
        p.reset(testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetFirstRead, ss, afl::base::Nothing));
        a.checkEqual("27. firstRead", server::toInteger(p.get()), 0);
    }

    // Any/All
    {
        static const int32_t ps[] = {5,6,7,8,9};
        p.reset(testee.accessNewsrc(TalkUser::NoModification, TalkUser::CheckIfAnyRead, afl::base::Nothing, ps));
        a.checkEqual("31. anyRead", server::toInteger(p.get()), 1);
        p.reset(testee.accessNewsrc(TalkUser::NoModification, TalkUser::CheckIfAllRead, afl::base::Nothing, ps));
        a.checkEqual("32. allRead", server::toInteger(p.get()), 0);
    }
    {
        static const int32_t ps[] = {8,9,10};
        p.reset(testee.accessNewsrc(TalkUser::NoModification, TalkUser::CheckIfAnyRead, afl::base::Nothing, ps));
        a.checkEqual("33. anyRead", server::toInteger(p.get()), 0);
        p.reset(testee.accessNewsrc(TalkUser::NoModification, TalkUser::CheckIfAllRead, afl::base::Nothing, ps));
        a.checkEqual("34. allRead", server::toInteger(p.get()), 0);
    }
    {
        static const int32_t ps[] = {5,6,7};
        p.reset(testee.accessNewsrc(TalkUser::NoModification, TalkUser::CheckIfAnyRead, afl::base::Nothing, ps));
        a.checkEqual("35. anyRead", server::toInteger(p.get()), 1);
        p.reset(testee.accessNewsrc(TalkUser::NoModification, TalkUser::CheckIfAllRead, afl::base::Nothing, ps));
        a.checkEqual("36. allRead", server::toInteger(p.get()), 1);
    }
    {
        static const int32_t ps[] = {14,15,16};
        p.reset(testee.accessNewsrc(TalkUser::NoModification, TalkUser::CheckIfAnyRead, afl::base::Nothing, ps));
        a.checkEqual("37. anyRead", server::toInteger(p.get()), 1);
        p.reset(testee.accessNewsrc(TalkUser::NoModification, TalkUser::CheckIfAllRead, afl::base::Nothing, ps));
        a.checkEqual("38. allRead", server::toInteger(p.get()), 0);
    }

    // Modifications
    // start with 11111110000000011111111
    static const TalkUser::Selection all[] = {
        TalkUser::RangeScope, 1, 23
    };
    p.reset(testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetAll, all, afl::base::Nothing));
    a.checkEqual("41. get", server::toString(p.get()), "11111110000000011111111");

    {
        // Get and mark unread
        static const TalkUser::Selection ss[] = {
            TalkUser::RangeScope, 6, 9
        };
        p.reset(testee.accessNewsrc(TalkUser::MarkUnread, TalkUser::GetAll, ss, afl::base::Nothing));
        a.checkEqual("51. markUnread", server::toString(p.get()), "1100");
        p.reset(testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetAll, ss, afl::base::Nothing));
        a.checkEqual("52. get", server::toString(p.get()), "0000");
    }
    p.reset(testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetAll, all, afl::base::Nothing));
    a.checkEqual("53. get", server::toString(p.get()), "11111000000000011111111");

    {
        // Find and mark read
        static const TalkUser::Selection ss[] = {
            TalkUser::RangeScope, 4, 9
        };
        p.reset(testee.accessNewsrc(TalkUser::MarkRead, TalkUser::GetFirstUnread, ss, afl::base::Nothing));
        a.checkEqual("61. markRead", server::toInteger(p.get()), 6);
    }
    p.reset(testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetAll, all, afl::base::Nothing));
    a.checkEqual("62. get", server::toString(p.get()), "11111111100000011111111");
}

/** Test accessNewsrc errors. */
AFL_TEST("server.talk.TalkUser:accessNewsrc:error", a)
{
    // Infrastructure
    InternalDatabase db;
    Session session;
    Root root(db, Configuration());
    session.setUser("1004");

    // Prepare database. We only need the message counter to pass limit checks.
    root.lastMessageId().set(200);

    // Do it
    TalkUser testee(session, root);
    {
        static const TalkUser::Selection ss[] = {
            TalkUser::RangeScope, 201, 210
        };
        AFL_CHECK_THROWS(a("01. bad id range"), testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetAll, ss, afl::base::Nothing), std::exception);
    }
    {
        static const int32_t ps[] = { 100, 200, 201, 210 };
        AFL_CHECK_THROWS(a("02. bad id"), testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetAll, afl::base::Nothing, ps), std::exception);
    }
}

/** Test accessNewsrc for single elements. */
AFL_TEST("server.talk.TalkUser:accessNewsrc:single", a)
{
    // Infrastructure
    InternalDatabase db;
    Session session;
    Root root(db, Configuration());
    session.setUser("1004");
    TalkUser testee(session, root);

    // Prepare database. We only need the message counter to pass limit checks.
    root.lastMessageId().set(200);

    static const int32_t ps[] = {1};
    std::auto_ptr<afl::data::Value> p;

    // Initial state: unread
    p.reset(testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetAll, afl::base::Nothing, ps));
    a.checkEqual("01. get", server::toInteger(p.get()), 0);

    // Mark read
    p.reset(testee.accessNewsrc(TalkUser::MarkRead, TalkUser::NoResult, afl::base::Nothing, ps));

    // Verify
    p.reset(testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetAll, afl::base::Nothing, ps));
    a.checkEqual("11. get", server::toInteger(p.get()), 1);

    // Mark unread
    p.reset(testee.accessNewsrc(TalkUser::MarkUnread, TalkUser::NoResult, afl::base::Nothing, ps));

    // Verify
    p.reset(testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetAll, afl::base::Nothing, ps));
    a.checkEqual("21. get", server::toInteger(p.get()), 0);
}

/** Test accessNewsrc for sets. */
AFL_TEST("server.talk.TalkUser:accessNewsrc:set", a)
{
    // Infrastructure
    InternalDatabase db;
    Session session;
    Root root(db, Configuration());
    session.setUser("1004");
    TalkUser testee(session, root);

    // Preload database
    // - a forum
    const int FORUM_ID = 3;
    Forum f(root, FORUM_ID);
    f.name().set("f");
    root.allForums().add(FORUM_ID);

    // - topic
    const int TOPIC_ID = 42;
    Topic t(root, TOPIC_ID);
    t.subject().set("s");
    f.topics().add(TOPIC_ID);

    // - messages
    for (int i = 3; i < 20; ++i) {
        f.messages().add(i);
        t.messages().add(i);
    }

    /*
     *  Test
     */

    std::auto_ptr<afl::data::Value> p;

    // Mark forum read
    static const TalkUser::Selection forumSelection[] = {
        { TalkUser::ForumScope, FORUM_ID, 0 }
    };
    AFL_CHECK_SUCCEEDS(a("01. mark forum read"), p.reset(testee.accessNewsrc(TalkUser::MarkRead, TalkUser::NoResult, forumSelection, afl::base::Nothing)));

    // Find unread in thread
    static const TalkUser::Selection topicSelection[] = {
        { TalkUser::ThreadScope, TOPIC_ID, 0 }
    };
    AFL_CHECK_SUCCEEDS(a("11. firstUnread"), p.reset(testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetFirstUnread, topicSelection, afl::base::Nothing)));
    a.checkEqual("12. firstUnread", server::toInteger(p.get()), 0);

    // Find read in thread
    AFL_CHECK_SUCCEEDS(a("21. firstRead"), p.reset(testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetFirstRead, topicSelection, afl::base::Nothing)));
    a.checkEqual("22. firstRead", server::toInteger(p.get()), 3);

    // Mark thread unread
    AFL_CHECK_SUCCEEDS(a("31. mark thread unread"), p.reset(testee.accessNewsrc(TalkUser::MarkUnread, TalkUser::NoResult, topicSelection, afl::base::Nothing)));

    // Find read
    AFL_CHECK_SUCCEEDS(a("41. firstRead"), p.reset(testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetFirstRead, forumSelection, afl::base::Nothing)));
    a.checkEqual("42. firstRead", server::toInteger(p.get()), 0);

    // Find unread
    AFL_CHECK_SUCCEEDS(a("51. firstUnread"), p.reset(testee.accessNewsrc(TalkUser::NoModification, TalkUser::GetFirstUnread, forumSelection, afl::base::Nothing)));
    a.checkEqual("52. firstUnread", server::toInteger(p.get()), 3);
}

/** Test commands as root. */
AFL_TEST("server.talk.TalkUser:admin", a)
{
    // Infrastructure
    InternalDatabase db;
    Session session;
    Root root(db, Configuration());
    TalkUser testee(session, root);

    // Test must fail
    AFL_CHECK_THROWS(a("01. accessNewsrc"),      testee.accessNewsrc(TalkUser::NoModification, TalkUser::NoResult, afl::base::Nothing, afl::base::Nothing), std::exception);
    AFL_CHECK_THROWS(a("02. watch"),             testee.watch(afl::base::Nothing), std::exception);
    AFL_CHECK_THROWS(a("03. unwatch"),           testee.unwatch(afl::base::Nothing), std::exception);
    AFL_CHECK_THROWS(a("04. markSeen"),          testee.markSeen(afl::base::Nothing), std::exception);
    AFL_CHECK_THROWS(a("05. getWatchedThreads"), testee.getWatchedThreads(TalkUser::ListParameters()), std::exception);
    AFL_CHECK_THROWS(a("06. getWatchedForums"),  testee.getWatchedForums(TalkUser::ListParameters()), std::exception);
}

/** Test watch/unwatch/getWatchedForums/getWatchedThreads. */
AFL_TEST("server.talk.TalkUser:watch", a)
{
    // Infrastructure
    InternalDatabase db;
    Session session;
    Root root(db, Configuration());
    session.setUser("1004");

    // Populate database
    // - forums 8..12
    for (int i = 8; i < 12; ++i) {
        root.allForums().add(i);
        root.forumRoot().subtree(i).hashKey("header").stringField("name").set("f");
        a.check("01. forum exists", Forum(root, i).exists(root));
    }
    // - topics 1..20
    for (int i = 1; i < 20; ++i) {
        root.topicRoot().subtree(i).hashKey("header").stringField("subject").set("s");
        a.check("02. topic exists", Topic(root, i).exists());
    }

    // Test
    TalkUser testee(session, root);
    std::auto_ptr<afl::data::Value> p;

    // Verify initial state
    AFL_CHECK_SUCCEEDS(a("11. getWatchedForums"), p.reset(testee.getWatchedForums(TalkUser::ListParameters())));
    a.checkEqual("12. getWatchedForums", Access(p).getArraySize(), 0U);

    AFL_CHECK_SUCCEEDS(a("21. getWatchedThreads"), p.reset(testee.getWatchedThreads(TalkUser::ListParameters())));
    a.checkEqual("22. getWatchedThreads", Access(p).getArraySize(), 0U);

    // Watch some things
    {
        static const TalkUser::Selection s[] = {
            { TalkUser::ForumScope,   9, 0 },
            { TalkUser::ThreadScope, 10, 0 },
            { TalkUser::ForumScope,  11, 0 },
        };
        AFL_CHECK_SUCCEEDS(a("31. watch"), testee.watch(s));
    }

    // Verify new state
    AFL_CHECK_SUCCEEDS(a("41. getWatchedForums"), p.reset(testee.getWatchedForums(TalkUser::ListParameters())));
    a.checkEqual("42. getWatchedForums", Access(p).getArraySize(), 2U);
    a.checkEqual("43. getWatchedForums", Access(p)[0].toInteger(), 9);
    a.checkEqual("44. getWatchedForums", Access(p)[1].toInteger(), 11);

    AFL_CHECK_SUCCEEDS(a("51. getWatchedThreads"), p.reset(testee.getWatchedThreads(TalkUser::ListParameters())));
    a.checkEqual("52. getWatchedThreads", Access(p).getArraySize(), 1U);
    a.checkEqual("53. getWatchedThreads", Access(p)[0].toInteger(), 10);

    // Verify new state - use ListParameters for a change
    {
        TalkUser::ListParameters lp;
        lp.mode = lp.WantSize;
        AFL_CHECK_SUCCEEDS(a("61. getWatchedForums"), p.reset(testee.getWatchedForums(lp)));
        a.checkEqual("62. result", Access(p).toInteger(), 2);
    }

    // Mark a topic notified in DB, then unsubscribe it. This should reset the notification.
    {
        User(root, "1004").notifiedTopics().add(10);
        User(root, "1004").notifiedForums().add(9);
        static const TalkUser::Selection s[] = {
            { TalkUser::ForumScope,   9, 0 },
            { TalkUser::ThreadScope, 10, 0 },
        };
        AFL_CHECK_SUCCEEDS(a("71. unwatch"), testee.unwatch(s));

        a.check("81. notifiedTopics", !User(root, "1004").notifiedTopics().contains(10));
        a.check("82. notifiedForums", !User(root, "1004").notifiedForums().contains(9));
    }

    // Mark a forum notified in DB, then mark it seen.
    {
        User(root, "1004").notifiedForums().add(11);
        static const TalkUser::Selection s[] = {
            { TalkUser::ForumScope,  11, 0 },
        };
        AFL_CHECK_SUCCEEDS(a("91. markSeen"), testee.markSeen(s));
        a.check("92. notifiedForums", !User(root, "1004").notifiedForums().contains(11));
    }

    // Error case: cannot access ranges
    {
        static const TalkUser::Selection s[] = {
            { TalkUser::RangeScope, 3, 9 },
        };
        AFL_CHECK_THROWS(a("101. markSeen"), testee.markSeen(s), std::exception);
        AFL_CHECK_THROWS(a("102. watch"),    testee.watch(s), std::exception);
        AFL_CHECK_THROWS(a("103. unwatch"),  testee.unwatch(s), std::exception);
    }
}

/** Test getPostedMessages. */
AFL_TEST("server.talk.TalkUser:getPostedMessages", a)
{
    // Infrastructure
    InternalDatabase db;
    Root root(db, Configuration());

    // Preload DB
    User(root, "1002").postedMessages().add(9);
    User(root, "1002").postedMessages().add(10);
    User(root, "1002").postedMessages().add(12);

    // Access as root
    std::auto_ptr<afl::data::Value> p;
    {
        Session s;
        TalkUser testee(s, root);
        AFL_CHECK_SUCCEEDS(a("01. getPostedMessages"), p.reset(TalkUser(s, root).getPostedMessages("1002", TalkUser::ListParameters())));
        a.checkEqual("02. count", Access(p).getArraySize(), 3U);
        a.checkEqual("03. result", Access(p)[0].toInteger(), 9);
        a.checkEqual("04. result", Access(p)[1].toInteger(), 10);
        a.checkEqual("05. result", Access(p)[2].toInteger(), 12);
    }

    // Access as 1002
    {
        Session s;
        s.setUser("1002");
        TalkUser testee(s, root);
        AFL_CHECK_SUCCEEDS(a("11. getPostedMessages"), p.reset(TalkUser(s, root).getPostedMessages("1002", TalkUser::ListParameters())));
        a.checkEqual("12. count", Access(p).getArraySize(), 3U);
    }

    // Access as 1009
    {
        Session s;
        s.setUser("1009");
        TalkUser testee(s, root);
        AFL_CHECK_SUCCEEDS(a("21. getPostedMessages"), p.reset(TalkUser(s, root).getPostedMessages("1002", TalkUser::ListParameters())));
        a.checkEqual("22. count", Access(p).getArraySize(), 3U);
    }
}

/*
 *  Test getCrosspostToGameCandidates().
 */

namespace {
    struct CrossEnvironment {
        InternalDatabase db;
        Root root;
        Session session;

        CrossEnvironment(String_t user)
            : db(), root(db, Configuration()), session()
            {
                // Two users, one who can crosspost, one who can't
                User(root, "yes").profile().intField("allowgpost").set(1);    // User 'yes': on games, and allowed to cross-post
                User(root, "not").profile().intField("allowgpost").set(0);    // User 'not': on games but not allowed to cross-post
                User(root, "adm").profile().intField("allowgpost").set(1);    // User 'adm': not on games, but allowed
                // No profile for user 'und' - undefined user

                // Games
                for (int i = 1; i <= 5; ++i) {
                    // Game data
                    root.gameRoot().intSetKey("all").add(i);
                    root.gameRoot().subtree(i).stringKey("state").set("running");
                    root.gameRoot().subtree("state").intSetKey("running").add(i);
                    root.gameRoot().subtree("pubstate").intSetKey("running").add(i);

                    // Users on odd games
                    if ((i % 2) != 0) {
                        root.gameRoot().subtree(i).hashKey("users").intField("yes").set(1);
                        root.gameRoot().subtree(i).hashKey("users").intField("not").set(1);
                        root.gameRoot().subtree(i).hashKey("users").intField("und").set(1);
                    }

                    // Related forums
                    Forum f(root, i+10);
                    f.description().set(Format("forum:for [game]%d[/game]", i));
                    root.allForums().add(i+10);
                }

                // Session
                session.setUser(user);
            }
    };
}

// User 'yes', test all branches
AFL_TEST("server.talk.TalkUser:getCrosspostToGameCandidates:yes:all", a)
{
    CrossEnvironment env("yes");
    TalkUser t(env.session, env.root);

    TalkUser::ListParameters p;
    std::auto_ptr<afl::data::Value> result(t.getCrosspostToGameCandidates(p));
    a.checkEqual("01", Access(result).getArraySize(), 3U);
    a.checkEqual("02", Access(result)[0].toInteger(), 11);
    a.checkEqual("03", Access(result)[1].toInteger(), 13);
    a.checkEqual("04", Access(result)[2].toInteger(), 15);
}

AFL_TEST("server.talk.TalkUser:getCrosspostToGameCandidates:yes:all:sort", a)
{
    CrossEnvironment env("yes");
    Forum(env.root, 11).key().set("31");
    Forum(env.root, 12).key().set("45");
    Forum(env.root, 13).key().set("92");
    Forum(env.root, 14).key().set("65");
    Forum(env.root, 15).key().set("35");

    TalkUser t(env.session, env.root);

    TalkUser::ListParameters p;
    p.sortKey = "KEY";
    std::auto_ptr<afl::data::Value> result(t.getCrosspostToGameCandidates(p));
    a.checkEqual("01", Access(result).getArraySize(), 3U);
    a.checkEqual("02", Access(result)[0].toInteger(), 11);
    a.checkEqual("03", Access(result)[1].toInteger(), 15);
    a.checkEqual("04", Access(result)[2].toInteger(), 13);
}

AFL_TEST("server.talk.TalkUser:getCrosspostToGameCandidates:yes:range", a)
{
    CrossEnvironment env("yes");
    TalkUser t(env.session, env.root);

    TalkUser::ListParameters p;
    p.mode = TalkUser::ListParameters::WantRange;
    p.start = 2;
    p.count = 5;
    std::auto_ptr<afl::data::Value> result(t.getCrosspostToGameCandidates(p));
    a.checkEqual("01", Access(result).getArraySize(), 1U);
    a.checkEqual("02", Access(result)[0].toInteger(), 15);
}

AFL_TEST("server.talk.TalkUser:getCrosspostToGameCandidates:yes:beyond-range", a)
{
    CrossEnvironment env("yes");
    TalkUser t(env.session, env.root);

    TalkUser::ListParameters p;
    p.mode = TalkUser::ListParameters::WantRange;
    p.start = 20;
    std::auto_ptr<afl::data::Value> result(t.getCrosspostToGameCandidates(p));
    a.checkEqual("01", Access(result).getArraySize(), 0U);
}

AFL_TEST("server.talk.TalkUser:getCrosspostToGameCandidates:yes:short-size", a)
{
    CrossEnvironment env("yes");
    TalkUser t(env.session, env.root);

    TalkUser::ListParameters p;
    p.mode = TalkUser::ListParameters::WantRange;
    p.count = 2;
    std::auto_ptr<afl::data::Value> result(t.getCrosspostToGameCandidates(p));
    a.checkEqual("01", Access(result).getArraySize(), 2U);
    a.checkEqual("02", Access(result)[0].toInteger(), 11);
    a.checkEqual("03", Access(result)[1].toInteger(), 13);
}

AFL_TEST("server.talk.TalkUser:getCrosspostToGameCandidates:yes:size", a)
{
    CrossEnvironment env("yes");
    TalkUser t(env.session, env.root);

    TalkUser::ListParameters p;
    p.mode = TalkUser::ListParameters::WantSize;
    std::auto_ptr<afl::data::Value> result(t.getCrosspostToGameCandidates(p));
    Access r(result);
    a.checkEqual("01", Access(result).toInteger(), 3);
}

AFL_TEST("server.talk.TalkUser:getCrosspostToGameCandidates:yes:check", a)
{
    CrossEnvironment env("yes");
    TalkUser t(env.session, env.root);

    TalkUser::ListParameters p;
    p.mode = TalkUser::ListParameters::WantMemberCheck;
    p.item = 3;
    std::auto_ptr<afl::data::Value> result(t.getCrosspostToGameCandidates(p));
    Access r(result);
    a.checkEqual("01", Access(result).toInteger(), 1);
}

AFL_TEST("server.talk.TalkUser:getCrosspostToGameCandidates:yes:check-fail", a)
{
    CrossEnvironment env("yes");
    TalkUser t(env.session, env.root);

    TalkUser::ListParameters p;
    p.mode = TalkUser::ListParameters::WantMemberCheck;
    p.item = 2;
    std::auto_ptr<afl::data::Value> result(t.getCrosspostToGameCandidates(p));
    Access r(result);
    a.checkEqual("01", Access(result).toInteger(), 0);
}

// User 'adm', quick test
AFL_TEST("server.talk.TalkUser:getCrosspostToGameCandidates:adm:all", a)
{
    CrossEnvironment env("adm");
    TalkUser t(env.session, env.root);

    TalkUser::ListParameters p;
    std::auto_ptr<afl::data::Value> result(t.getCrosspostToGameCandidates(p));
    a.checkEqual("01", Access(result).getArraySize(), 0U);
}

AFL_TEST("server.talk.TalkUser:getCrosspostToGameCandidates:adm:size", a)
{
    CrossEnvironment env("adm");
    TalkUser t(env.session, env.root);

    TalkUser::ListParameters p;
    p.mode = TalkUser::ListParameters::WantSize;
    std::auto_ptr<afl::data::Value> result(t.getCrosspostToGameCandidates(p));
    Access r(result);
    a.checkEqual("01", Access(result).toInteger(), 0);
}

AFL_TEST("server.talk.TalkUser:getCrosspostToGameCandidates:adm:check", a)
{
    CrossEnvironment env("adm");
    TalkUser t(env.session, env.root);

    TalkUser::ListParameters p;
    p.mode = TalkUser::ListParameters::WantMemberCheck;
    p.item = 3;
    std::auto_ptr<afl::data::Value> result(t.getCrosspostToGameCandidates(p));
    Access r(result);
    a.checkEqual("01", Access(result).toInteger(), 0);
}

// User 'not', quick test
AFL_TEST("server.talk.TalkUser:getCrosspostToGameCandidates:not:all", a)
{
    CrossEnvironment env("not");
    TalkUser t(env.session, env.root);

    TalkUser::ListParameters p;
    std::auto_ptr<afl::data::Value> result(t.getCrosspostToGameCandidates(p));
    a.checkEqual("01", Access(result).getArraySize(), 0U);
}

AFL_TEST("server.talk.TalkUser:getCrosspostToGameCandidates:not:size", a)
{
    CrossEnvironment env("not");
    TalkUser t(env.session, env.root);

    TalkUser::ListParameters p;
    p.mode = TalkUser::ListParameters::WantSize;
    std::auto_ptr<afl::data::Value> result(t.getCrosspostToGameCandidates(p));
    Access r(result);
    a.checkEqual("01", Access(result).toInteger(), 0);
}

AFL_TEST("server.talk.TalkUser:getCrosspostToGameCandidates:not:check", a)
{
    CrossEnvironment env("not");
    TalkUser t(env.session, env.root);

    TalkUser::ListParameters p;
    p.mode = TalkUser::ListParameters::WantMemberCheck;
    p.item = 3;
    std::auto_ptr<afl::data::Value> result(t.getCrosspostToGameCandidates(p));
    Access r(result);
    a.checkEqual("01", Access(result).toInteger(), 0);
}

// User 'und', quick test
AFL_TEST("server.talk.TalkUser:getCrosspostToGameCandidates:und:all", a)
{
    CrossEnvironment env("und");
    TalkUser t(env.session, env.root);

    TalkUser::ListParameters p;
    std::auto_ptr<afl::data::Value> result(t.getCrosspostToGameCandidates(p));
    a.checkEqual("01", Access(result).getArraySize(), 0U);
}

AFL_TEST("server.talk.TalkUser:getCrosspostToGameCandidates:und:size", a)
{
    CrossEnvironment env("und");
    TalkUser t(env.session, env.root);

    TalkUser::ListParameters p;
    p.mode = TalkUser::ListParameters::WantSize;
    std::auto_ptr<afl::data::Value> result(t.getCrosspostToGameCandidates(p));
    Access r(result);
    a.checkEqual("01", Access(result).toInteger(), 0);
}

AFL_TEST("server.talk.TalkUser:getCrosspostToGameCandidates:und:check", a)
{
    CrossEnvironment env("und");
    TalkUser t(env.session, env.root);

    TalkUser::ListParameters p;
    p.mode = TalkUser::ListParameters::WantMemberCheck;
    p.item = 3;
    std::auto_ptr<afl::data::Value> result(t.getCrosspostToGameCandidates(p));
    Access r(result);
    a.checkEqual("01", Access(result).toInteger(), 0);
}
