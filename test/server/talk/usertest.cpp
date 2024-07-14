/**
  *  \file test/server/talk/usertest.cpp
  *  \brief Test for server::talk::User
  */

#include "server/talk/user.hpp"

#include "afl/data/access.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/test/testrunner.hpp"
#include "server/talk/root.hpp"
#include <memory>

namespace {
    struct Environment {
        afl::net::redis::InternalDatabase db;
        server::talk::Root root;

        Environment()
            : db(), root(db, server::talk::Configuration())
            { }
    };
}

/** Test basic properties. */
AFL_TEST("server.talk.User:basics", a)
{
    using afl::data::Access;

    // Prepare database
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, server::talk::Configuration());

    const char* UID = "1009";
    afl::net::redis::Subtree userTree = root.userRoot().subtree(UID);
    userTree.hashKey("profile").stringField("screenname").set("the screen name");
    userTree.stringKey("name").set("the_login_name");
    userTree.subtree("forum").intSetKey("posted").add(42);
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
    a.checkEqual("01. getScreenName", testee.getScreenName(), "the screen name");
    a.checkEqual("02. getLoginName",  testee.getLoginName(), "the_login_name");
    a.check    ("03. postedMessages", testee.postedMessages().contains(42));

    a.checkEqual("11. pmFolderCount",  testee.pmFolderCount().get(), 105);
    a.check     ("12. pmFolders",      testee.pmFolders().contains(103));
    a.check     ("13. watchedForums",  testee.watchedForums().contains(99));
    a.check     ("14. watchedTopics",  testee.watchedTopics().contains(77));
    a.check     ("15. notifiedForums", testee.notifiedForums().contains(98));
    a.check     ("16. notifiedTopics", testee.notifiedTopics().contains(76));

    std::auto_ptr<afl::data::Value> p;
    p.reset(testee.getProfileRaw("userfield"));
    a.checkEqual("21. userfield", Access(p).toString(), "uservalue");
    p.reset(testee.getProfileRaw("userint"));
    a.checkEqual("22. userint", Access(p).toInteger(), 0);
    p.reset(testee.getProfileRaw("defaultfield"));
    a.checkEqual("23. defaultfield", Access(p).toString(), "defaultvalue");
    p.reset(testee.getProfileRaw("defaultint"));
    a.checkEqual("24. defaultint", Access(p).toInteger(), 2);

    testee.rateTime().set(99);
    testee.rateScore().set(77);
    a.checkEqual("31. rateTime", testee.rateTime().get(), 99);
    a.checkEqual("32. rateScore", testee.rateScore().get(), 77);
}

/*
 *  Test getPMMailType().
 */

// Not set
AFL_TEST("server.talk.User:getPMMailType:not-set", a)
{
    Environment env;
    server::talk::User testee(env.root, "1001");
    a.checkEqual("", testee.getPMMailType(), "");
}

// Set in user profile
AFL_TEST("server.talk.User:getPMMailType:user-profile", a)
{
    Environment env;
    env.root.userRoot().subtree("1001").hashKey("profile").stringField("mailpmtype").set("a");
    server::talk::User testee(env.root, "1001");
    a.checkEqual("", testee.getPMMailType(), "a");
}

// Set in default profile
AFL_TEST("server.talk.User:getPMMailType:default-profile", a)
{
    Environment env;
    env.root.defaultProfile().stringField("mailpmtype").set("b");
    server::talk::User testee(env.root, "1001");
    a.checkEqual("", testee.getPMMailType(), "b");
}

// Set in both
AFL_TEST("server.talk.User:getPMMailType:both", a)
{
    Environment env;
    env.root.userRoot().subtree("1001").hashKey("profile").stringField("mailpmtype").set("a");
    env.root.defaultProfile().stringField("mailpmtype").set("b");
    server::talk::User testee(env.root, "1001");
    a.checkEqual("", testee.getPMMailType(), "a");
}

// Set in both, blank in user profile
AFL_TEST("server.talk.User:getPMMailType:blank-in-user-profile", a)
{
    Environment env;
    env.root.userRoot().subtree("1001").hashKey("profile").stringField("mailpmtype").set("");
    env.root.defaultProfile().stringField("mailpmtype").set("b");
    server::talk::User testee(env.root, "1001");
    a.checkEqual("", testee.getPMMailType(), "");
}

/*
 *  Test isAutoWatch().
 */

// Not set; default means yes
AFL_TEST("server.talk.User:isAutoWatch:not-set", a)
{
    Environment env;
    server::talk::User testee(env.root, "1001");
    a.check("", testee.isAutoWatch());
}

// Enabled in user profile
AFL_TEST("server.talk.User:isAutoWatch:enabled-in-profile", a)
{
    Environment env;
    env.root.userRoot().subtree("1001").hashKey("profile").intField("talkautowatch").set(1);
    server::talk::User testee(env.root, "1001");
    a.check("", testee.isAutoWatch());
}

// Disabled in user profile
AFL_TEST("server.talk.User:isAutoWatch:disabled-in-profile", a)
{
    Environment env;
    env.root.userRoot().subtree("1001").hashKey("profile").intField("talkautowatch").set(0);
    server::talk::User testee(env.root, "1001");
    a.check("", !testee.isAutoWatch());
}

// Enabled in default profile
AFL_TEST("server.talk.User:isAutoWatch:enabled-in-default", a)
{
    Environment env;
    env.root.defaultProfile().intField("talkautowatch").set(1);
    server::talk::User testee(env.root, "1001");
    a.check("", testee.isAutoWatch());
}

// Disabled in default profile
AFL_TEST("server.talk.User:isAutoWatch:disabled-in-default", a)
{
    Environment env;
    env.root.defaultProfile().intField("talkautowatch").set(0);
    server::talk::User testee(env.root, "1001");
    a.check("", !testee.isAutoWatch());
}

// Enabled in user, disabled in default
AFL_TEST("server.talk.User:isAutoWatch:enabled-in-profile-disabled-in-default", a)
{
    Environment env;
    env.root.userRoot().subtree("1001").hashKey("profile").intField("talkautowatch").set(1);
    env.root.defaultProfile().intField("talkautowatch").set(0);
    server::talk::User testee(env.root, "1001");
    a.check("", testee.isAutoWatch());
}

// Disabled in user, enabled in default
AFL_TEST("server.talk.User:isAutoWatch:disabled-in-profile-enabled-in-default", a)
{
    Environment env;
    env.root.userRoot().subtree("1001").hashKey("profile").intField("talkautowatch").set(0);
    env.root.defaultProfile().intField("talkautowatch").set(1);
    server::talk::User testee(env.root, "1001");
    a.check("", !testee.isAutoWatch());
}


/*
 *  Test isAllowedToPost().
 */

// Not set; default means yes
AFL_TEST("server.talk.User:isAllowedToPost:not-set", a)
{
    Environment env;
    server::talk::User testee(env.root, "1001");
    a.check("", testee.isAllowedToPost());
}

// Enabled in user profile
AFL_TEST("server.talk.User:isAllowedToPost:enabled-in-profile", a)
{
    Environment env;
    env.root.userRoot().subtree("1001").hashKey("profile").intField("allowpost").set(1);
    server::talk::User testee(env.root, "1001");
    a.check("", testee.isAllowedToPost());
}

// Disabled in user profile
AFL_TEST("server.talk.User:isAllowedToPost:disabled-in-profile", a)
{
    Environment env;
    env.root.userRoot().subtree("1001").hashKey("profile").intField("allowpost").set(0);
    server::talk::User testee(env.root, "1001");
    a.check("", !testee.isAllowedToPost());
}

// Enabled in default profile
AFL_TEST("server.talk.User:isAllowedToPost:enabled-in-default", a)
{
    Environment env;
    env.root.defaultProfile().intField("allowpost").set(1);
    server::talk::User testee(env.root, "1001");
    a.check("", testee.isAllowedToPost());
}

// Disabled in default profile
AFL_TEST("server.talk.User:isAllowedToPost:disabled-in-default", a)
{
    Environment env;
    env.root.defaultProfile().intField("allowpost").set(0);
    server::talk::User testee(env.root, "1001");
    a.check("", !testee.isAllowedToPost());
}

// Enabled in user, disabled in default
AFL_TEST("server.talk.User:isAllowedToPost:enabled-in-profile-disabled-in-default", a)
{
    Environment env;
    env.root.userRoot().subtree("1001").hashKey("profile").intField("allowpost").set(1);
    env.root.defaultProfile().intField("allowpost").set(0);
    server::talk::User testee(env.root, "1001");
    a.check("", testee.isAllowedToPost());
}

// Disabled in user, enabled in default
AFL_TEST("server.talk.User:isAllowedToPost:disabled-in-profile-enabled-in-default", a)
{
    Environment env;
    env.root.userRoot().subtree("1001").hashKey("profile").intField("allowpost").set(0);
    env.root.defaultProfile().intField("allowpost").set(1);
    server::talk::User testee(env.root, "1001");
    a.check("", !testee.isAllowedToPost());
}


/*
 *  Test isAllowedToSendPMs().
 */

// Not set; default means yes
AFL_TEST("server.talk.User:isAllowedToSendPMs:not-set", a)
{
    Environment env;
    server::talk::User testee(env.root, "1001");
    a.check("", testee.isAllowedToSendPMs());
}

// Enabled in user profile
AFL_TEST("server.talk.User:isAllowedToSendPMs:enabled-in-profile", a)
{
    Environment env;
    env.root.userRoot().subtree("1001").hashKey("profile").intField("allowpm").set(1);
    server::talk::User testee(env.root, "1001");
    a.check("", testee.isAllowedToSendPMs());
}

// Disabled in user profile
AFL_TEST("server.talk.User:isAllowedToSendPMs:disabled-in-profile", a)
{
    Environment env;
    env.root.userRoot().subtree("1001").hashKey("profile").intField("allowpm").set(0);
    server::talk::User testee(env.root, "1001");
    a.check("", !testee.isAllowedToSendPMs());
}

// Enabled in default profile
AFL_TEST("server.talk.User:isAllowedToSendPMs:enabled-in-default", a)
{
    Environment env;
    env.root.defaultProfile().intField("allowpm").set(1);
    server::talk::User testee(env.root, "1001");
    a.check("", testee.isAllowedToSendPMs());
}

// Disabled in default profile
AFL_TEST("server.talk.User:isAllowedToSendPMs:disabled-in-default", a)
{
    Environment env;
    env.root.defaultProfile().intField("allowpm").set(0);
    server::talk::User testee(env.root, "1001");
    a.check("", !testee.isAllowedToSendPMs());
}

// Enabled in user, disabled in default
AFL_TEST("server.talk.User:isAllowedToSendPMs:enabled-in-profile-disabled-in-default", a)
{
    Environment env;
    env.root.userRoot().subtree("1001").hashKey("profile").intField("allowpm").set(1);
    env.root.defaultProfile().intField("allowpm").set(0);
    server::talk::User testee(env.root, "1001");
    a.check("", testee.isAllowedToSendPMs());
}

// Disabled in user, enabled in default
AFL_TEST("server.talk.User:isAllowedToSendPMs:disabled-in-profile-enabled-in-default", a)
{
    Environment env;
    env.root.userRoot().subtree("1001").hashKey("profile").intField("allowpm").set(0);
    env.root.defaultProfile().intField("allowpm").set(1);
    server::talk::User testee(env.root, "1001");
    a.check("", !testee.isAllowedToSendPMs());
}


/*
 *  Test isWatchIndividual().
 */

// Not set; default means no
AFL_TEST("server.talk.User:isWatchIndividual:not-set", a)
{
    Environment env;
    server::talk::User testee(env.root, "1001");
    a.check("", !testee.isWatchIndividual());
}

// Enabled in user profile
AFL_TEST("server.talk.User:isWatchIndividual:enabled-in-profile", a)
{
    Environment env;
    env.root.userRoot().subtree("1001").hashKey("profile").intField("talkwatchindividual").set(1);
    server::talk::User testee(env.root, "1001");
    a.check("", testee.isWatchIndividual());
}

// Disabled in user profile
AFL_TEST("server.talk.User:isWatchIndividual:disabled-in-profile", a)
{
    Environment env;
    env.root.userRoot().subtree("1001").hashKey("profile").intField("talkwatchindividual").set(0);
    server::talk::User testee(env.root, "1001");
    a.check("", !testee.isWatchIndividual());
}

// Enabled in default profile
AFL_TEST("server.talk.User:isWatchIndividual:enabled-in-default", a)
{
    Environment env;
    env.root.defaultProfile().intField("talkwatchindividual").set(1);
    server::talk::User testee(env.root, "1001");
    a.check("", testee.isWatchIndividual());
}

// Disabled in default profile
AFL_TEST("server.talk.User:isWatchIndividual:disabled-in-default", a)
{
    Environment env;
    env.root.defaultProfile().intField("talkwatchindividual").set(0);
    server::talk::User testee(env.root, "1001");
    a.check("", !testee.isWatchIndividual());
}

// Enabled in user, disabled in default
AFL_TEST("server.talk.User:isWatchIndividual:enabled-in-profile-disabled-in-default", a)
{
    Environment env;
    env.root.userRoot().subtree("1001").hashKey("profile").intField("talkwatchindividual").set(1);
    env.root.defaultProfile().intField("talkwatchindividual").set(0);
    server::talk::User testee(env.root, "1001");
    a.check("", testee.isWatchIndividual());
}

// Disabled in user, enabled in default
AFL_TEST("server.talk.User:isWatchIndividual:disabled-in-profile-enabled-in-default", a)
{
    Environment env;
    env.root.userRoot().subtree("1001").hashKey("profile").intField("talkwatchindividual").set(0);
    env.root.defaultProfile().intField("talkwatchindividual").set(1);
    server::talk::User testee(env.root, "1001");
    a.check("", !testee.isWatchIndividual());
}
