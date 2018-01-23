/**
  *  \file u/t_server_interface_talkforumserver.cpp
  *  \brief Test for server::interface::TalkForumServer
  */

#include "server/interface/talkforumserver.hpp"

#include <stdexcept>
#include "t_server_interface.hpp"
#include "afl/data/access.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "server/interface/talkforum.hpp"
#include "server/interface/talkforumclient.hpp"
#include "server/types.hpp"

using afl::string::Format;
using afl::data::Segment;

namespace {
    class TalkForumMock : public server::interface::TalkForum, public afl::test::CallReceiver {
     public:
        TalkForumMock(afl::test::Assert a)
            : CallReceiver(a)
            { }
        virtual int32_t add(afl::base::Memory<const String_t> config)
            {
                String_t cmd = "add(";
                while (const String_t* p = config.eat()) {
                    cmd += *p;
                    if (!config.empty()) {
                        cmd += ",";
                    }
                }
                cmd += ")";
                checkCall(cmd);
                return consumeReturnValue<int32_t>();
            }
        virtual void configure(int32_t fid, afl::base::Memory<const String_t> config)
            {
                String_t cmd = Format("configure(%s", fid);
                while (const String_t* p = config.eat()) {
                    cmd += ",";
                    cmd += *p;
                }
                cmd += ")";
                checkCall(cmd);
            }

        virtual afl::data::Value* getValue(int32_t fid, String_t keyName)
            {
                checkCall(Format("getValue(%d,%s)", fid, keyName));
                return consumeReturnValue<afl::data::Value*>();
            }

        virtual Info getInfo(int32_t fid)
            {
                checkCall(Format("getInfo(%d)", fid));
                return consumeReturnValue<Info>();
            }

        virtual void getInfo(afl::base::Memory<const int32_t> fids, afl::container::PtrVector<Info>& result)
            {
                String_t cmd = "getInfos(";
                while (const int32_t* p = fids.eat()) {
                    result.pushBackNew(consumeReturnValue<Info*>());
                    cmd += Format("%d", *p);
                    if (!fids.empty()) {
                        cmd += ",";
                    }
                }
                cmd += ")";
                checkCall(cmd);
            }

        virtual int32_t getPermissions(int32_t fid, afl::base::Memory<const String_t> permissionList)
            {
                String_t cmd = Format("getPermissions(%d", fid);
                while (const String_t* p = permissionList.eat()) {
                    cmd += ",";
                    cmd += *p;
                }
                cmd += ")";
                checkCall(cmd);
                return consumeReturnValue<int32_t>();
            }

        virtual Size getSize(int32_t fid)
            {
                checkCall(Format("getSize(%d)", fid));
                return consumeReturnValue<Size>();
            }

        virtual afl::data::Value* getThreads(int32_t fid, const ListParameters& params)
            {
                checkCall(Format("getThreads(%d,%s)", fid, formatListParameters(params)));
                return consumeReturnValue<afl::data::Value*>();
            }

        virtual afl::data::Value* getStickyThreads(int32_t fid, const ListParameters& params)
            {
                checkCall(Format("getStickyThreads(%d,%s)", fid, formatListParameters(params)));
                return consumeReturnValue<afl::data::Value*>();
            }

        virtual afl::data::Value* getPosts(int32_t fid, const ListParameters& params)
            {
                checkCall(Format("getPosts(%d,%s)", fid, formatListParameters(params)));
                return consumeReturnValue<afl::data::Value*>();
            }

        static String_t formatListParameters(const ListParameters& params)
            {
                String_t result;
                switch (params.mode) {
                 case ListParameters::WantAll:
                    result = "all";
                    break;
                 case ListParameters::WantRange:
                    result = Format("range(%d,%d)", params.start, params.count);
                    break;
                 case ListParameters::WantSize:
                    result = "size";
                    break;
                 case ListParameters::WantMemberCheck:
                    result = Format("member(%d)", params.item);
                    break;
                }
                if (const String_t* p = params.sortKey.get()) {
                    result += Format(",sort(%s)", *p);
                }
                return result;
            }
    };
}

/** Test calls. */
void
TestServerInterfaceTalkForumServer::testIt()
{
    TalkForumMock mock("testIt");
    server::interface::TalkForumServer testee(mock);

    // add/FORUMADD
    mock.expectCall("add()");
    mock.provideReturnValue<int32_t>(7);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("FORUMADD")), 7);

    mock.expectCall("add(name,New Forum,description,More info...)");
    mock.provideReturnValue<int32_t>(8);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("FORUMADD").pushBackString("name").pushBackString("New Forum").pushBackString("description").pushBackString("More info...")), 8);

    // configure/FORUMSET
    mock.expectCall("configure(8)");
    testee.callVoid(Segment().pushBackString("FORUMSET").pushBackInteger(8));
    mock.expectCall("configure(7,name,Old Forum)");
    testee.callVoid(Segment().pushBackString("FORUMSET").pushBackInteger(7).pushBackString("name").pushBackString("Old Forum"));

    // getValue/FORUMGET
    {
        mock.expectCall("getValue(12,vv1)");
        mock.provideReturnValue<afl::data::Value*>(0);
        std::auto_ptr<afl::data::Value> p(testee.call(Segment().pushBackString("FORUMGET").pushBackInteger(12).pushBackString("vv1")));
        TS_ASSERT(p.get() == 0);
    }

    mock.expectCall("getValue(13,vv2)");
    mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(47));
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("FORUMGET").pushBackInteger(13).pushBackString("vv2")), 47);

    // getInfo/FORUMSTAT
    {
        server::interface::TalkForum::Info in;
        in.name = "theName";
        in.parentGroup = "theGroup";
        in.description = "theDescription";
        in.newsgroupName = "theNewsgroup";
        mock.expectCall("getInfo(77)");
        mock.provideReturnValue(in);

        std::auto_ptr<afl::data::Value> p(testee.call(Segment().pushBackString("FORUMSTAT").pushBackInteger(77)));
        TS_ASSERT(p.get() != 0);

        afl::data::Access a(p);
        TS_ASSERT_EQUALS(a("name").toString(), "theName");
        TS_ASSERT_EQUALS(a("parent").toString(), "theGroup");
        TS_ASSERT_EQUALS(a("description").toString(), "theDescription");
        TS_ASSERT_EQUALS(a("newsgroup").toString(), "theNewsgroup");
    }

    // getInfo/FORUMMSTAT
    {
        server::interface::TalkForum::Info in;
        in.name = "theName";
        in.parentGroup = "theGroup";
        in.description = "theDescription";
        in.newsgroupName = "theNewsgroup";
        mock.provideReturnValue<server::interface::TalkForum::Info*>(new server::interface::TalkForum::Info(in));
        mock.provideReturnValue<server::interface::TalkForum::Info*>(0);
        in.name = "otherName";
        mock.provideReturnValue<server::interface::TalkForum::Info*>(new server::interface::TalkForum::Info(in));
        mock.expectCall("getInfos(7,8,9)");

        std::auto_ptr<afl::data::Value> p(testee.call(Segment().pushBackString("FORUMMSTAT").pushBackInteger(7).pushBackInteger(8).pushBackInteger(9)));
        TS_ASSERT(p.get() != 0);

        afl::data::Access a(p);
        TS_ASSERT_EQUALS(a.getArraySize(), 3U);
        TS_ASSERT_EQUALS(a[0]("name").toString(), "theName");
        TS_ASSERT(a[1].getValue() == 0);
        TS_ASSERT_EQUALS(a[2]("name").toString(), "otherName");
    }

    // getPermissions/FORUMPERMS
    mock.expectCall("getPermissions(3,read)");
    mock.provideReturnValue<int32_t>(7);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("FORUMPERMS").pushBackInteger(3).pushBackString("read")), 7);

    // getSize/FORUMSIZE
    {
        server::interface::TalkForum::Size sz;
        sz.numThreads = 3;
        sz.numStickyThreads = 1;
        sz.numMessages = 33;
        mock.expectCall("getSize(6)");
        mock.provideReturnValue(sz);

        std::auto_ptr<afl::data::Value> p(testee.call(Segment().pushBackString("FORUMSIZE").pushBackInteger(6)));
        TS_ASSERT(p.get() != 0);

        afl::data::Access a(p);
        TS_ASSERT_EQUALS(a("threads").toInteger(), 3);
        TS_ASSERT_EQUALS(a("stickythreads").toInteger(), 1);
        TS_ASSERT_EQUALS(a("messages").toInteger(), 33);
    }

    // getThreads/FORUMLSTHREAD
    mock.expectCall("getThreads(6,all)");
    mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(9));
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("FORUMLSTHREAD").pushBackInteger(6)), 9);

    mock.expectCall("getThreads(6,all,sort(TIME))");
    mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(9));
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("FORUMLSTHREAD").pushBackInteger(6).pushBackString("SORT").pushBackString("time")), 9);

    mock.expectCall("getThreads(6,range(10,20),sort(TIME))");
    mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(9));
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("FORUMLSTHREAD").pushBackInteger(6).pushBackString("SORT").pushBackString("time").pushBackString("LIMIT").pushBackInteger(10).pushBackInteger(20)), 9);

    mock.expectCall("getThreads(6,member(9))");
    mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(1));
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("FORUMLSTHREAD").pushBackInteger(6).pushBackString("CONTAINS").pushBackInteger(9)), 1);

    mock.expectCall("getThreads(6,size)");
    mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(71));
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("FORUMLSTHREAD").pushBackInteger(6).pushBackString("SIZE")), 71);

    // getStickyThreads/FORUMLSSTICKY
    mock.expectCall("getStickyThreads(6,all)");
    mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(9));
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("FORUMLSSTICKY").pushBackInteger(6)), 9);

    mock.expectCall("getStickyThreads(6,range(10,20),sort(TIME))");
    mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(9));
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("FORUMLSSTICKY").pushBackInteger(6).pushBackString("SORT").pushBackString("time").pushBackString("LIMIT").pushBackInteger(10).pushBackInteger(20)), 9);

    // getPosts/FORUMLSPOST
    mock.expectCall("getPosts(6,all)");
    mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(9));
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("FORUMLSPOST").pushBackInteger(6)), 9);

    mock.expectCall("getPosts(6,range(10,20),sort(TIME))");
    mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(9));
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("FORUMLSPOST").pushBackInteger(6).pushBackString("SORT").pushBackString("time").pushBackString("LIMIT").pushBackInteger(10).pushBackInteger(20)), 9);

    // Variations
    mock.expectCall("add()");
    mock.provideReturnValue<int32_t>(9);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("forumAdd")), 9);

    mock.expectCall("getStickyThreads(6,range(10,20),sort(TIME))");
    mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(9));
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("forumlssticky").pushBackInteger(6).pushBackString("sort").pushBackString("Time").pushBackString("limit").pushBackInteger(10).pushBackInteger(20)), 9);

    mock.checkFinish();
}

/** Test erroneous calls. */
void
TestServerInterfaceTalkForumServer::testErrors()
{
    TalkForumMock mock("testErrors");
    server::interface::TalkForumServer testee(mock);

    Segment empty;    // g++-3.4 sees an invocation of a copy constructor if I construct this object in-place.
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("UNKNOWN")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(empty), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("FORUMLSSTICKY")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("FORUMLSSTICKY").pushBackString("boom")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("FORUMLSSTICKY").pushBackInteger(6).pushBackString("sort")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("FORUMLSSTICKY").pushBackInteger(6).pushBackString("limit").pushBackInteger(10)), std::exception);

    interpreter::Arguments args(empty, 0, 0);
    std::auto_ptr<afl::data::Value> p;
    TS_ASSERT_EQUALS(testee.handleCommand("huhu", args, p), false);
}

/** Test roundtrip behaviour. */
void
TestServerInterfaceTalkForumServer::testRoundtrip()
{
    TalkForumMock mock("testRoundtrip");
    server::interface::TalkForumServer level1(mock);
    server::interface::TalkForumClient level2(level1);
    server::interface::TalkForumServer level3(level2);
    server::interface::TalkForumClient level4(level3);

    // // add/FORUMADD
    mock.expectCall("add()");
    mock.provideReturnValue<int32_t>(7);
    TS_ASSERT_EQUALS(level4.add(afl::base::Nothing), 7);
    {
        mock.expectCall("add(name,New Forum,description,More info...)");
        mock.provideReturnValue<int32_t>(8);
        const String_t args[] = {"name","New Forum","description","More info..."};
        TS_ASSERT_EQUALS(level4.add(args), 8);
    }

    // configure/FORUMSET
    mock.expectCall("configure(8)");
    level4.configure(8, afl::base::Nothing);
    {
        mock.expectCall("configure(7,name,Old Forum)");
        const String_t args[] = {"name","Old Forum"};
        level4.configure(7, args);
    }

    // getValue/FORUMGET
    {
        mock.expectCall("getValue(12,vv1)");
        mock.provideReturnValue<afl::data::Value*>(0);
        std::auto_ptr<afl::data::Value> p(level4.getValue(12, "vv1"));
        TS_ASSERT(p.get() == 0);
    }

    mock.expectCall("getValue(13,vv2)");
    mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(47));
    TS_ASSERT_EQUALS(level4.getIntegerValue(13, "vv2"), 47);

    // getInfo/FORUMSTAT
    {
        server::interface::TalkForum::Info in;
        in.name = "theName";
        in.parentGroup = "theGroup";
        in.description = "theDescription";
        in.newsgroupName = "theNewsgroup";
        mock.expectCall("getInfo(77)");
        mock.provideReturnValue(in);

        server::interface::TalkForum::Info out = level4.getInfo(77);
        TS_ASSERT_EQUALS(out.name, "theName");
        TS_ASSERT_EQUALS(out.parentGroup, "theGroup");
        TS_ASSERT_EQUALS(out.description, "theDescription");
        TS_ASSERT_EQUALS(out.newsgroupName, "theNewsgroup");
    }

    // getInfo/FORUMMSTAT
    {
        server::interface::TalkForum::Info in;
        in.name = "theName";
        in.parentGroup = "theGroup";
        in.description = "theDescription";
        in.newsgroupName = "theNewsgroup";
        mock.provideReturnValue<server::interface::TalkForum::Info*>(new server::interface::TalkForum::Info(in));
        mock.provideReturnValue<server::interface::TalkForum::Info*>(0);
        in.name = "otherName";
        mock.provideReturnValue<server::interface::TalkForum::Info*>(new server::interface::TalkForum::Info(in));
        mock.expectCall("getInfos(7,8,9)");

        afl::container::PtrVector<server::interface::TalkForum::Info> out;
        const int32_t fids[] = {7,8,9};
        level4.getInfo(fids, out);

        TS_ASSERT_EQUALS(out.size(), 3U);
        TS_ASSERT(out[0] != 0);
        TS_ASSERT(out[1] == 0);
        TS_ASSERT(out[2] != 0);
        TS_ASSERT_EQUALS(out[0]->name, "theName");
        TS_ASSERT_EQUALS(out[2]->name, "otherName");
    }

    // getPermissions/FORUMPERMS
    {
        const String_t perms[] = {"read","write","delete"};
        mock.expectCall("getPermissions(3,read,write,delete)");
        mock.provideReturnValue<int32_t>(7);
        TS_ASSERT_EQUALS(level4.getPermissions(3, perms), 7);
    }

    // getSize/FORUMSIZE
    {
        server::interface::TalkForum::Size sz;
        sz.numThreads = 3;
        sz.numStickyThreads = 1;
        sz.numMessages = 33;
        mock.expectCall("getSize(6)");
        mock.provideReturnValue(sz);

        server::interface::TalkForum::Size out = level4.getSize(6);
        TS_ASSERT_EQUALS(out.numThreads, 3);
        TS_ASSERT_EQUALS(out.numStickyThreads, 1);
        TS_ASSERT_EQUALS(out.numMessages, 33);
    }

    // getThreads/FORUMLSTHREAD
    {
        mock.expectCall("getThreads(6,all)");
        mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(9));
        std::auto_ptr<afl::data::Value> p(level4.getThreads(6, server::interface::TalkForum::ListParameters()));
        TS_ASSERT_EQUALS(server::toInteger(p.get()), 9);
    }

    {
        mock.expectCall("getThreads(6,all,sort(TIME))");
        mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(9));
        server::interface::TalkForum::ListParameters param;
        param.sortKey = "TIME";
        std::auto_ptr<afl::data::Value> p(level4.getThreads(6, param));
        TS_ASSERT_EQUALS(server::toInteger(p.get()), 9);
    }

    {
        mock.expectCall("getThreads(6,range(10,20),sort(TIME))");
        mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(9));
        server::interface::TalkForum::ListParameters param;
        param.sortKey = "TIME";
        param.mode = param.WantRange;
        param.start = 10;
        param.count = 20;
        std::auto_ptr<afl::data::Value> p(level4.getThreads(6, param));
        TS_ASSERT_EQUALS(server::toInteger(p.get()), 9);
    }

    {
        mock.expectCall("getThreads(6,member(9))");
        mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(9));
        server::interface::TalkForum::ListParameters param;
        param.mode = param.WantMemberCheck;
        param.item = 9;
        std::auto_ptr<afl::data::Value> p(level4.getThreads(6, param));
        TS_ASSERT_EQUALS(server::toInteger(p.get()), 9);
    }

    {
        mock.expectCall("getThreads(6,size)");
        mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(9));
        server::interface::TalkForum::ListParameters param;
        param.mode = param.WantSize;
        std::auto_ptr<afl::data::Value> p(level4.getThreads(6, param));
        TS_ASSERT_EQUALS(server::toInteger(p.get()), 9);
    }

    // getStickyThreads/FORUMLSSTICKY
    {
        mock.expectCall("getStickyThreads(6,all)");
        mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(9));
        std::auto_ptr<afl::data::Value> p(level4.getStickyThreads(6, server::interface::TalkForum::ListParameters()));
        TS_ASSERT_EQUALS(server::toInteger(p.get()), 9);
    }

    {
        mock.expectCall("getStickyThreads(6,range(10,20),sort(TIME))");
        mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(9));
        server::interface::TalkForum::ListParameters param;
        param.sortKey = "TIME";
        param.mode = param.WantRange;
        param.start = 10;
        param.count = 20;
        std::auto_ptr<afl::data::Value> p(level4.getStickyThreads(6, param));
        TS_ASSERT_EQUALS(server::toInteger(p.get()), 9);
    }

    // getPosts/FORUMLSPOST
    {
        mock.expectCall("getPosts(6,all)");
        mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(9));
        std::auto_ptr<afl::data::Value> p(level4.getPosts(6, server::interface::TalkForum::ListParameters()));
        TS_ASSERT_EQUALS(server::toInteger(p.get()), 9);
    }

    {
        mock.expectCall("getPosts(6,range(10,20),sort(TIME))");
        mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(9));
        server::interface::TalkForum::ListParameters param;
        param.sortKey = "TIME";
        param.mode = param.WantRange;
        param.start = 10;
        param.count = 20;
        std::auto_ptr<afl::data::Value> p(level4.getPosts(6, param));
        TS_ASSERT_EQUALS(server::toInteger(p.get()), 9);
    }

    mock.checkFinish();
}

