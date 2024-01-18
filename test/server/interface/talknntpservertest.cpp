/**
  *  \file test/server/interface/talknntpservertest.cpp
  *  \brief Test for server::interface::TalkNNTPServer
  */

#include "server/interface/talknntpserver.hpp"

#include "afl/data/access.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"
#include "server/interface/talknntpclient.hpp"
#include "server/types.hpp"
#include <memory>
#include <stdexcept>

using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Segment;
using afl::string::Format;
using server::interface::TalkNNTP;

namespace {
    class TalkNNTPMock : public TalkNNTP, public afl::test::CallReceiver {
     public:
        TalkNNTPMock(afl::test::Assert a)
            : CallReceiver(a)
            { }
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
AFL_TEST("server.interface.TalkNNTPServer:commands", a)
{
    TalkNNTPMock mock(a);
    server::interface::TalkNNTPServer testee(mock);

    // listNewsgroups
    {
        mock.expectCall("listNewsgroups()");
        std::auto_ptr<afl::data::Value> p(testee.call(Segment().pushBackString("NNTPLIST")));

        afl::data::Access ap(p);
        a.checkEqual("01. getArraySize", ap.getArraySize(), 2U);
        a.checkNull("02. result",        ap[0].getValue());
        a.checkNonNull("03. result",     ap[1].getValue());
        a.checkEqual("04. newsgroup",    ap[1]("newsgroup").toString(), "ng.name");
        a.checkEqual("05. description",  ap[1]("description").toString(), "Description");
        a.checkEqual("06. firstseq",     ap[1]("firstSeq").toInteger(), 77);
        a.checkEqual("07. lastseq",      ap[1]("lastSeq").toInteger(), 99);
        a.checkEqual("08. writeallowed", ap[1]("writeAllowed").toInteger(), 1);
        a.checkEqual("09. id",           ap[1]("id").toInteger(), 42);
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
        afl::data::Access ap(p);

        a.checkEqual("11. newsgroup",    ap("newsgroup").toString(), "ng.name2");
        a.checkEqual("12. description",  ap("description").toString(), "Des");
        a.checkEqual("13. firstseq",     ap("firstSeq").toInteger(), 1);
        a.checkEqual("14. lastseq",      ap("lastSeq").toInteger(), 9);
        a.checkEqual("15. writeallowed", ap("writeAllowed").toInteger(), 0);
        a.checkEqual("16. id",           ap("id").toInteger(), 17);
    }

    // findMessage
    mock.expectCall("findMessage(a@b)");
    mock.provideReturnValue<int32_t>(76);
    a.checkEqual("21. nntpfindmid", testee.callInt(Segment().pushBackString("NNTPFINDMID").pushBackString("a@b")), 76);

    // listMessages
    {
        mock.expectCall("listMessages(48)");

        std::auto_ptr<afl::data::Value> p(testee.call(Segment().pushBackString("NNTPFORUMLS").pushBackInteger(48)));
        afl::data::Access ap(p);
        a.checkEqual("31. getArraySize", ap.getArraySize(), 6U);
        a.checkEqual("32. result", ap[0].toInteger(), 1);
        a.checkEqual("33. result", ap[1].toInteger(), 10);
        a.checkEqual("34. result", ap[2].toInteger(), 2);
        a.checkEqual("35. result", ap[3].toInteger(), 12);
        a.checkEqual("36. result", ap[4].toInteger(), 4);
        a.checkEqual("37. result", ap[5].toInteger(), 13);
    }

    // getMessageHeader
    {
        Hash::Ref_t in = Hash::create();
        in->setNew("Message-Id", server::makeStringValue("x.y3@z"));

        mock.expectCall("getMessageHeader(3)");
        mock.provideReturnValue(in);

        std::auto_ptr<afl::data::Value> p(testee.call(Segment().pushBackString("NNTPPOSTHEAD").pushBackInteger(3)));
        afl::data::Access ap(p);

        a.checkEqual("41. Message-Id", ap("Message-Id").toString(), "x.y3@z");
    }

    // getMessageHeaders
    {
        Hash::Ref_t in = Hash::create();
        in->setNew("Message-Id", server::makeStringValue("post9@z"));

        mock.expectCall("getMessageHeader(9,10)");
        mock.provideReturnValue<afl::data::Value*>(new HashValue(in));
        mock.provideReturnValue<afl::data::Value*>(0);

        std::auto_ptr<afl::data::Value> p(testee.call(Segment().pushBackString("NNTPPOSTMHEAD").pushBackInteger(9).pushBackInteger(10)));
        afl::data::Access ap(p);

        a.checkEqual("51. getArraySize", ap.getArraySize(), 2U);
        a.checkNonNull("52. entry 0", ap[0].getValue());
        a.checkNull("53. entry 1", ap[1].getValue());
        a.checkEqual("54. Message-Id", ap[0]("Message-Id").toString(), "post9@z");
    }

    // listNewsgroupsByGroup
    {
        mock.expectCall("listNewsgroupsByGroup(ngg)");

        std::auto_ptr<afl::data::Value> p(testee.call(Segment().pushBackString("NNTPGROUPLS").pushBackString("ngg")));
        afl::data::Access ap(p);

        a.checkEqual("61. getArraySize", ap.getArraySize(), 3U);
        a.checkEqual("62. result", ap[0].toString(), "a");
        a.checkEqual("63. result", ap[1].toString(), "b");
        a.checkEqual("64. result", ap[2].toString(), "c");
    }

    // Variants
    mock.expectCall("findMessage(a@b)");
    mock.provideReturnValue<int32_t>(67);
    a.checkEqual("71. nntpfindmid", testee.callInt(Segment().pushBackString("nntpfindmid").pushBackString("a@b")), 67);

    mock.checkFinish();
}

/** Test errors. */
AFL_TEST("server.interface.TalkNNTPServer:errors", a)
{
    TalkNNTPMock mock(a);
    server::interface::TalkNNTPServer testee(mock);

    Segment empty;    // g++-3.4 sees an invocation of a copy constructor if I construct this object in-place.
    AFL_CHECK_THROWS(a("01. no verb"),       testee.callVoid(empty), std::exception);
    AFL_CHECK_THROWS(a("02. bad verb"),      testee.callVoid(Segment().pushBackString("BAD")), std::exception);
    AFL_CHECK_THROWS(a("03. missing arg"),   testee.callVoid(Segment().pushBackString("NNTPGROUPLS")), std::exception);
    AFL_CHECK_THROWS(a("04. too many args"), testee.callVoid(Segment().pushBackString("NNTPGROUPLS").pushBackString("a").pushBackString("b")), std::exception);
    AFL_CHECK_THROWS(a("05. bad type"),      testee.callVoid(Segment().pushBackString("NNTPFORUMLS").pushBackString("x")), std::exception);

    interpreter::Arguments args(empty, 0, 0);
    std::auto_ptr<afl::data::Value> p;
    a.checkEqual("11. bad verb", testee.handleCommand("huhu", args, p), false);

    mock.checkFinish();
}

/** Test roundtrip behaviour. */
AFL_TEST("server.interface.TalkNNTPServer:roundtrip", a)
{
    TalkNNTPMock mock(a);
    server::interface::TalkNNTPServer level1(mock);
    server::interface::TalkNNTPClient level2(level1);
    server::interface::TalkNNTPServer level3(level2);
    server::interface::TalkNNTPClient level4(level3);

    // listNewsgroups
    {
        mock.expectCall("listNewsgroups()");

        afl::container::PtrVector<TalkNNTP::Info> result;
        level4.listNewsgroups(result);

        a.checkEqual("01. size", result.size(), 2U);
        // Null is not preserved, TalkNNTPClient replaces it by a default-initialized Info.
        // a.checkNull("02",                    result[0]);
        a.checkNonNull("03. result",            result[1]);
        a.checkEqual("04. newsgroupName",       result[1]->newsgroupName, "ng.name");
        a.checkEqual("05. description",         result[1]->description, "Description");
        a.checkEqual("06. firstSequenceNumber", result[1]->firstSequenceNumber, 77);
        a.checkEqual("07. lastSequenceNumber",  result[1]->lastSequenceNumber, 99);
        a.checkEqual("08. writeAllowed",        result[1]->writeAllowed, true);
        a.checkEqual("09. forumId",             result[1]->forumId, 42);
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

        a.checkEqual("11. newsgroupName",       out.newsgroupName, "ng.name2");
        a.checkEqual("12. description",         out.description, "Des");
        a.checkEqual("13. firstSequenceNumber", out.firstSequenceNumber, 1);
        a.checkEqual("14. lastSequenceNumber",  out.lastSequenceNumber, 9);
        a.checkEqual("15. writeAllowed",        out.writeAllowed, false);
        a.checkEqual("16. forumId",             out.forumId, 17);
    }

    // findMessage
    mock.expectCall("findMessage(a@b)");
    mock.provideReturnValue<int32_t>(76);
    a.checkEqual("21", level4.findMessage("a@b"), 76);

    // listMessages
    {
        mock.expectCall("listMessages(48)");

        afl::data::IntegerList_t result;
        level4.listMessages(48, result);
        a.checkEqual("31. size", result.size(), 6U);
        a.checkEqual("32. result", result[0], 1);
        a.checkEqual("33. result", result[1], 10);
        a.checkEqual("34. result", result[2], 2);
        a.checkEqual("35. result", result[3], 12);
        a.checkEqual("36. result", result[4], 4);
        a.checkEqual("37. result", result[5], 13);
    }

    // getMessageHeader
    {
        Hash::Ref_t in = Hash::create();
        in->setNew("Message-Id", server::makeStringValue("x.y3@z"));

        mock.expectCall("getMessageHeader(3)");
        mock.provideReturnValue(in);

        Hash::Ref_t out = level4.getMessageHeader(3);

        a.checkEqual("41. Message-Id", server::toString(out->get("Message-Id")), "x.y3@z");
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

        a.checkEqual("51. size", seg.size(), 2U);
        a.checkNonNull("52. entry", seg[0]);
        a.checkNull("53. entry", seg[1]);
        a.checkEqual("54. Message-Id", afl::data::Access(seg[0])("Message-Id").toString(), "post9@z");
    }

    // listNewsgroupsByGroup
    {
        mock.expectCall("listNewsgroupsByGroup(ngg)");
        afl::data::StringList_t result;
        level4.listNewsgroupsByGroup("ngg", result);

        a.checkEqual("61. size", result.size(), 3U);
        a.checkEqual("62. result", result[0], "a");
        a.checkEqual("63. result", result[1], "b");
        a.checkEqual("64. result", result[2], "c");
    }

    mock.checkFinish();
}
