/**
  *  \file u/t_server_interface_talkthreadclient.cpp
  *  \brief Test for server::interface::TalkThreadClient
  */

#include "server/interface/talkthreadclient.hpp"

#include "t_server_interface.hpp"
#include "afl/data/access.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/test/commandhandler.hpp"
#include "server/types.hpp"

using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Segment;
using afl::data::Value;
using afl::data::Vector;
using afl::data::VectorValue;

/** Test it. */
void
TestServerInterfaceTalkThreadClient::testIt()
{
    afl::test::CommandHandler mock("testIt");
    server::interface::TalkThreadClient testee(mock);

    // getInfo
    {
        Hash::Ref_t in = Hash::create();
        in->setNew("subject", server::makeStringValue("Subj"));
        in->setNew("forum", server::makeIntegerValue(3));
        mock.expectCall("THREADSTAT, 92");
        mock.provideNewResult(new HashValue(in));

        server::interface::TalkThread::Info out = testee.getInfo(92);
        TS_ASSERT_EQUALS(out.subject, "Subj");
        TS_ASSERT_EQUALS(out.forumId, 3);
        TS_ASSERT_EQUALS(out.firstPostId, 0);
        TS_ASSERT_EQUALS(out.lastPostId, 0);
        TS_ASSERT_EQUALS(out.lastTime, 0);
        TS_ASSERT(!out.isSticky);
    }

    // getInfos
    {
        Hash::Ref_t in = Hash::create();
        in->setNew("subject", server::makeStringValue("Subj"));
        in->setNew("forum", server::makeIntegerValue(3));
        in->setNew("firstpost", server::makeIntegerValue(300));
        in->setNew("lastpost", server::makeIntegerValue(333));
        in->setNew("lasttime", server::makeIntegerValue(22222));
        in->setNew("sticky", server::makeIntegerValue(1));

        Vector::Ref_t vec = Vector::create();
        vec->pushBackNew(new HashValue(in));
        vec->pushBackNew(0);
        
        mock.expectCall("THREADMSTAT, 420, 421");
        mock.provideNewResult(new VectorValue(vec));

        static const int32_t threadIds[] = {420,421};
        afl::container::PtrVector<server::interface::TalkThread::Info> out;
        testee.getInfo(threadIds, out);

        TS_ASSERT_EQUALS(out.size(), 2U);
        TS_ASSERT(out[0] != 0);
        TS_ASSERT(out[1] == 0);

        TS_ASSERT_EQUALS(out[0]->subject, "Subj");
        TS_ASSERT_EQUALS(out[0]->forumId, 3);
        TS_ASSERT_EQUALS(out[0]->firstPostId, 300);
        TS_ASSERT_EQUALS(out[0]->lastPostId, 333);
        TS_ASSERT_EQUALS(out[0]->lastTime, 22222);
        TS_ASSERT(out[0]->isSticky);
    }

    // getPosts
    {
        server::interface::TalkThread::ListParameters params;
        mock.expectCall("THREADLSPOST, 77");
        mock.provideNewResult(server::makeIntegerValue(9));

        std::auto_ptr<afl::data::Value> result(testee.getPosts(77, params));
        TS_ASSERT_EQUALS(server::toInteger(result.get()), 9);
    }
    {
        server::interface::TalkThread::ListParameters params;
        params.sortKey = "LASTTIME";
        params.mode = params.WantRange;
        params.start = 30;
        params.count = 10;
        mock.expectCall("THREADLSPOST, 77, LIMIT, 30, 10, SORT, LASTTIME");
        mock.provideNewResult(server::makeIntegerValue(9));

        std::auto_ptr<afl::data::Value> result(testee.getPosts(77, params));
        TS_ASSERT_EQUALS(server::toInteger(result.get()), 9);
    }

    // setSticky
    mock.expectCall("THREADSTICKY, 78, 1");
    mock.provideNewResult(0);
    testee.setSticky(78, true);
    mock.expectCall("THREADSTICKY, 79, 0");
    mock.provideNewResult(0);
    testee.setSticky(79, false);

    // getPermissions
    mock.expectCall("THREADPERMS, 12");
    mock.provideNewResult(server::makeIntegerValue(0));
    TS_ASSERT_EQUALS(testee.getPermissions(12, afl::base::Nothing), 0);
    {
        const String_t perms[] = {"read","write","delete"};
        mock.expectCall("THREADPERMS, 12, read, write, delete");
        mock.provideNewResult(server::makeIntegerValue(7));
        TS_ASSERT_EQUALS(testee.getPermissions(12, perms), 7);
    }

    // moveToForum
    mock.expectCall("THREADMV, 35, 2");
    mock.provideNewResult(0);
    testee.moveToForum(35, 2);

    // remove
    mock.expectCall("THREADRM, 8");
    mock.provideNewResult(server::makeIntegerValue(1));
    TS_ASSERT(testee.remove(8));
    mock.expectCall("THREADRM, 81");
    mock.provideNewResult(server::makeIntegerValue(0));
    TS_ASSERT(!testee.remove(81));

    mock.checkFinish();
}
