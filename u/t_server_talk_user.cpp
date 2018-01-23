/**
  *  \file u/t_server_talk_user.cpp
  *  \brief Test for server::talk::User
  */

#include <memory>
#include "server/talk/user.hpp"

#include "t_server_talk.hpp"
#include "server/talk/root.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/data/access.hpp"

/** Test basic properties. */
void
TestServerTalkUser::testBasicProperties()
{
    using afl::data::Access;

    // Prepare database
    afl::net::redis::InternalDatabase db;
    afl::net::NullCommandHandler mail;
    server::talk::Root root(db, mail, server::talk::Configuration());

    const char* UID = "1009";
    afl::net::redis::Subtree userTree = root.userRoot().subtree(UID);
    userTree.hashKey("profile").stringField("screenname").set("the screen name");
    userTree.stringKey("name").set("the_login_name");
    userTree.subtree("forum").intSetKey("posted").add(42);
    userTree.stringKey("password").set("12345");     // That's the stupidest combination I've ever heard of in my life! That's the kinda thing an idiot would have on his luggage!
    userTree.subtree("pm:folder").intKey("id").set(105);
    userTree.subtree("pm:folder").intSetKey("all").add(103);
    userTree.subtree("forum").intSetKey("watchedForums").add(99);
    userTree.subtree("forum").intSetKey("watchedThreads").add(77);
    userTree.subtree("forum").intSetKey("notifiedForums").add(98);
    userTree.subtree("forum").intSetKey("notifiedThreads").add(76);

    userTree.hashKey("profile").stringField("userfield").set("uservalue");
    userTree.hashKey("profile").intField("userint").set(0);

    afl::net::redis::HashKey defaultKey = root.defaultProfile();
    defaultKey.stringField("userfield").set("defaultuservalue");
    defaultKey.intField("userint").set(1);
    defaultKey.stringField("defaultfield").set("defaultvalue");
    defaultKey.intField("defaultint").set(2);

    // Test accessors
    server::talk::User testee(root, UID);
    TS_ASSERT_EQUALS(testee.getScreenName(), "the screen name");
    TS_ASSERT_EQUALS(testee.getLoginName(), "the_login_name");
    TS_ASSERT(testee.postedMessages().contains(42));
    TS_ASSERT_EQUALS(testee.passwordHash().get(), "12345");  // That's amazing! I've got the same combination on my luggage!

    TS_ASSERT_EQUALS(testee.pmFolderCount().get(), 105);
    TS_ASSERT(testee.pmFolders().contains(103));
    TS_ASSERT(testee.watchedForums().contains(99));
    TS_ASSERT(testee.watchedTopics().contains(77));
    TS_ASSERT(testee.notifiedForums().contains(98));
    TS_ASSERT(testee.notifiedTopics().contains(76));

    std::auto_ptr<afl::data::Value> p;
    p.reset(testee.getProfileRaw("userfield"));
    TS_ASSERT_EQUALS(Access(p).toString(), "uservalue");
    p.reset(testee.getProfileRaw("userint"));
    TS_ASSERT_EQUALS(Access(p).toInteger(), 0);
    p.reset(testee.getProfileRaw("defaultfield"));
    TS_ASSERT_EQUALS(Access(p).toString(), "defaultvalue");
    p.reset(testee.getProfileRaw("defaultint"));
    TS_ASSERT_EQUALS(Access(p).toInteger(), 2);
}

/** Test getPMMailType(). */
void
TestServerTalkUser::testMailPMType()
{
    afl::net::NullCommandHandler mail;

    // Not set
    {
        afl::net::redis::InternalDatabase db;
        server::talk::Root root(db, mail, server::talk::Configuration());
        server::talk::User testee(root, "1001");
        TS_ASSERT_EQUALS(testee.getPMMailType(), "");
    }

    // Set in user profile
    {
        afl::net::redis::InternalDatabase db;
        server::talk::Root root(db, mail, server::talk::Configuration());
        root.userRoot().subtree("1001").hashKey("profile").stringField("mailpmtype").set("a");
        server::talk::User testee(root, "1001");
        TS_ASSERT_EQUALS(testee.getPMMailType(), "a");
    }

    // Set in default profile
    {
        afl::net::redis::InternalDatabase db;
        server::talk::Root root(db, mail, server::talk::Configuration());
        root.defaultProfile().stringField("mailpmtype").set("b");
        server::talk::User testee(root, "1001");
        TS_ASSERT_EQUALS(testee.getPMMailType(), "b");
    }

    // Set in both
    {
        afl::net::redis::InternalDatabase db;
        server::talk::Root root(db, mail, server::talk::Configuration());
        root.userRoot().subtree("1001").hashKey("profile").stringField("mailpmtype").set("a");
        root.defaultProfile().stringField("mailpmtype").set("b");
        server::talk::User testee(root, "1001");
        TS_ASSERT_EQUALS(testee.getPMMailType(), "a");
    }

    // Set in both, blank in user profile
    {
        afl::net::redis::InternalDatabase db;
        server::talk::Root root(db, mail, server::talk::Configuration());
        root.userRoot().subtree("1001").hashKey("profile").stringField("mailpmtype").set("");
        root.defaultProfile().stringField("mailpmtype").set("b");
        server::talk::User testee(root, "1001");
        TS_ASSERT_EQUALS(testee.getPMMailType(), "");
    }
}

/** Test isAutoWatch(). */
void
TestServerTalkUser::testAutowatch()
{
    afl::net::NullCommandHandler mail;

    // Not set; default means yes
    {
        afl::net::redis::InternalDatabase db;
        server::talk::Root root(db, mail, server::talk::Configuration());
        server::talk::User testee(root, "1001");
        TS_ASSERT(testee.isAutoWatch());
    }

    // Enabled in user profile
    {
        afl::net::redis::InternalDatabase db;
        server::talk::Root root(db, mail, server::talk::Configuration());
        root.userRoot().subtree("1001").hashKey("profile").intField("talkautowatch").set(1);
        server::talk::User testee(root, "1001");
        TS_ASSERT(testee.isAutoWatch());
    }

    // Disabled in user profile
    {
        afl::net::redis::InternalDatabase db;
        server::talk::Root root(db, mail, server::talk::Configuration());
        root.userRoot().subtree("1001").hashKey("profile").intField("talkautowatch").set(0);
        server::talk::User testee(root, "1001");
        TS_ASSERT(!testee.isAutoWatch());
    }

    // Enabled in default profile
    {
        afl::net::redis::InternalDatabase db;
        server::talk::Root root(db, mail, server::talk::Configuration());
        root.defaultProfile().intField("talkautowatch").set(1);
        server::talk::User testee(root, "1001");
        TS_ASSERT(testee.isAutoWatch());
    }

    // Disabled in default profile
    {
        afl::net::redis::InternalDatabase db;
        server::talk::Root root(db, mail, server::talk::Configuration());
        root.defaultProfile().intField("talkautowatch").set(0);
        server::talk::User testee(root, "1001");
        TS_ASSERT(!testee.isAutoWatch());
    }

    // Enabled in user, disabled in default
    {
        afl::net::redis::InternalDatabase db;
        server::talk::Root root(db, mail, server::talk::Configuration());
        root.userRoot().subtree("1001").hashKey("profile").intField("talkautowatch").set(1);
        root.defaultProfile().intField("talkautowatch").set(0);
        server::talk::User testee(root, "1001");
        TS_ASSERT(testee.isAutoWatch());
    }

    // Disabled in user, enabled in default
    {
        afl::net::redis::InternalDatabase db;
        server::talk::Root root(db, mail, server::talk::Configuration());
        root.userRoot().subtree("1001").hashKey("profile").intField("talkautowatch").set(0);
        root.defaultProfile().intField("talkautowatch").set(1);
        server::talk::User testee(root, "1001");
        TS_ASSERT(!testee.isAutoWatch());
    }
}

/** Test isWatchIndividual(). */
void
TestServerTalkUser::testWatchIndividual()
{
    afl::net::NullCommandHandler mail;

    // Not set; default means no
    {
        afl::net::redis::InternalDatabase db;
        server::talk::Root root(db, mail, server::talk::Configuration());
        server::talk::User testee(root, "1001");
        TS_ASSERT(!testee.isWatchIndividual());
    }

    // Enabled in user profile
    {
        afl::net::redis::InternalDatabase db;
        server::talk::Root root(db, mail, server::talk::Configuration());
        root.userRoot().subtree("1001").hashKey("profile").intField("talkwatchindividual").set(1);
        server::talk::User testee(root, "1001");
        TS_ASSERT(testee.isWatchIndividual());
    }

    // Disabled in user profile
    {
        afl::net::redis::InternalDatabase db;
        server::talk::Root root(db, mail, server::talk::Configuration());
        root.userRoot().subtree("1001").hashKey("profile").intField("talkwatchindividual").set(0);
        server::talk::User testee(root, "1001");
        TS_ASSERT(!testee.isWatchIndividual());
    }

    // Enabled in default profile
    {
        afl::net::redis::InternalDatabase db;
        server::talk::Root root(db, mail, server::talk::Configuration());
        root.defaultProfile().intField("talkwatchindividual").set(1);
        server::talk::User testee(root, "1001");
        TS_ASSERT(testee.isWatchIndividual());
    }

    // Disabled in default profile
    {
        afl::net::redis::InternalDatabase db;
        server::talk::Root root(db, mail, server::talk::Configuration());
        root.defaultProfile().intField("talkwatchindividual").set(0);
        server::talk::User testee(root, "1001");
        TS_ASSERT(!testee.isWatchIndividual());
    }

    // Enabled in user, disabled in default
    {
        afl::net::redis::InternalDatabase db;
        server::talk::Root root(db, mail, server::talk::Configuration());
        root.userRoot().subtree("1001").hashKey("profile").intField("talkwatchindividual").set(1);
        root.defaultProfile().intField("talkwatchindividual").set(0);
        server::talk::User testee(root, "1001");
        TS_ASSERT(testee.isWatchIndividual());
    }

    // Disabled in user, enabled in default
    {
        afl::net::redis::InternalDatabase db;
        server::talk::Root root(db, mail, server::talk::Configuration());
        root.userRoot().subtree("1001").hashKey("profile").intField("talkwatchindividual").set(0);
        root.defaultProfile().intField("talkwatchindividual").set(1);
        server::talk::User testee(root, "1001");
        TS_ASSERT(!testee.isWatchIndividual());
    }
}

