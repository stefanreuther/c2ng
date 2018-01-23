/**
  *  \file u/t_server_interface_talknntpclient.cpp
  *  \brief Test for server::interface::TalkNNTPClient
  */

#include "server/interface/talknntpclient.hpp"

#include "t_server_interface.hpp"
#include "afl/data/access.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/test/commandhandler.hpp"
#include "server/types.hpp"

using server::makeStringValue;
using server::makeIntegerValue;
using server::interface::TalkNNTP;
using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Vector;
using afl::data::VectorValue;

/** Simple tests. */
void
TestServerInterfaceTalkNNTPClient::testIt()
{
    afl::test::CommandHandler mock("testIt");
    server::interface::TalkNNTPClient testee(mock);

    // checkUser
    mock.expectCall("NNTPUSER, theUser, thePassword");
    mock.provideNewResult(makeStringValue("1030"));
    TS_ASSERT_EQUALS(testee.checkUser("theUser", "thePassword"), "1030");

    // listNewsgroups
    {
        mock.expectCall("NNTPLIST");
        mock.provideNewResult(0);
        afl::container::PtrVector<TalkNNTP::Info> result;
        testee.listNewsgroups(result);
        TS_ASSERT_EQUALS(result.size(), 0U);
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
        TS_ASSERT_EQUALS(result.size(), 1U);
        TS_ASSERT(result[0] != 0);
        TS_ASSERT_EQUALS(result[0]->newsgroupName, "pcc.group");
        TS_ASSERT_EQUALS(result[0]->forumId, 3);
        TS_ASSERT_EQUALS(result[0]->description, "This is a newsgroup");
        TS_ASSERT_EQUALS(result[0]->firstSequenceNumber, 103);
        TS_ASSERT_EQUALS(result[0]->lastSequenceNumber, 245);
        TS_ASSERT_EQUALS(result[0]->writeAllowed, true);
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
        TS_ASSERT_EQUALS(out.newsgroupName, "pcc.another.group");
        TS_ASSERT_EQUALS(out.forumId, 5);
        TS_ASSERT_EQUALS(out.description, "Another...");
        TS_ASSERT_EQUALS(out.firstSequenceNumber, 1);
        TS_ASSERT_EQUALS(out.lastSequenceNumber, 27);
        TS_ASSERT_EQUALS(out.writeAllowed, false);
    }

    // findMessage
    mock.expectCall("NNTPFINDMID, a.b.c@d");
    mock.provideNewResult(makeIntegerValue(580));
    TS_ASSERT_EQUALS(testee.findMessage("a.b.c@d"), 580);

    // listMessages
    {
        mock.expectCall("NNTPFORUMLS, 9");
        mock.provideNewResult(new VectorValue(Vector::create(afl::data::Segment().pushBackInteger(1).pushBackInteger(37).pushBackInteger(2).pushBackInteger(45))));

        afl::data::IntegerList_t result;
        testee.listMessages(9, result);

        TS_ASSERT_EQUALS(result.size(), 4U);
        TS_ASSERT_EQUALS(result[0], 1);
        TS_ASSERT_EQUALS(result[1], 37);
        TS_ASSERT_EQUALS(result[2], 2);
        TS_ASSERT_EQUALS(result[3], 45);
    }

    // getMessageHeader
    {
        Hash::Ref_t in = Hash::create();
        in->setNew("Content-Type", makeStringValue("text/plain"));
        in->setNew("Message-Id",   makeStringValue("<foo@bar>"));

        mock.expectCall("NNTPPOSTHEAD, 45");
        mock.provideNewResult(new HashValue(in));

        Hash::Ref_t out = testee.getMessageHeader(45);
        TS_ASSERT_EQUALS(server::toString(out->get("Content-Type")), "text/plain");
        TS_ASSERT_EQUALS(server::toString(out->get("Message-Id")), "<foo@bar>");
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

        TS_ASSERT_EQUALS(result.size(), 2U);
        TS_ASSERT(result[0] == 0);
        TS_ASSERT(result[1] != 0);
        TS_ASSERT_EQUALS(afl::data::Access(result[1])("Content-Type").toString(), "text/plain");
    }

    // listNewsgroupsByGroup
    {
        mock.expectCall("NNTPGROUPLS, root");
        mock.provideNewResult(new VectorValue(Vector::create(afl::data::Segment().pushBackString("pcc.news").pushBackString("pcc.info").pushBackString("pcc.talk"))));

        afl::data::StringList_t result;
        testee.listNewsgroupsByGroup("root", result);

        TS_ASSERT_EQUALS(result.size(), 3U);
        TS_ASSERT_EQUALS(result[0], "pcc.news");
        TS_ASSERT_EQUALS(result[1], "pcc.info");
        TS_ASSERT_EQUALS(result[2], "pcc.talk");
    }

    mock.checkFinish();
}
