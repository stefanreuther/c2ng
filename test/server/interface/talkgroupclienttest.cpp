/**
  *  \file test/server/interface/talkgroupclienttest.cpp
  *  \brief Test for server::interface::TalkGroupClient
  */

#include "server/interface/talkgroupclient.hpp"

#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/segment.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/test/commandhandler.hpp"
#include "afl/test/testrunner.hpp"
#include "server/types.hpp"

using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Vector;
using afl::data::VectorValue;
using afl::data::Segment;

AFL_TEST("server.interface.TalkGroupClient", a)
{
    afl::test::CommandHandler mock(a);
    server::interface::TalkGroupClient testee(mock);

    // add
    {
        mock.expectCall("GROUPADD, g");
        mock.provideNewResult(0);
        testee.add("g", server::interface::TalkGroupClient::Description());
    }
    {
        mock.expectCall("GROUPADD, g2, name, theName, description, theDescription, parent, theParent, unlisted, 0");
        mock.provideNewResult(0);

        server::interface::TalkGroupClient::Description desc;
        desc.name = "theName";
        desc.description = "theDescription";
        desc.parentGroup = "theParent";
        desc.unlisted = false;
        testee.add("g2", desc);
    }

    // set
    {
        mock.expectCall("GROUPSET, g");
        mock.provideNewResult(0);
        testee.set("g", server::interface::TalkGroupClient::Description());
    }
    {
        mock.expectCall("GROUPSET, g2, name, theName, description, theDescription, parent, theParent, unlisted, 0");
        mock.provideNewResult(0);

        server::interface::TalkGroupClient::Description desc;
        desc.name = "theName";
        desc.description = "theDescription";
        desc.parentGroup = "theParent";
        desc.unlisted = false;
        testee.set("g2", desc);
    }

    // getField
    mock.expectCall("GROUPGET, g, name");
    mock.provideNewResult(server::makeStringValue("theName"));
    a.checkEqual("01. getField", testee.getField("g", "name"), "theName");

    // list
    {
        // Return null - should produce no groups/forums
        mock.expectCall("GROUPLS, g");
        mock.provideNewResult(0);

        afl::data::StringList_t groups;
        afl::data::IntegerList_t forums;
        testee.list("g", groups, forums);

        a.checkEqual("11. groups", groups.size(), 0U);
        a.checkEqual("12. forums", forums.size(), 0U);
    }
    {
        // Return proper values
        Hash::Ref_t h = Hash::create();
        h->setNew("groups", new VectorValue(Vector::create(Segment().pushBackString("sub1").pushBackString("sub2"))));
        h->setNew("forums", new VectorValue(Vector::create(Segment().pushBackInteger(32).pushBackInteger(16).pushBackInteger(8))));
        mock.expectCall("GROUPLS, top");
        mock.provideNewResult(new HashValue(h));

        afl::data::StringList_t groups;
        afl::data::IntegerList_t forums;
        testee.list("top", groups, forums);

        a.checkEqual("21. groups", groups.size(), 2U);
        a.checkEqual("22. group",  groups[0], "sub1");
        a.checkEqual("23. group",  groups[1], "sub2");
        a.checkEqual("24. forums", forums.size(), 3U);
        a.checkEqual("25. forum",  forums[0], 32);
        a.checkEqual("26. forum",  forums[1], 16);
        a.checkEqual("27. forum",  forums[2], 8);
    }

    // getDescription
    {
        // Return null
        mock.expectCall("GROUPSTAT, gg");
        mock.provideNewResult(0);

        server::interface::TalkGroup::Description desc = testee.getDescription("gg");

        a.check("31. name",        !desc.name.isValid());
        a.check("32. description", !desc.description.isValid());
        a.check("33. parentGroup", !desc.parentGroup.isValid());
        a.check("34. unlisted",    !desc.unlisted.isValid());
    }
    {
        // Return non-null
        Hash::Ref_t h = Hash::create();
        h->setNew("name", server::makeStringValue("The Name"));
        h->setNew("description", server::makeStringValue("This is the description"));
        h->setNew("parent", server::makeStringValue("parent"));
        h->setNew("unlisted", server::makeIntegerValue(1));
        mock.expectCall("GROUPSTAT, gg2");
        mock.provideNewResult(new HashValue(h));

        server::interface::TalkGroup::Description desc = testee.getDescription("gg2");

        a.checkNonNull("41. name",         desc.name.get());
        a.checkEqual  ("42. name",        *desc.name.get(), "The Name");
        a.checkNonNull("43. description",  desc.description.get());
        a.checkEqual  ("44. description", *desc.description.get(), "This is the description");
        a.checkNonNull("45. parentGroup",  desc.parentGroup.get());
        a.checkEqual  ("46. parentGroup", *desc.parentGroup.get(), "parent");
        a.checkNonNull("47. unlisted",     desc.unlisted.get());
        a.checkEqual  ("48. unlisted",    *desc.unlisted.get(), true);
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
        mock.expectCall("GROUPMSTAT, foo, bar");
        mock.provideNewResult(new VectorValue(vec));

        afl::data::StringList_t names;
        names.push_back("foo");
        names.push_back("bar");
        afl::container::PtrVector<server::interface::TalkGroup::Description> result;
        testee.getDescriptions(names, result);
    }

    mock.checkFinish();
}
