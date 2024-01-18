/**
  *  \file test/server/interface/talkgroupservertest.cpp
  *  \brief Test for server::interface::TalkGroupServer
  */

#include "server/interface/talkgroupserver.hpp"

#include "afl/data/access.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"
#include "server/interface/talkgroupclient.hpp"
#include <memory>
#include <stdexcept>

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
AFL_TEST("server.interface.TalkGroupServer:commands", a)
{
    TalkGroupMock mock(a);
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
    a.checkEqual("01. groupget", testee.callString(Segment().pushBackString("GROUPGET").pushBackString("gg").pushBackString("ff")), "rr");

    // list
    {
        mock.expectCall("list(gg)");

        std::auto_ptr<afl::data::Value> p(testee.call(Segment().pushBackString("GROUPLS").pushBackString("gg")));
        a.checkNonNull("11. groupls", p.get());

        afl::data::Access ap(p);
        a.checkEqual("21. groups", ap("groups").getArraySize(), 1U);
        a.checkEqual("22. groups", ap("groups")[0].toString(), "sub");
        a.checkEqual("23. forums", ap("forums").getArraySize(), 2U);
        a.checkEqual("24. forums", ap("forums")[0].toInteger(), 12);
        a.checkEqual("25. forums", ap("forums")[1].toInteger(), 13);
    }

    // getDescription
    {
        TalkGroup::Description in;
        in.name = "The Name";
        in.description = "The Description";
        mock.expectCall("getDescription(zz)");
        mock.provideReturnValue(in);

        std::auto_ptr<afl::data::Value> p(testee.call(Segment().pushBackString("GROUPSTAT").pushBackString("zz")));
        a.checkNonNull("31", p.get());

        afl::data::Access ap(p);
        a.checkEqual("41", ap("name").toString(), "The Name");
        a.checkEqual("42", ap("description").toString(), "The Description");
        a.checkNull("43", ap("unlisted").getValue());
        a.checkNull("44", ap("parent").getValue());
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
        a.checkNonNull("51. groupmstat", p.get());

        afl::data::Access ap(p);
        a.checkEqual  ("61. getArraySize", ap.getArraySize(), 3U);
        a.checkNonNull("62. entry 0",      ap[0].getValue());
        a.checkNull   ("63. entry 1",      ap[1].getValue());
        a.checkNonNull("64. entry 2",      ap[2].getValue());
        a.checkEqual  ("65. name",         ap[0]("name").toString(), "The Name");
        a.checkEqual  ("66. description",  ap[0]("description").toString(), "The Description");
        a.checkEqual  ("67. name",         ap[2]("name").toString(), "Other Name");
        a.checkEqual  ("68. description",  ap[2]("description").toString(), "The Description");
        a.checkEqual  ("69. parent",       ap[2]("parent").toString(), "pp");
    }

    // Variations
    mock.expectCall("getField(Gg,Ff)");
    mock.provideReturnValue<String_t>("rr");
    a.checkEqual("71. groupget", testee.callString(Segment().pushBackString("groupget").pushBackString("Gg").pushBackString("Ff")), "rr");

    mock.checkFinish();
}

/** Test errors. */
AFL_TEST("server.interface.TalkGroupServer:errors", a)
{
    TalkGroupMock mock(a);
    server::interface::TalkGroupServer testee(mock);

    // bad arg count
    Segment empty;    // g++-3.4 sees an invocation of a copy constructor if I construct this object in-place.
    AFL_CHECK_THROWS(a("01. empty"),          testee.callVoid(empty), std::exception);
    AFL_CHECK_THROWS(a("02. missing arg"),    testee.callVoid(Segment().pushBackString("GROUPADD")), std::exception);
    AFL_CHECK_THROWS(a("03. missing option"), testee.callVoid(Segment().pushBackString("GROUPADD").pushBackString("x").pushBackString("name")), std::exception);
    AFL_CHECK_THROWS(a("04. missing arg"),    testee.callVoid(Segment().pushBackString("GROUPGET").pushBackString("x")), std::exception);
    AFL_CHECK_THROWS(a("05. too many args"),  testee.callVoid(Segment().pushBackString("GROUPGET").pushBackString("x").pushBackString("x").pushBackString("x")), std::exception);

    // bad option
    AFL_CHECK_THROWS(a("11. bad option"), testee.callVoid(Segment().pushBackString("GROUPADD").pushBackString("g")
                                                          .pushBackString("PARENT").pushBackString("Parent")
                                                          .pushBackString("name").pushBackString("Name")
                                                          .pushBackString("Unlisted").pushBackInteger(0)),
                     std::exception);

    // bad command
    AFL_CHECK_THROWS(a("21. bad verb"), testee.callVoid(Segment().pushBackString("hi")), std::exception);

    // ComposableCommandHandler personality
    interpreter::Arguments args(empty, 0, 0);
    std::auto_ptr<afl::data::Value> p;
    a.checkEqual("31. bad verb", testee.handleCommand("huhu", args, p), false);
}

/** Test roundtrip invocation. */
AFL_TEST("server.interface.TalkGroupServer:roundtrip", a)
{
    TalkGroupMock mock(a);
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
    a.checkEqual("01. getField", level4.getField("gg", "ff"), "rr");

    // list
    {
        mock.expectCall("list(gg)");

        afl::data::StringList_t groups;
        afl::data::IntegerList_t forums;
        level4.list("gg", groups, forums);

        a.checkEqual("11. groups", groups.size(), 1U);
        a.checkEqual("12. group",  groups[0], "sub");
        a.checkEqual("13. forums", forums.size(), 2U);
        a.checkEqual("14. forum",  forums[0], 12);
        a.checkEqual("15. forum",  forums[1], 13);
    }

    // getDescription
    {
        TalkGroup::Description in;
        in.name = "The Name";
        in.description = "The Description";
        mock.expectCall("getDescription(zz)");
        mock.provideReturnValue(in);

        TalkGroup::Description out = level4.getDescription("zz");
        a.checkNonNull("21. name",         out.name.get());
        a.checkEqual  ("22. name",        *out.name.get(), "The Name");
        a.checkNonNull("23. description",  out.description.get());
        a.checkEqual  ("24. description", *out.description.get(), "The Description");
        a.checkNull   ("25. parentGroup",  out.parentGroup.get());
        a.checkNull   ("26. unlisted",     out.unlisted.get());
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

        a.checkEqual  ("31. size", out.size(), 3U);
        a.checkNonNull("32. entry 0", out[0]);
        a.checkNull   ("33. entry 1", out[1]);
        a.checkNonNull("34. entry 2", out[2]);
        a.checkNonNull("35. name",    out[0]->name.get());
        a.checkEqual  ("36. name",   *out[0]->name.get(), "The Name");
        a.checkNonNull("37. name",    out[2]->name.get());
        a.checkEqual  ("38. name",   *out[2]->name.get(), "Other Name");
    }

    mock.checkFinish();
}
