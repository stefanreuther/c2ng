/**
  *  \file u/t_server_talk_talkgroup.cpp
  *  \brief Test for server::talk::TalkGroup
  */

#include "server/talk/talkgroup.hpp"

#include "t_server_talk.hpp"
#include "server/talk/root.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "server/talk/session.hpp"
#include "server/talk/group.hpp"

/** Simple tests. */
void
TestServerTalkTalkGroup::testIt()
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
        TS_ASSERT_THROWS(TalkGroup(userSession, root).add("root", d), std::exception);
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
        TS_ASSERT_THROWS(TalkGroup(rootSession, root).add("root", d), std::exception);

        // Configuring root works
        TS_ASSERT_THROWS_NOTHING(TalkGroup(rootSession, root).set("root", d));

        // ...but not as user
        TS_ASSERT_THROWS(TalkGroup(userSession, root).set("root", d), std::exception);

        // Configuring other fails because it does not exist
        TS_ASSERT_THROWS(TalkGroup(rootSession, root).set("other", d), std::exception);
    }

    // Query info
    TS_ASSERT_EQUALS(TalkGroup(rootSession, root).getField("root", "name"), "Root");
    TS_ASSERT_EQUALS(TalkGroup(rootSession, root).getField("root", "key"), "000-root");
    TS_ASSERT_EQUALS(TalkGroup(userSession, root).getField("root", "name"), "Root");
    TS_ASSERT_EQUALS(TalkGroup(rootSession, root).getField("unlisted", "description"), "text:Secret");

    // Query content
    {
        // Root queries root group
        afl::data::StringList_t groups;
        afl::data::IntegerList_t forums;
        TalkGroup(rootSession, root).list("root", groups, forums);
        TS_ASSERT_EQUALS(groups.size(), 1U);
        TS_ASSERT_EQUALS(groups[0], "sub");
        TS_ASSERT_EQUALS(forums.size(), 1U);
        TS_ASSERT_EQUALS(forums[0], 1);
    }
    {
        // User queries root group
        afl::data::StringList_t groups;
        afl::data::IntegerList_t forums;
        TalkGroup(userSession, root).list("root", groups, forums);
        TS_ASSERT_EQUALS(groups.size(), 1U);
        TS_ASSERT_EQUALS(groups[0], "sub");
        TS_ASSERT_EQUALS(forums.size(), 1U);
        TS_ASSERT_EQUALS(forums[0], 1);
    }
    {
        // Root queries unlisted group - root can do that
        afl::data::StringList_t groups;
        afl::data::IntegerList_t forums;
        TalkGroup(rootSession, root).list("unlisted", groups, forums);
        TS_ASSERT_EQUALS(groups.size(), 0U);
        TS_ASSERT_EQUALS(forums.size(), 1U);
        TS_ASSERT_EQUALS(forums[0], 3);
    }
    {
        // User queries unlisted group
        afl::data::StringList_t groups;
        afl::data::IntegerList_t forums;
        TalkGroup(userSession, root).list("unlisted", groups, forums);
        TS_ASSERT_EQUALS(groups.size(), 0U);
        TS_ASSERT_EQUALS(forums.size(), 0U);
    }

    // Get description; this renders, and also provides unlisted group headers.
    userSession.renderOptions().setFormat("html");
    {
        TalkGroup::Description desc = TalkGroup(userSession, root).getDescription("root");
        TS_ASSERT_EQUALS(desc.name.orElse("-"), "Root");
        TS_ASSERT_EQUALS(desc.description.orElse("-"), "<p>All forums</p>\n");
        TS_ASSERT_EQUALS(desc.parentGroup.orElse("-"), "");
        TS_ASSERT_EQUALS(desc.unlisted.orElse(true), false);
    }
    {
        TalkGroup::Description desc = TalkGroup(userSession, root).getDescription("unlisted");
        TS_ASSERT_EQUALS(desc.name.orElse("-"), "Unlisted forums");
        TS_ASSERT_EQUALS(desc.description.orElse("-"), "<p>Secret</p>\n");
        TS_ASSERT_EQUALS(desc.parentGroup.orElse("-"), "");
        TS_ASSERT_EQUALS(desc.unlisted.orElse(false), true);
    }

    // Same thing, multiple in one call
    {
        afl::data::StringList_t request;
        request.push_back("root");
        request.push_back("sub");
        request.push_back("unlisted");

        afl::container::PtrVector<TalkGroup::Description> result;
        TalkGroup(userSession, root).getDescriptions(request, result);

        TS_ASSERT_EQUALS(result.size(), 3U);
        TS_ASSERT(result[0] != 0);
        TS_ASSERT(result[1] != 0);
        TS_ASSERT(result[2] != 0);
        TS_ASSERT_EQUALS(result[0]->name.orElse("-"), "Root");
        TS_ASSERT_EQUALS(result[1]->name.orElse("-"), "Subgroup");
        TS_ASSERT_EQUALS(result[1]->description.orElse("-"), "<p>Some more forums</p>\n");
        TS_ASSERT_EQUALS(result[1]->parentGroup.orElse("-"), "root");
        TS_ASSERT_EQUALS(result[2]->name.orElse("-"), "Unlisted forums");
    }
}

