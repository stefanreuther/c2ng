/**
  *  \file u/t_server_interface_talkgroupclient.cpp
  *  \brief Test for server::interface::TalkGroupClient
  */

#include "server/interface/talkgroupclient.hpp"

#include "t_server_interface.hpp"
#include "u/helper/commandhandlermock.hpp"
#include "server/types.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/data/segment.hpp"

using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Vector;
using afl::data::VectorValue;
using afl::data::Segment;

void
TestServerInterfaceTalkGroupClient::testIt()
{
    CommandHandlerMock mock;
    server::interface::TalkGroupClient testee(mock);

    // add
    {
        mock.expectCall("GROUPADD|g");
        mock.provideReturnValue(0);
        testee.add("g", server::interface::TalkGroupClient::Description());
    }
    {
        mock.expectCall("GROUPADD|g2|name|theName|description|theDescription|parent|theParent|unlisted|0");
        mock.provideReturnValue(0);

        server::interface::TalkGroupClient::Description desc;
        desc.name = "theName";
        desc.description = "theDescription";
        desc.parentGroup = "theParent";
        desc.unlisted = false;
        testee.add("g2", desc);
    }

    // set
    {
        mock.expectCall("GROUPSET|g");
        mock.provideReturnValue(0);
        testee.set("g", server::interface::TalkGroupClient::Description());
    }
    {
        mock.expectCall("GROUPSET|g2|name|theName|description|theDescription|parent|theParent|unlisted|0");
        mock.provideReturnValue(0);

        server::interface::TalkGroupClient::Description desc;
        desc.name = "theName";
        desc.description = "theDescription";
        desc.parentGroup = "theParent";
        desc.unlisted = false;
        testee.set("g2", desc);
    }

    // getField
    mock.expectCall("GROUPGET|g|name");
    mock.provideReturnValue(server::makeStringValue("theName"));
    TS_ASSERT_EQUALS(testee.getField("g", "name"), "theName");

    // list
    {
        // Return null - should produce no groups/forums
        mock.expectCall("GROUPLS|g");
        mock.provideReturnValue(0);

        afl::data::StringList_t groups;
        afl::data::IntegerList_t forums;
        testee.list("g", groups, forums);

        TS_ASSERT_EQUALS(groups.size(), 0U);
        TS_ASSERT_EQUALS(forums.size(), 0U);
    }
    {
        // Return proper values
        Hash::Ref_t h = Hash::create();
        h->setNew("groups", new VectorValue(Vector::create(Segment().pushBackString("sub1").pushBackString("sub2"))));
        h->setNew("forums", new VectorValue(Vector::create(Segment().pushBackInteger(32).pushBackInteger(16).pushBackInteger(8))));
        mock.expectCall("GROUPLS|top");
        mock.provideReturnValue(new HashValue(h));

        afl::data::StringList_t groups;
        afl::data::IntegerList_t forums;
        testee.list("top", groups, forums);

        TS_ASSERT_EQUALS(groups.size(), 2U);
        TS_ASSERT_EQUALS(groups[0], "sub1");
        TS_ASSERT_EQUALS(groups[1], "sub2");
        TS_ASSERT_EQUALS(forums.size(), 3U);
        TS_ASSERT_EQUALS(forums[0], 32);
        TS_ASSERT_EQUALS(forums[1], 16);
        TS_ASSERT_EQUALS(forums[2], 8);
    }

    // getDescription
    {
        // Return null
        mock.expectCall("GROUPSTAT|gg");
        mock.provideReturnValue(0);

        server::interface::TalkGroup::Description desc = testee.getDescription("gg");

        TS_ASSERT(!desc.name.isValid());
        TS_ASSERT(!desc.description.isValid());
        TS_ASSERT(!desc.parentGroup.isValid());
        TS_ASSERT(!desc.unlisted.isValid());
    }
    {
        // Return non-null
        Hash::Ref_t h = Hash::create();
        h->setNew("name", server::makeStringValue("The Name"));
        h->setNew("description", server::makeStringValue("This is the description"));
        h->setNew("parent", server::makeStringValue("parent"));
        h->setNew("unlisted", server::makeIntegerValue(1));
        mock.expectCall("GROUPSTAT|gg2");
        mock.provideReturnValue(new HashValue(h));

        server::interface::TalkGroup::Description desc = testee.getDescription("gg2");

        TS_ASSERT(desc.name.get() != 0);
        TS_ASSERT_EQUALS(*desc.name.get(), "The Name");
        TS_ASSERT(desc.description.get() != 0);
        TS_ASSERT_EQUALS(*desc.description.get(), "This is the description");
        TS_ASSERT(desc.parentGroup.get() != 0);
        TS_ASSERT_EQUALS(*desc.parentGroup.get(), "parent");
        TS_ASSERT(desc.unlisted.get() != 0);
        TS_ASSERT_EQUALS(*desc.unlisted.get(), true);
    }

    // getDescriptions
    {
        Hash::Ref_t h = Hash::create();
        h->setNew("name", server::makeStringValue("n"));
        h->setNew("description", server::makeStringValue("d"));
        h->setNew("parent", server::makeStringValue("p"));

        Vector::Ref_t vec = Vector::create();
        vec->pushBackNew(0);
        vec->pushBackNew(new HashValue(h));
        mock.expectCall("GROUPMSTAT|foo|bar");
        mock.provideReturnValue(new VectorValue(vec));

        afl::data::StringList_t names;
        names.push_back("foo");
        names.push_back("bar");
        afl::container::PtrVector<server::interface::TalkGroup::Description> result;
        testee.getDescriptions(names, result);
    }

    mock.checkFinish();
}

