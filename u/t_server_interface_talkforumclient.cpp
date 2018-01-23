/**
  *  \file u/t_server_interface_talkforumclient.cpp
  *  \brief Test for server::interface::TalkForumClient
  */

#include "server/interface/talkforumclient.hpp"

#include <memory>
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

void
TestServerInterfaceTalkForumClient::testIt()
{
    afl::test::CommandHandler mock("testIt");
    server::interface::TalkForumClient testee(mock);

    // add
    mock.expectCall("FORUMADD");
    mock.provideNewResult(server::makeIntegerValue(32));
    TS_ASSERT_EQUALS(testee.add(afl::base::Nothing), 32);

    {
        const String_t ps[] = { "a","b","c","d" };
        mock.expectCall("FORUMADD, a, b, c, d");
        mock.provideNewResult(server::makeIntegerValue(33));
        TS_ASSERT_EQUALS(testee.add(ps), 33);
    }

    // configure
    mock.expectCall("FORUMSET, 12");
    mock.provideNewResult(0);
    testee.configure(12, afl::base::Nothing);

    {
        const String_t ps[] = { "p","q","r","s","t","u" };
        mock.expectCall("FORUMSET, 13, p, q, r, s, t, u");
        mock.provideNewResult(0);
        testee.configure(13, ps);
    }

    // getValue
    {
        mock.expectCall("FORUMGET, 89, fn");
        mock.provideNewResult(0);
        std::auto_ptr<Value> p(testee.getValue(89, "fn"));
        TS_ASSERT(p.get() == 0);

        mock.expectCall("FORUMGET, 89, ifn");
        mock.provideNewResult(server::makeIntegerValue(424242));
        p.reset(testee.getValue(89, "ifn"));
        TS_ASSERT_EQUALS(server::toInteger(p.get()), 424242);

        mock.expectCall("FORUMGET, 98, sfn");
        mock.provideNewResult(server::makeStringValue("hu"));
        p.reset(testee.getValue(98, "sfn"));
        TS_ASSERT_EQUALS(server::toString(p.get()), "hu");
    }

    // getInfo
    {
        Hash::Ref_t ret = Hash::create();
        ret->setNew("name", server::makeStringValue("Talk"));
        ret->setNew("newsgroup", server::makeStringValue("pcc.talk.ng"));
        ret->setNew("parent", server::makeStringValue("dad"));
        mock.expectCall("FORUMSTAT, 124");
        mock.provideNewResult(new HashValue(ret));

        server::interface::TalkForum::Info info = testee.getInfo(124);
        TS_ASSERT_EQUALS(info.name, "Talk");
        TS_ASSERT_EQUALS(info.parentGroup, "dad");
        TS_ASSERT_EQUALS(info.description, "");   // missing in provided hash
        TS_ASSERT_EQUALS(info.newsgroupName, "pcc.talk.ng");
    }

    // getInfo
    {
        Hash::Ref_t ret = Hash::create();
        ret->setNew("name", server::makeStringValue("Talk 2"));
        ret->setNew("newsgroup", server::makeStringValue("pcc.talk.ng2"));
        ret->setNew("parent", server::makeStringValue("root"));
        ret->setNew("description", server::makeStringValue("Desc..."));

        Vector::Ref_t retArray = Vector::create();
        retArray->pushBackNew(0);
        retArray->pushBackNew(new HashValue(ret));
        mock.expectCall("FORUMMSTAT, 77, 78");
        mock.provideNewResult(new VectorValue(retArray));

        static const int32_t fids[] = { 77, 78 };
        afl::container::PtrVector<server::interface::TalkForum::Info> result;
        testee.getInfo(fids, result);
        TS_ASSERT_EQUALS(result.size(), 2U);
        TS_ASSERT(result[0] == 0);
        TS_ASSERT(result[1] != 0);
        TS_ASSERT_EQUALS(result[1]->name, "Talk 2");
        TS_ASSERT_EQUALS(result[1]->parentGroup, "root");
        TS_ASSERT_EQUALS(result[1]->newsgroupName, "pcc.talk.ng2");
        TS_ASSERT_EQUALS(result[1]->description, "Desc...");
    }

    // getPermissions
    mock.expectCall("FORUMPERMS, 42");
    mock.provideNewResult(server::makeIntegerValue(0));
    TS_ASSERT_EQUALS(testee.getPermissions(42, afl::base::Nothing), 0);

    {
        mock.expectCall("FORUMPERMS, 43, write, read, answer");
        mock.provideNewResult(server::makeIntegerValue(7));
        const String_t perms[] = {"write","read","answer"};
        TS_ASSERT_EQUALS(testee.getPermissions(43, perms), 7);
    }

    // getSize
    {
        Hash::Ref_t ret = Hash::create();
        ret->setNew("threads", server::makeIntegerValue(42));
        ret->setNew("stickythreads", server::makeIntegerValue(2));
        ret->setNew("messages", server::makeIntegerValue(1701));
        mock.expectCall("FORUMSIZE, 32168");
        mock.provideNewResult(new HashValue(ret));

        server::interface::TalkForum::Size sz = testee.getSize(32168);
        TS_ASSERT_EQUALS(sz.numThreads, 42);
        TS_ASSERT_EQUALS(sz.numStickyThreads, 2);
        TS_ASSERT_EQUALS(sz.numMessages, 1701);
    }

    // getThreads
    {
        // - plain
        mock.expectCall("FORUMLSTHREAD, 9");
        mock.provideNewResult(new VectorValue(Vector::create(Segment().pushBackInteger(1).pushBackInteger(3))));
        std::auto_ptr<Value> p(testee.getThreads(9, server::interface::TalkForum::ListParameters()));

        afl::data::Access a(p);
        TS_ASSERT_EQUALS(a.getArraySize(), 2U);
        TS_ASSERT_EQUALS(a[0].toInteger(), 1);
        TS_ASSERT_EQUALS(a[1].toInteger(), 3);
    }
    {
        // - plain with sort
        mock.expectCall("FORUMLSTHREAD, 9, SORT, author");
        mock.provideNewResult(new VectorValue(Vector::create(Segment().pushBackInteger(11).pushBackInteger(13))));
        server::interface::TalkForum::ListParameters params;
        params.sortKey = "author";
        std::auto_ptr<Value> p(testee.getThreads(9, params));

        afl::data::Access a(p);
        TS_ASSERT_EQUALS(a.getArraySize(), 2U);
    }
    {
        // - limited
        mock.expectCall("FORUMLSTHREAD, 9, LIMIT, 10, 20");
        mock.provideNewResult(new VectorValue(Vector::create(Segment().pushBackInteger(11).pushBackInteger(13))));
        server::interface::TalkForum::ListParameters params;
        params.mode = params.WantRange;
        params.start = 10;
        params.count = 20;
        std::auto_ptr<Value> p(testee.getThreads(9, params));

        afl::data::Access a(p);
        TS_ASSERT_EQUALS(a.getArraySize(), 2U);
    }
    {
        // - limited with sort
        mock.expectCall("FORUMLSTHREAD, 9, LIMIT, 10, 20, SORT, time");
        mock.provideNewResult(new VectorValue(Vector::create(Segment().pushBackInteger(11).pushBackInteger(13))));
        server::interface::TalkForum::ListParameters params;
        params.mode = params.WantRange;
        params.start = 10;
        params.count = 20;
        params.sortKey = "time";
        std::auto_ptr<Value> p(testee.getThreads(9, params));

        afl::data::Access a(p);
        TS_ASSERT_EQUALS(a.getArraySize(), 2U);
    }
    {
        // - size
        mock.expectCall("FORUMLSTHREAD, 9, SIZE");
        mock.provideNewResult(server::makeIntegerValue(7));
        server::interface::TalkForum::ListParameters params;
        params.mode = params.WantSize;
        std::auto_ptr<Value> p(testee.getThreads(9, params));

        afl::data::Access a(p);
        TS_ASSERT_EQUALS(a.toInteger(), 7);
    }
    {
        // - member check
        mock.expectCall("FORUMLSTHREAD, 9, CONTAINS, 12");
        mock.provideNewResult(server::makeIntegerValue(1));
        server::interface::TalkForum::ListParameters params;
        params.mode = params.WantMemberCheck;
        params.item = 12;
        std::auto_ptr<Value> p(testee.getThreads(9, params));

        afl::data::Access a(p);
        TS_ASSERT_EQUALS(a.toInteger(), 1);
    }

    // getStickyThreads
    {
        // - plain
        mock.expectCall("FORUMLSSTICKY, 85");
        mock.provideNewResult(new VectorValue(Vector::create(Segment().pushBackInteger(1).pushBackInteger(3))));
        std::auto_ptr<Value> p(testee.getStickyThreads(85, server::interface::TalkForum::ListParameters()));

        afl::data::Access a(p);
        TS_ASSERT_EQUALS(a.getArraySize(), 2U);
        TS_ASSERT_EQUALS(a[0].toInteger(), 1);
        TS_ASSERT_EQUALS(a[1].toInteger(), 3);
    }
    {
        // - plain with sort
        // (we assume that if this variation works, the others work, too).
        mock.expectCall("FORUMLSSTICKY, 86, SORT, name");
        mock.provideNewResult(new VectorValue(Vector::create(Segment().pushBackInteger(11).pushBackInteger(13))));
        server::interface::TalkForum::ListParameters params;
        params.sortKey = "name";
        std::auto_ptr<Value> p(testee.getStickyThreads(86, params));

        afl::data::Access a(p);
        TS_ASSERT_EQUALS(a.getArraySize(), 2U);
    }

    // getPosts
    {
        // - plain
        mock.expectCall("FORUMLSPOST, 1");
        mock.provideNewResult(new VectorValue(Vector::create(Segment().pushBackInteger(1).pushBackInteger(3).pushBackInteger(8))));
        std::auto_ptr<Value> p(testee.getPosts(1, server::interface::TalkForum::ListParameters()));

        afl::data::Access a(p);
        TS_ASSERT_EQUALS(a.getArraySize(), 3U);
        TS_ASSERT_EQUALS(a[0].toInteger(), 1);
        TS_ASSERT_EQUALS(a[1].toInteger(), 3);
        TS_ASSERT_EQUALS(a[2].toInteger(), 8);
    }
    {
        // - plain with sort
        // (we assume that if this variation works, the others work, too).
        mock.expectCall("FORUMLSPOST, 2, SORT, name");
        mock.provideNewResult(new VectorValue(Vector::create(Segment().pushBackInteger(11).pushBackInteger(13))));
        server::interface::TalkForum::ListParameters params;
        params.sortKey = "name";
        std::auto_ptr<Value> p(testee.getPosts(2, params));

        afl::data::Access a(p);
        TS_ASSERT_EQUALS(a.getArraySize(), 2U);
    }
    mock.checkFinish();
}

