/**
  *  \file test/server/talk/talkgrouptest.cpp
  *  \brief Test for server::talk::TalkGroup
  */

#include "server/talk/talkgroup.hpp"

#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/test/testrunner.hpp"
#include "server/talk/group.hpp"
#include "server/talk/root.hpp"
#include "server/talk/session.hpp"

/** Simple tests. */
AFL_TEST("server.talk.TalkGroup:basics", a)
{
    using server::talk::TalkGroup;

    // Infrastructure
    afl::net::redis::InternalDatabase db;
    afl::net::NullCommandHandler mq;
    server::talk::Root root(db, mq, server::talk::Configuration());
    server::talk::Session rootSession;
    server::talk::Session userSession;
    userSession.setUser("a");

    // Create some groups
    {
        // A root group
        TalkGroup::Description d;
        d.name = "All";
        d.description = "text:All forums";
        d.key = "000-root";
        TalkGroup(rootSession, root).add("root", d);
    }
    {
        // A subgroup
        TalkGroup::Description d;
        d.name = "Subgroup";
        d.description = "text:Some more forums";
        d.parentGroup = "root";
        TalkGroup(rootSession, root).add("sub", d);
    }
    {
        // An unlisted group
        TalkGroup::Description d;
        d.name = "Unlisted forums";
        d.description = "text:Secret";
        d.unlisted = true;
        TalkGroup(rootSession, root).add("unlisted", d);
    }
    {
        // User creating a group - fails, users cannot do that
        TalkGroup::Description d;
        d.name = "My";
        d.description = "text:My forums";
        AFL_CHECK_THROWS(a("01. add"), TalkGroup(userSession, root).add("root", d), std::exception);
    }

    // Add some forums [just for testing]
    server::talk::Group(root, "root").forums().add(1);
    server::talk::Group(root, "sub").forums().add(2);
    server::talk::Group(root, "unlisted").forums().add(3);

    // Configure
    {
        TalkGroup::Description d;
        d.name = "Root";

        // Adding root fails because it already exists
        AFL_CHECK_THROWS(a("11. add dup"), TalkGroup(rootSession, root).add("root", d), std::exception);

        // Configuring root works
        AFL_CHECK_SUCCEEDS(a("21. set"), TalkGroup(rootSession, root).set("root", d));

        // ...but not as user
        AFL_CHECK_THROWS(a("31. set as user"), TalkGroup(userSession, root).set("root", d), std::exception);

        // Configuring other fails because it does not exist
        AFL_CHECK_THROWS(a("41. set nonexistant"), TalkGroup(rootSession, root).set("other", d), std::exception);
    }

    // Query info
    a.checkEqual("51. getField", TalkGroup(rootSession, root).getField("root", "name"), "Root");
    a.checkEqual("52. getField", TalkGroup(rootSession, root).getField("root", "key"), "000-root");
    a.checkEqual("53. getField", TalkGroup(userSession, root).getField("root", "name"), "Root");
    a.checkEqual("54. getField", TalkGroup(rootSession, root).getField("unlisted", "description"), "text:Secret");

    // Query content
    {
        // Root queries root group
        afl::data::StringList_t groups;
        afl::data::IntegerList_t forums;
        TalkGroup(rootSession, root).list("root", groups, forums);
        a.checkEqual("61. size",  groups.size(), 1U);
        a.checkEqual("62. group", groups[0], "sub");
        a.checkEqual("63. size",  forums.size(), 1U);
        a.checkEqual("64. forum", forums[0], 1);
    }
    {
        // User queries root group
        afl::data::StringList_t groups;
        afl::data::IntegerList_t forums;
        TalkGroup(userSession, root).list("root", groups, forums);
        a.checkEqual("65. size",  groups.size(), 1U);
        a.checkEqual("66. group", groups[0], "sub");
        a.checkEqual("67. size",  forums.size(), 1U);
        a.checkEqual("68. forum", forums[0], 1);
    }
    {
        // Root queries unlisted group - root can do that
        afl::data::StringList_t groups;
        afl::data::IntegerList_t forums;
        TalkGroup(rootSession, root).list("unlisted", groups, forums);
        a.checkEqual("69. size",  groups.size(), 0U);
        a.checkEqual("70. size",  forums.size(), 1U);
        a.checkEqual("71. forum", forums[0], 3);
    }
    {
        // User queries unlisted group
        afl::data::StringList_t groups;
        afl::data::IntegerList_t forums;
        TalkGroup(userSession, root).list("unlisted", groups, forums);
        a.checkEqual("72. size", groups.size(), 0U);
        a.checkEqual("73. size", forums.size(), 0U);
    }

    // Get description; this renders, and also provides unlisted group headers.
    userSession.renderOptions().setFormat("html");
    {
        TalkGroup::Description desc = TalkGroup(userSession, root).getDescription("root");
        a.checkEqual("81. name",        desc.name.orElse("-"), "Root");
        a.checkEqual("82. description", desc.description.orElse("-"), "<p>All forums</p>\n");
        a.checkEqual("83. parentGroup", desc.parentGroup.orElse("-"), "");
        a.checkEqual("84. unlisted",    desc.unlisted.orElse(true), false);
    }
    {
        TalkGroup::Description desc = TalkGroup(userSession, root).getDescription("unlisted");
        a.checkEqual("85. name",        desc.name.orElse("-"), "Unlisted forums");
        a.checkEqual("86. description", desc.description.orElse("-"), "<p>Secret</p>\n");
        a.checkEqual("87. parentGroup", desc.parentGroup.orElse("-"), "");
        a.checkEqual("88. unlisted",    desc.unlisted.orElse(false), true);
    }

    // Same thing, multiple in one call
    {
        afl::data::StringList_t request;
        request.push_back("root");
        request.push_back("sub");
        request.push_back("unlisted");

        afl::container::PtrVector<TalkGroup::Description> result;
        TalkGroup(userSession, root).getDescriptions(request, result);

        a.checkEqual("91. size", result.size(), 3U);
        a.checkNonNull("92. result",    result[0]);
        a.checkNonNull("93. result",    result[1]);
        a.checkNonNull("94. result",    result[2]);
        a.checkEqual("95. name",        result[0]->name.orElse("-"), "Root");
        a.checkEqual("96. name",        result[1]->name.orElse("-"), "Subgroup");
        a.checkEqual("97. description", result[1]->description.orElse("-"), "<p>Some more forums</p>\n");
        a.checkEqual("98. parentGroup", result[1]->parentGroup.orElse("-"), "root");
        a.checkEqual("99. name",        result[2]->name.orElse("-"), "Unlisted forums");
    }
}
