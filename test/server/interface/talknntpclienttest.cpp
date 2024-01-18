/**
  *  \file test/server/interface/talknntpclienttest.cpp
  *  \brief Test for server::interface::TalkNNTPClient
  */

#include "server/interface/talknntpclient.hpp"

#include "afl/data/access.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/test/commandhandler.hpp"
#include "afl/test/testrunner.hpp"
#include "server/types.hpp"

using server::makeStringValue;
using server::makeIntegerValue;
using server::interface::TalkNNTP;
using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Vector;
using afl::data::VectorValue;

/** Simple tests. */
AFL_TEST("server.interface.TalkNNTPClient", a)
{
    afl::test::CommandHandler mock(a);
    server::interface::TalkNNTPClient testee(mock);

    // listNewsgroups
    {
        mock.expectCall("NNTPLIST");
        mock.provideNewResult(0);
        afl::container::PtrVector<TalkNNTP::Info> result;
        testee.listNewsgroups(result);
        a.checkEqual("01. size", result.size(), 0U);
    }
    {
        Hash::Ref_t in = Hash::create();
        in->setNew("newsgroup",    makeStringValue("pcc.group"));
        in->setNew("id",           makeIntegerValue(3));
        in->setNew("description",  makeStringValue("This is a newsgroup"));
        in->setNew("firstSeq",     makeIntegerValue(103));
        in->setNew("lastSeq",      makeIntegerValue(245));
        in->setNew("writeAllowed", makeIntegerValue(1));

        Vector::Ref_t vec = Vector::create();
        vec->pushBackNew(new HashValue(in));

        mock.expectCall("NNTPLIST");
        mock.provideNewResult(new VectorValue(vec));

        afl::container::PtrVector<TalkNNTP::Info> result;
        testee.listNewsgroups(result);
        a.checkEqual("11. size", result.size(), 1U);
        a.checkNonNull("12. result",            result[0]);
        a.checkEqual("13. newsgroupName",       result[0]->newsgroupName, "pcc.group");
        a.checkEqual("14. forumId",             result[0]->forumId, 3);
        a.checkEqual("15. description",         result[0]->description, "This is a newsgroup");
        a.checkEqual("16. firstSequenceNumber", result[0]->firstSequenceNumber, 103);
        a.checkEqual("17. lastSequenceNumber",  result[0]->lastSequenceNumber, 245);
        a.checkEqual("18. writeAllowed",        result[0]->writeAllowed, true);
    }

    // findNewsgroup
    {
        Hash::Ref_t in = Hash::create();
        in->setNew("newsgroup",    makeStringValue("pcc.another.group"));
        in->setNew("id",           makeIntegerValue(5));
        in->setNew("description",  makeStringValue("Another..."));
        in->setNew("firstSeq",     makeIntegerValue(1));
        in->setNew("lastSeq",      makeIntegerValue(27));
        in->setNew("writeAllowed", makeIntegerValue(0));

        mock.expectCall("NNTPFINDNG, pcc.another.group");
        mock.provideNewResult(new HashValue(in));

        TalkNNTP::Info out = testee.findNewsgroup("pcc.another.group");
        a.checkEqual("21. newsgroupName",       out.newsgroupName, "pcc.another.group");
        a.checkEqual("22. forumId",             out.forumId, 5);
        a.checkEqual("23. description",         out.description, "Another...");
        a.checkEqual("24. firstSequenceNumber", out.firstSequenceNumber, 1);
        a.checkEqual("25. lastSequenceNumber",  out.lastSequenceNumber, 27);
        a.checkEqual("26. writeAllowed",        out.writeAllowed, false);
    }

    // findMessage
    mock.expectCall("NNTPFINDMID, a.b.c@d");
    mock.provideNewResult(makeIntegerValue(580));
    a.checkEqual("31", testee.findMessage("a.b.c@d"), 580);

    // listMessages
    {
        mock.expectCall("NNTPFORUMLS, 9");
        mock.provideNewResult(new VectorValue(Vector::create(afl::data::Segment().pushBackInteger(1).pushBackInteger(37).pushBackInteger(2).pushBackInteger(45))));

        afl::data::IntegerList_t result;
        testee.listMessages(9, result);

        a.checkEqual("41. size", result.size(), 4U);
        a.checkEqual("42. result", result[0], 1);
        a.checkEqual("43. result", result[1], 37);
        a.checkEqual("44. result", result[2], 2);
        a.checkEqual("45. result", result[3], 45);
    }

    // getMessageHeader
    {
        Hash::Ref_t in = Hash::create();
        in->setNew("Content-Type", makeStringValue("text/plain"));
        in->setNew("Message-Id",   makeStringValue("<foo@bar>"));

        mock.expectCall("NNTPPOSTHEAD, 45");
        mock.provideNewResult(new HashValue(in));

        Hash::Ref_t out = testee.getMessageHeader(45);
        a.checkEqual("51. Content-Type", server::toString(out->get("Content-Type")), "text/plain");
        a.checkEqual("52. Message-Id",   server::toString(out->get("Message-Id")), "<foo@bar>");
    }

    // getMessageHeaders
    {
        Hash::Ref_t in = Hash::create();
        in->setNew("Content-Type", makeStringValue("text/plain"));
        in->setNew("Message-Id",   makeStringValue("<foo@bar>"));

        Vector::Ref_t vec = Vector::create();
        vec->pushBackNew(0);
        vec->pushBackNew(new HashValue(in));

        mock.expectCall("NNTPPOSTMHEAD, 42, 45");
        mock.provideNewResult(new VectorValue(vec));

        afl::data::Segment result;
        static const int32_t msgids[] = { 42, 45 };
        testee.getMessageHeader(msgids, result);

        a.checkEqual("61. size", result.size(), 2U);
        a.checkNull("62. result", result[0]);
        a.checkNonNull("63. result", result[1]);
        a.checkEqual("64. Content-Type", afl::data::Access(result[1])("Content-Type").toString(), "text/plain");
    }

    // listNewsgroupsByGroup
    {
        mock.expectCall("NNTPGROUPLS, root");
        mock.provideNewResult(new VectorValue(Vector::create(afl::data::Segment().pushBackString("pcc.news").pushBackString("pcc.info").pushBackString("pcc.talk"))));

        afl::data::StringList_t result;
        testee.listNewsgroupsByGroup("root", result);

        a.checkEqual("71. size", result.size(), 3U);
        a.checkEqual("72. result", result[0], "pcc.news");
        a.checkEqual("73. result", result[1], "pcc.info");
        a.checkEqual("74. result", result[2], "pcc.talk");
    }

    mock.checkFinish();
}
