/**
  *  \file u/t_server_interface_talknntpserver.cpp
  *  \brief Test for server::interface::TalkNNTPServer
  */

#include <memory>
#include <stdexcept>
#include "server/interface/talknntpserver.hpp"

#include "t_server_interface.hpp"
#include "u/helper/callreceiver.hpp"
#include "afl/string/format.hpp"
#include "afl/data/access.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "server/types.hpp"
#include "server/interface/talknntpclient.hpp"

using afl::string::Format;
using afl::data::Segment;
using server::interface::TalkNNTP;
using afl::data::Hash;
using afl::data::HashValue;

namespace {
    class TalkNNTPMock : public TalkNNTP, public CallReceiver {
     public:
        virtual String_t checkUser(String_t loginName, String_t password)
            {
                checkCall(Format("checkUser(%s,%s)", loginName, password));
                return consumeReturnValue<String_t>();
            }
        virtual void listNewsgroups(afl::container::PtrVector<Info>& result)
            {
                checkCall("listNewsgroups()");
                result.pushBackNew(0);            // Not a normal value, but let's check how it passes through the infrastructure
                Info& i = *result.pushBackNew(new Info());
                i.newsgroupName = "ng.name";
                i.description = "Description";
                i.firstSequenceNumber = 77;
                i.lastSequenceNumber = 99;
                i.writeAllowed = true;
                i.forumId = 42;
            }
        virtual Info findNewsgroup(String_t newsgroupName)
            {
                checkCall(Format("findNewsgroup(%s)", newsgroupName));
                return consumeReturnValue<Info>();
            }
        virtual int32_t findMessage(String_t rfcMsgId)
            {
                checkCall(Format("findMessage(%s)", rfcMsgId));
                return consumeReturnValue<int32_t>();
            }
        virtual void listMessages(int32_t forumId, afl::data::IntegerList_t& result)
            {
                checkCall(Format("listMessages(%d)", forumId));
                result.push_back(1);
                result.push_back(10);
                result.push_back(2);
                result.push_back(12);
                result.push_back(4);
                result.push_back(13);
            }
        virtual afl::data::Hash::Ref_t getMessageHeader(int32_t messageId)
            {
                checkCall(Format("getMessageHeader(%d)", messageId));
                return consumeReturnValue<afl::data::Hash::Ref_t>();
            }
        virtual void getMessageHeader(afl::base::Memory<const int32_t> messageIds, afl::data::Segment& results)
            {
                String_t cmd = "getMessageHeader(";
                while (const int32_t* p = messageIds.eat()) {
                    cmd += Format("%d", *p);
                    if (!messageIds.empty()) {
                        cmd += ",";
                    }
                    results.pushBackNew(consumeReturnValue<afl::data::Value*>());
                }
                cmd += ")";
                checkCall(cmd);
            }
        virtual void listNewsgroupsByGroup(String_t groupId, afl::data::StringList_t& result)
            {
                checkCall(Format("listNewsgroupsByGroup(%s)", groupId));
                result.push_back("a");
                result.push_back("b");
                result.push_back("c");
            }
    };
}

/** Test calls. */
void
TestServerInterfaceTalkNNTPServer::testIt()
{
    TalkNNTPMock mock;
    server::interface::TalkNNTPServer testee(mock);

    // checkUser
    mock.expectCall("checkUser(uu,pp)");
    mock.provideReturnValue<String_t>("1045");
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("NNTPUSER").pushBackString("uu").pushBackString("pp")), "1045");

    mock.expectCall("checkUser(u\xc2\x80,pp)");
    mock.provideReturnValue<String_t>("1046");
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("NNTPUSER").pushBackString("u\xc2\x80").pushBackString("pp")), "1046");

    // listNewsgroups
    {
        mock.expectCall("listNewsgroups()");
        std::auto_ptr<afl::data::Value> p(testee.call(Segment().pushBackString("NNTPLIST")));

        afl::data::Access a(p);
        TS_ASSERT_EQUALS(a.getArraySize(), 2U);
        TS_ASSERT(a[0].getValue() == 0);
        TS_ASSERT(a[1].getValue() != 0);
        TS_ASSERT_EQUALS(a[1]("newsgroup").toString(), "ng.name");
        TS_ASSERT_EQUALS(a[1]("description").toString(), "Description");
        TS_ASSERT_EQUALS(a[1]("firstSeq").toInteger(), 77);
        TS_ASSERT_EQUALS(a[1]("lastSeq").toInteger(), 99);
        TS_ASSERT_EQUALS(a[1]("writeAllowed").toInteger(), 1);
        TS_ASSERT_EQUALS(a[1]("id").toInteger(), 42);
    }

    // findNewsgroup
    {
        TalkNNTP::Info in;
        in.newsgroupName = "ng.name2";
        in.description = "Des";
        in.firstSequenceNumber = 1;
        in.lastSequenceNumber = 9;
        in.writeAllowed = false;
        in.forumId = 17;

        mock.expectCall("findNewsgroup(ng.name2)");
        mock.provideReturnValue(in);

        std::auto_ptr<afl::data::Value> p(testee.call(Segment().pushBackString("NNTPFINDNG").pushBackString("ng.name2")));
        afl::data::Access a(p);

        TS_ASSERT_EQUALS(a("newsgroup").toString(), "ng.name2");
        TS_ASSERT_EQUALS(a("description").toString(), "Des");
        TS_ASSERT_EQUALS(a("firstSeq").toInteger(), 1);
        TS_ASSERT_EQUALS(a("lastSeq").toInteger(), 9);
        TS_ASSERT_EQUALS(a("writeAllowed").toInteger(), 0);
        TS_ASSERT_EQUALS(a("id").toInteger(), 17);
    }

    // findMessage
    mock.expectCall("findMessage(a@b)");
    mock.provideReturnValue<int32_t>(76);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("NNTPFINDMID").pushBackString("a@b")), 76);

    // listMessages
    {
        mock.expectCall("listMessages(48)");

        std::auto_ptr<afl::data::Value> p(testee.call(Segment().pushBackString("NNTPFORUMLS").pushBackInteger(48)));
        afl::data::Access a(p);
        TS_ASSERT_EQUALS(a.getArraySize(), 6U);
        TS_ASSERT_EQUALS(a[0].toInteger(), 1);
        TS_ASSERT_EQUALS(a[1].toInteger(), 10);
        TS_ASSERT_EQUALS(a[2].toInteger(), 2);
        TS_ASSERT_EQUALS(a[3].toInteger(), 12);
        TS_ASSERT_EQUALS(a[4].toInteger(), 4);
        TS_ASSERT_EQUALS(a[5].toInteger(), 13);
    }

    // getMessageHeader
    {
        Hash::Ref_t in = Hash::create();
        in->setNew("Message-Id", server::makeStringValue("x.y3@z"));

        mock.expectCall("getMessageHeader(3)");
        mock.provideReturnValue(in);

        std::auto_ptr<afl::data::Value> p(testee.call(Segment().pushBackString("NNTPPOSTHEAD").pushBackInteger(3)));
        afl::data::Access a(p);

        TS_ASSERT_EQUALS(a("Message-Id").toString(), "x.y3@z");
    }

    // getMessageHeaders
    {
        Hash::Ref_t in = Hash::create();
        in->setNew("Message-Id", server::makeStringValue("post9@z"));

        mock.expectCall("getMessageHeader(9,10)");
        mock.provideReturnValue<afl::data::Value*>(new HashValue(in));
        mock.provideReturnValue<afl::data::Value*>(0);

        std::auto_ptr<afl::data::Value> p(testee.call(Segment().pushBackString("NNTPPOSTMHEAD").pushBackInteger(9).pushBackInteger(10)));
        afl::data::Access a(p);

        TS_ASSERT_EQUALS(a.getArraySize(), 2U);
        TS_ASSERT(a[0].getValue() != 0);
        TS_ASSERT(a[1].getValue() == 0);
        TS_ASSERT_EQUALS(a[0]("Message-Id").toString(), "post9@z");
    }

    // listNewsgroupsByGroup
    {
        mock.expectCall("listNewsgroupsByGroup(ngg)");

        std::auto_ptr<afl::data::Value> p(testee.call(Segment().pushBackString("NNTPGROUPLS").pushBackString("ngg")));
        afl::data::Access a(p);

        TS_ASSERT_EQUALS(a.getArraySize(), 3U);
        TS_ASSERT_EQUALS(a[0].toString(), "a");
        TS_ASSERT_EQUALS(a[1].toString(), "b");
        TS_ASSERT_EQUALS(a[2].toString(), "c");
    }

    // Variants
    mock.expectCall("findMessage(a@b)");
    mock.provideReturnValue<int32_t>(67);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("nntpfindmid").pushBackString("a@b")), 67);

    mock.checkFinish();
}

/** Test errors. */
void
TestServerInterfaceTalkNNTPServer::testErrors()
{
    TalkNNTPMock mock;
    server::interface::TalkNNTPServer testee(mock);

    Segment empty;    // g++-3.4 sees an invocation of a copy constructor if I construct this object in-place.
    TS_ASSERT_THROWS(testee.callVoid(empty), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("BAD")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("NNTPGROUPLS")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("NNTPGROUPLS").pushBackString("a").pushBackString("b")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("NNTPFORUMLS").pushBackString("x")), std::exception);

    interpreter::Arguments args(empty, 0, 0);
    std::auto_ptr<afl::data::Value> p;
    TS_ASSERT_EQUALS(testee.handleCommand("huhu", args, p), false);

    mock.checkFinish();
}

/** Test roundtrip behaviour. */
void
TestServerInterfaceTalkNNTPServer::testRoundtrip()
{
    TalkNNTPMock mock;
    server::interface::TalkNNTPServer level1(mock);
    server::interface::TalkNNTPClient level2(level1);
    server::interface::TalkNNTPServer level3(level2);
    server::interface::TalkNNTPClient level4(level3);

    // checkUser
    mock.expectCall("checkUser(uu,pp)");
    mock.provideReturnValue<String_t>("1045");
    TS_ASSERT_EQUALS(level4.checkUser("uu", "pp"), "1045");

    mock.expectCall("checkUser(u\xc2\x80,pp)");
    mock.provideReturnValue<String_t>("1046");
    TS_ASSERT_EQUALS(level4.checkUser("u\xc2\x80", "pp"), "1046");

    // listNewsgroups
    {
        mock.expectCall("listNewsgroups()");

        afl::container::PtrVector<TalkNNTP::Info> result;
        level4.listNewsgroups(result);

        TS_ASSERT_EQUALS(result.size(), 2U);
        // Null is not preserved, TalkNNTPClient replaces it by a default-initialized Info.
        // TS_ASSERT(result[0] == 0);
        TS_ASSERT(result[1] != 0);
        TS_ASSERT_EQUALS(result[1]->newsgroupName, "ng.name");
        TS_ASSERT_EQUALS(result[1]->description, "Description");
        TS_ASSERT_EQUALS(result[1]->firstSequenceNumber, 77);
        TS_ASSERT_EQUALS(result[1]->lastSequenceNumber, 99);
        TS_ASSERT_EQUALS(result[1]->writeAllowed, true);
        TS_ASSERT_EQUALS(result[1]->forumId, 42);
    }

    // findNewsgroup
    {
        TalkNNTP::Info in;
        in.newsgroupName = "ng.name2";
        in.description = "Des";
        in.firstSequenceNumber = 1;
        in.lastSequenceNumber = 9;
        in.writeAllowed = false;
        in.forumId = 17;

        mock.expectCall("findNewsgroup(ng.name2)");
        mock.provideReturnValue(in);

        TalkNNTP::Info out = level4.findNewsgroup("ng.name2");

        TS_ASSERT_EQUALS(out.newsgroupName, "ng.name2");
        TS_ASSERT_EQUALS(out.description, "Des");
        TS_ASSERT_EQUALS(out.firstSequenceNumber, 1);
        TS_ASSERT_EQUALS(out.lastSequenceNumber, 9);
        TS_ASSERT_EQUALS(out.writeAllowed, false);
        TS_ASSERT_EQUALS(out.forumId, 17);
    }

    // findMessage
    mock.expectCall("findMessage(a@b)");
    mock.provideReturnValue<int32_t>(76);
    TS_ASSERT_EQUALS(level4.findMessage("a@b"), 76);

    // listMessages
    {
        mock.expectCall("listMessages(48)");

        afl::data::IntegerList_t result;
        level4.listMessages(48, result);
        TS_ASSERT_EQUALS(result.size(), 6U);
        TS_ASSERT_EQUALS(result[0], 1);
        TS_ASSERT_EQUALS(result[1], 10);
        TS_ASSERT_EQUALS(result[2], 2);
        TS_ASSERT_EQUALS(result[3], 12);
        TS_ASSERT_EQUALS(result[4], 4);
        TS_ASSERT_EQUALS(result[5], 13);
    }

    // getMessageHeader
    {
        Hash::Ref_t in = Hash::create();
        in->setNew("Message-Id", server::makeStringValue("x.y3@z"));

        mock.expectCall("getMessageHeader(3)");
        mock.provideReturnValue(in);

        Hash::Ref_t out = level4.getMessageHeader(3);

        TS_ASSERT_EQUALS(server::toString(out->get("Message-Id")), "x.y3@z");
    }

    // getMessageHeaders
    {
        Hash::Ref_t in = Hash::create();
        in->setNew("Message-Id", server::makeStringValue("post9@z"));

        mock.expectCall("getMessageHeader(9,10)");
        mock.provideReturnValue<afl::data::Value*>(new HashValue(in));
        mock.provideReturnValue<afl::data::Value*>(0);

        afl::data::Segment seg;
        static const int32_t mids[] = { 9, 10 };
        level4.getMessageHeader(mids, seg);

        TS_ASSERT_EQUALS(seg.size(), 2U);
        TS_ASSERT(seg[0] != 0);
        TS_ASSERT(seg[1] == 0);
        TS_ASSERT_EQUALS(afl::data::Access(seg[0])("Message-Id").toString(), "post9@z");
    }

    // listNewsgroupsByGroup
    {
        mock.expectCall("listNewsgroupsByGroup(ngg)");
        afl::data::StringList_t result;
        level4.listNewsgroupsByGroup("ngg", result);

        TS_ASSERT_EQUALS(result.size(), 3U);
        TS_ASSERT_EQUALS(result[0], "a");
        TS_ASSERT_EQUALS(result[1], "b");
        TS_ASSERT_EQUALS(result[2], "c");
    }

    mock.checkFinish();
}

