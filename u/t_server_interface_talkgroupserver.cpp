/**
  *  \file u/t_server_interface_talkgroupserver.cpp
  *  \brief Test for server::interface::TalkGroupServer
  */

#include "server/interface/talkgroupserver.hpp"

#include <memory>
#include <stdexcept>
#include "t_server_interface.hpp"
#include "afl/data/access.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "server/interface/talkgroupclient.hpp"

using afl::string::Format;
using afl::data::Segment;
using server::interface::TalkGroup;

namespace {
    class TalkGroupMock : public TalkGroup, public afl::test::CallReceiver {
     public:
        TalkGroupMock(afl::test::Assert a)
            : CallReceiver(a)
            { }
        virtual void add(String_t groupId, const Description& info)
            { checkCall(Format("add(%s,%s,%s,%s,%s,%d/%d)") << groupId << info.name.orElse("no-name") << info.description.orElse("no-description") << info.parentGroup.orElse("no-parent") << info.key.orElse("no-key") << int(info.unlisted.isValid()) << int(info.unlisted.orElse(false))); }
        virtual void set(String_t groupId, const Description& info)
            { checkCall(Format("set(%s,%s,%s,%s,%s,%d/%d)") << groupId << info.name.orElse("no-name") << info.description.orElse("no-description") << info.parentGroup.orElse("no-parent") << info.key.orElse("no-key") << int(info.unlisted.isValid()) << int(info.unlisted.orElse(false))); }
        virtual String_t getField(String_t groupId, String_t fieldName)
            {
                checkCall(Format("getField(%s,%s)", groupId, fieldName));
                return consumeReturnValue<String_t>();
            }
        virtual void list(String_t groupId, afl::data::StringList_t& groups, afl::data::IntegerList_t& forums)
            {
                checkCall(Format("list(%s)", groupId));
                groups.push_back("sub");
                forums.push_back(12);
                forums.push_back(13);
            }
        virtual Description getDescription(String_t groupId)
            {
                checkCall(Format("getDescription(%s)", groupId));
                return consumeReturnValue<Description>();
            }
        virtual void getDescriptions(const afl::data::StringList_t& groups, afl::container::PtrVector<Description>& results)
            {
                String_t cmd = "getDescriptions(";
                for (size_t i = 0; i < groups.size(); ++i) {
                    if (i != 0) {
                        cmd += ",";
                    }
                    cmd += groups[i];
                    results.pushBackNew(consumeReturnValue<Description*>());
                }
                cmd += ")";
                checkCall(cmd);
            }
    };
}

/** Test the server. */
void
TestServerInterfaceTalkGroupServer::testIt()
{
    TalkGroupMock mock("testIt");
    server::interface::TalkGroupServer testee(mock);

    // add
    mock.expectCall("add(g,no-name,no-description,no-parent,no-key,0/0)");
    testee.callVoid(Segment().pushBackString("GROUPADD").pushBackString("g"));

    mock.expectCall("add(g,Name,no-description,Parent,no-key,1/0)");
    testee.callVoid(Segment().pushBackString("GROUPADD").pushBackString("g")
                    .pushBackString("parent").pushBackString("Parent")
                    .pushBackString("name").pushBackString("Name")
                    .pushBackString("unlisted").pushBackInteger(0));

    mock.expectCall("add(g,no-name,no-description,no-parent,Key,0/0)");
    testee.callVoid(Segment().pushBackString("GROUPADD").pushBackString("g")
                    .pushBackString("key").pushBackString("Key"));

    // set
    mock.expectCall("set(g,no-name,no-description,no-parent,no-key,0/0)");
    testee.callVoid(Segment().pushBackString("GROUPSET").pushBackString("g"));

    mock.expectCall("set(g,Name,no-description,Parent,no-key,1/0)");
    testee.callVoid(Segment().pushBackString("GROUPSET").pushBackString("g")
                    .pushBackString("parent").pushBackString("Parent")
                    .pushBackString("name").pushBackString("Name")
                    .pushBackString("unlisted").pushBackInteger(0));

    mock.expectCall("set(g,no-name,no-description,no-parent,Key,0/0)");
    testee.callVoid(Segment().pushBackString("GROUPSET").pushBackString("g")
                    .pushBackString("key").pushBackString("Key"));

    // getField
    mock.expectCall("getField(gg,ff)");
    mock.provideReturnValue<String_t>("rr");
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("GROUPGET").pushBackString("gg").pushBackString("ff")), "rr");

    // list
    {
        mock.expectCall("list(gg)");

        std::auto_ptr<afl::data::Value> p(testee.call(Segment().pushBackString("GROUPLS").pushBackString("gg")));
        TS_ASSERT(p.get() != 0);

        afl::data::Access a(p);
        TS_ASSERT_EQUALS(a("groups").getArraySize(), 1U);
        TS_ASSERT_EQUALS(a("groups")[0].toString(), "sub");
        TS_ASSERT_EQUALS(a("forums").getArraySize(), 2U);
        TS_ASSERT_EQUALS(a("forums")[0].toInteger(), 12);
        TS_ASSERT_EQUALS(a("forums")[1].toInteger(), 13);
    }

    // getDescription
    {
        TalkGroup::Description in;
        in.name = "The Name";
        in.description = "The Description";
        mock.expectCall("getDescription(zz)");
        mock.provideReturnValue(in);

        std::auto_ptr<afl::data::Value> p(testee.call(Segment().pushBackString("GROUPSTAT").pushBackString("zz")));
        TS_ASSERT(p.get() != 0);

        afl::data::Access a(p);
        TS_ASSERT_EQUALS(a("name").toString(), "The Name");
        TS_ASSERT_EQUALS(a("description").toString(), "The Description");
        TS_ASSERT(a("unlisted").getValue() == 0);
        TS_ASSERT(a("parent").getValue() == 0);
    }

    // getDescriptions
    {
        TalkGroup::Description in;
        in.name = "The Name";
        in.description = "The Description";
        mock.expectCall("getDescriptions(q1,q2,q3)");
        mock.provideReturnValue<TalkGroup::Description*>(new TalkGroup::Description(in));
        mock.provideReturnValue<TalkGroup::Description*>(0);
        in.name = "Other Name";
        in.parentGroup = "pp";
        mock.provideReturnValue<TalkGroup::Description*>(new TalkGroup::Description(in));

        std::auto_ptr<afl::data::Value> p(testee.call(Segment().pushBackString("GROUPMSTAT").pushBackString("q1").pushBackString("q2").pushBackString("q3")));
        TS_ASSERT(p.get() != 0);

        afl::data::Access a(p);
        TS_ASSERT_EQUALS(a.getArraySize(), 3U);
        TS_ASSERT(a[0].getValue() != 0);
        TS_ASSERT(a[1].getValue() == 0);
        TS_ASSERT(a[2].getValue() != 0);
        TS_ASSERT_EQUALS(a[0]("name").toString(), "The Name");
        TS_ASSERT_EQUALS(a[0]("description").toString(), "The Description");
        TS_ASSERT_EQUALS(a[2]("name").toString(), "Other Name");
        TS_ASSERT_EQUALS(a[2]("description").toString(), "The Description");
        TS_ASSERT_EQUALS(a[2]("parent").toString(), "pp");
    }

    // Variations
    mock.expectCall("getField(Gg,Ff)");
    mock.provideReturnValue<String_t>("rr");
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("groupget").pushBackString("Gg").pushBackString("Ff")), "rr");

    mock.checkFinish();
}

/** Test errors. */
void
TestServerInterfaceTalkGroupServer::testErrors()
{
    TalkGroupMock mock("testErrors");
    server::interface::TalkGroupServer testee(mock);

    // bad arg count
    Segment empty;    // g++-3.4 sees an invocation of a copy constructor if I construct this object in-place.
    TS_ASSERT_THROWS(testee.callVoid(empty), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("GROUPADD")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("GROUPADD").pushBackString("x").pushBackString("name")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("GROUPGET").pushBackString("x")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("GROUPGET").pushBackString("x").pushBackString("x").pushBackString("x")), std::exception);

    // bad option
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("GROUPADD").pushBackString("g")
                                     .pushBackString("PARENT").pushBackString("Parent")
                                     .pushBackString("name").pushBackString("Name")
                                     .pushBackString("Unlisted").pushBackInteger(0)),
                     std::exception);

    // bad command
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("hi")), std::exception);

    // ComposableCommandHandler personality
    interpreter::Arguments args(empty, 0, 0);
    std::auto_ptr<afl::data::Value> p;
    TS_ASSERT_EQUALS(testee.handleCommand("huhu", args, p), false);
}

/** Test roundtrip invocation. */
void
TestServerInterfaceTalkGroupServer::testRoundtrip()
{
    TalkGroupMock mock("testRoundtrip");
    server::interface::TalkGroupServer level1(mock);
    server::interface::TalkGroupClient level2(level1);
    server::interface::TalkGroupServer level3(level2);
    server::interface::TalkGroupClient level4(level3);

    // add
    mock.expectCall("add(g,no-name,no-description,no-parent,no-key,0/0)");
    level4.add("g", TalkGroup::Description());

    {
        TalkGroup::Description d;
        d.parentGroup = "Parent";
        d.name = "Name";
        d.unlisted = false;
        d.key = "KK";

        mock.expectCall("add(g,Name,no-description,Parent,KK,1/0)");
        level4.add("g", d);
    }

    // set
    mock.expectCall("set(g,no-name,no-description,no-parent,no-key,0/0)");
    level4.set("g", TalkGroup::Description());

    {
        TalkGroup::Description d;
        d.parentGroup = "Parent";
        d.name = "Name";
        d.unlisted = false;

        mock.expectCall("set(g,Name,no-description,Parent,no-key,1/0)");
        level4.set("g", d);
    }

    // getField
    mock.expectCall("getField(gg,ff)");
    mock.provideReturnValue<String_t>("rr");
    TS_ASSERT_EQUALS(level4.getField("gg", "ff"), "rr");

    // list
    {
        mock.expectCall("list(gg)");

        afl::data::StringList_t groups;
        afl::data::IntegerList_t forums;
        level4.list("gg", groups, forums);

        TS_ASSERT_EQUALS(groups.size(), 1U);
        TS_ASSERT_EQUALS(groups[0], "sub");
        TS_ASSERT_EQUALS(forums.size(), 2U);
        TS_ASSERT_EQUALS(forums[0], 12);
        TS_ASSERT_EQUALS(forums[1], 13);
    }

    // getDescription
    {
        TalkGroup::Description in;
        in.name = "The Name";
        in.description = "The Description";
        mock.expectCall("getDescription(zz)");
        mock.provideReturnValue(in);

        TalkGroup::Description out = level4.getDescription("zz");
        TS_ASSERT(out.name.get() != 0);
        TS_ASSERT_EQUALS(*out.name.get(), "The Name");
        TS_ASSERT(out.description.get() != 0);
        TS_ASSERT_EQUALS(*out.description.get(), "The Description");
        TS_ASSERT(out.parentGroup.get() == 0);
        TS_ASSERT(out.unlisted.get() == 0);
    }

    // getDescriptions
    {
        TalkGroup::Description in;
        in.name = "The Name";
        in.description = "The Description";
        mock.expectCall("getDescriptions(q1,q2,q3)");
        mock.provideReturnValue<TalkGroup::Description*>(new TalkGroup::Description(in));
        mock.provideReturnValue<TalkGroup::Description*>(0);
        in.name = "Other Name";
        in.parentGroup = "pp";
        mock.provideReturnValue<TalkGroup::Description*>(new TalkGroup::Description(in));

        afl::data::StringList_t groups;
        groups.push_back("q1");
        groups.push_back("q2");
        groups.push_back("q3");
        afl::container::PtrVector<TalkGroup::Description> out;
        level4.getDescriptions(groups, out);

        TS_ASSERT_EQUALS(out.size(), 3U);
        TS_ASSERT(out[0] != 0);
        TS_ASSERT(out[1] == 0);
        TS_ASSERT(out[2] != 0);
        TS_ASSERT(out[0]->name.get() != 0);
        TS_ASSERT_EQUALS(*out[0]->name.get(), "The Name");
        TS_ASSERT(out[2]->name.get() != 0);
        TS_ASSERT_EQUALS(*out[2]->name.get(), "Other Name");
    }

    mock.checkFinish();
}

