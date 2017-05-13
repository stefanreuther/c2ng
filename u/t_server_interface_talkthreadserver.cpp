/**
  *  \file u/t_server_interface_talkthreadserver.cpp
  *  \brief Test for server::interface::TalkThreadServer
  */

#include <memory>
#include <stdexcept>
#include "server/interface/talkthreadserver.hpp"

#include "t_server_interface.hpp"
#include "server/interface/talkthread.hpp"
#include "u/helper/callreceiver.hpp"
#include "afl/string/format.hpp"
#include "server/interface/talkthreadclient.hpp"
#include "afl/data/segment.hpp"
#include "afl/data/access.hpp"

using afl::data::Segment;
using afl::string::Format;

namespace {
    class TalkThreadMock : public server::interface::TalkThread, public CallReceiver {
     public:
        virtual Info getInfo(int32_t threadId)
            {
                checkCall(Format("getInfo(%d)", threadId));
                return consumeReturnValue<Info>();
            }
        virtual void getInfo(afl::base::Memory<const int32_t> threadIds, afl::container::PtrVector<Info>& result)
            {
                String_t cmd = "getInfo(";
                while (const int32_t* p = threadIds.eat()) {
                    cmd += Format("%d", *p);
                    if (!threadIds.empty()) {
                        cmd += ",";
                    }
                    result.pushBackNew(consumeReturnValue<Info*>());
                }
                cmd += ")";
                checkCall(cmd);
            }
        virtual afl::data::Value* getPosts(int32_t threadId, const ListParameters& params)
            {
                checkCall(Format("getPosts(%d,%s)", threadId, formatListParameters(params)));
                return consumeReturnValue<afl::data::Value*>();
            }
        virtual void setSticky(int32_t threadId, bool flag)
            {
                checkCall(Format("setSticky(%d,%d)", threadId, int(flag)));
            }
        virtual int getPermissions(int32_t threadId, afl::base::Memory<const String_t> permissionList)
            {
                String_t cmd = Format("getPermissions(%d", threadId);
                while (const String_t* p = permissionList.eat()) {
                    cmd += ",";
                    cmd += *p;
                }
                cmd += ")";
                checkCall(cmd);
                return consumeReturnValue<int>();
            }
        virtual void moveToForum(int32_t threadId, int32_t forumId)
            {
                checkCall(Format("moveToForum(%d,%d)", threadId, forumId));
            }
        virtual bool remove(int32_t threadId)
            {
                checkCall(Format("remove(%d)", threadId));
                return consumeReturnValue<bool>();
            }

     private:
        // FIXME: duplicate from TalkForumServer test
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

void
TestServerInterfaceTalkThreadServer::testIt()
{
    TalkThreadMock mock;
    server::interface::TalkThreadServer testee(mock);

    // getInfo
    {
        TalkThreadMock::Info info;
        info.subject = "Su";
        info.forumId = 6;
        info.firstPostId = 1;
        info.lastPostId = 20;
        info.lastTime = 777777;
        info.isSticky = true;
        mock.expectCall("getInfo(1221)");
        mock.provideReturnValue(info);

        std::auto_ptr<afl::data::Value> p(testee.call(Segment().pushBackString("THREADSTAT").pushBackInteger(1221)));
        TS_ASSERT(p.get() != 0);

        afl::data::Access a(p);
        TS_ASSERT_EQUALS(a("subject").toString(), "Su");
        TS_ASSERT_EQUALS(a("forum").toInteger(), 6);
        TS_ASSERT_EQUALS(a("firstpost").toInteger(), 1);
        TS_ASSERT_EQUALS(a("lastpost").toInteger(), 20);
        TS_ASSERT_EQUALS(a("lasttime").toInteger(), 777777);
        TS_ASSERT_EQUALS(a("sticky").toInteger(), 1);
    }

    // getInfo
    {
        TalkThreadMock::Info info;
        info.subject = "Su1";
        info.forumId = 6;
        info.firstPostId = 1;
        info.lastPostId = 20;
        info.lastTime = 777777;
        info.isSticky = true;

        mock.expectCall("getInfo(55,69,105)");
        mock.provideReturnValue<TalkThreadMock::Info*>(new TalkThreadMock::Info(info));
        mock.provideReturnValue<TalkThreadMock::Info*>(0);
        info.subject = "Su2";
        mock.provideReturnValue<TalkThreadMock::Info*>(new TalkThreadMock::Info(info));

        std::auto_ptr<afl::data::Value> p(testee.call(Segment().pushBackString("THREADMSTAT").pushBackInteger(55).pushBackInteger(69).pushBackInteger(105)));
        TS_ASSERT(p.get() != 0);

        afl::data::Access a(p);
        TS_ASSERT_EQUALS(a.getArraySize(), 3U);
        TS_ASSERT(a[0].getValue() != 0);
        TS_ASSERT(a[1].getValue() == 0);
        TS_ASSERT(a[2].getValue() != 0);
        TS_ASSERT_EQUALS(a[0]("subject").toString(), "Su1");
        TS_ASSERT_EQUALS(a[2]("subject").toString(), "Su2");
    }

    // getPosts
    mock.expectCall("getPosts(12,all)");
    mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(3));
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("THREADLSPOST").pushBackInteger(12)), 3);

    mock.expectCall("getPosts(12,all,sort(EDITTIME))");
    mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(5));
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("THREADLSPOST").pushBackInteger(12).pushBackString("SORT").pushBackString("edittime")), 5);

    mock.expectCall("getPosts(12,size)");
    mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(15));
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("THREADLSPOST").pushBackInteger(12).pushBackString("SIZE")), 15);

    // setSticky
    mock.expectCall("setSticky(13,1)");
    testee.callVoid(Segment().pushBackString("THREADSTICKY").pushBackInteger(13).pushBackInteger(1));

    // getPermissions
    mock.expectCall("getPermissions(6)");
    mock.provideReturnValue<int>(0);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("THREADPERMS").pushBackInteger(6)), 0);

    mock.expectCall("getPermissions(6,r,w,x)");
    mock.provideReturnValue<int>(5);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("THREADPERMS").pushBackInteger(6).pushBackString("r").pushBackString("w").pushBackString("x")), 5);

    // moveToForum
    mock.expectCall("moveToForum(100,3)");
    testee.callVoid(Segment().pushBackString("THREADMV").pushBackInteger(100).pushBackInteger(3));

    // remove
    mock.expectCall("remove(78)");
    mock.provideReturnValue<bool>(true);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("THREADRM").pushBackInteger(78)), 1);

    mock.expectCall("remove(79)");
    mock.provideReturnValue<bool>(false);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("THREADRM").pushBackInteger(79)), 0);

    // Variations
    mock.expectCall("moveToForum(100,3)");
    testee.callVoid(Segment().pushBackString("threadmv").pushBackInteger(100).pushBackInteger(3));

    mock.expectCall("getPosts(12,all,sort(EDITTIME))");
    mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(5));
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("THREADLSPOST").pushBackInteger(12).pushBackString("sort").pushBackString("Edittime")), 5);

    mock.checkFinish();
}

/** Test errors. */
void
TestServerInterfaceTalkThreadServer::testErrors()
{
    TalkThreadMock mock;
    server::interface::TalkThreadServer testee(mock);

    // Bad command
    Segment empty;    // g++-3.4 sees an invocation of a copy constructor if I construct this object in-place.
    TS_ASSERT_THROWS(testee.callInt(empty), std::exception);
    TS_ASSERT_THROWS(testee.callInt(Segment().pushBackString("HUHU")), std::exception);

    // Bad arg count
    TS_ASSERT_THROWS(testee.callInt(Segment().pushBackString("THREADRM")), std::exception);
    TS_ASSERT_THROWS(testee.callInt(Segment().pushBackString("THREADRM").pushBackInteger(78).pushBackInteger(78)), std::exception);

    // ComposableCommandHandler personality
    interpreter::Arguments args(empty, 0, 0);
    std::auto_ptr<afl::data::Value> p;
    TS_ASSERT_EQUALS(testee.handleCommand("huhu", args, p), false);
}

/** Test roundtrip behaviour. */
void
TestServerInterfaceTalkThreadServer::testRoundtrip()
{
    TalkThreadMock mock;
    server::interface::TalkThreadServer level1(mock);
    server::interface::TalkThreadClient level2(level1);
    server::interface::TalkThreadServer level3(level2);
    server::interface::TalkThreadClient level4(level3);

    typedef server::interface::TalkThread::Info Info_t;
    typedef server::interface::TalkThread::ListParameters ListParameters_t;

    // getInfo
    {
        Info_t info;
        info.subject = "Su";
        info.forumId = 6;
        info.firstPostId = 1;
        info.lastPostId = 20;
        info.lastTime = 777777;
        info.isSticky = true;
        mock.expectCall("getInfo(1221)");
        mock.provideReturnValue(info);

        Info_t out = level4.getInfo(1221);

        TS_ASSERT_EQUALS(out.subject, "Su");
        TS_ASSERT_EQUALS(out.forumId, 6);
        TS_ASSERT_EQUALS(out.firstPostId, 1);
        TS_ASSERT_EQUALS(out.lastPostId, 20);
        TS_ASSERT_EQUALS(out.lastTime, 777777);
        TS_ASSERT_EQUALS(out.isSticky, 1);
    }

    // getInfo
    {
        Info_t info;
        info.subject = "Su1";
        info.forumId = 6;
        info.firstPostId = 1;
        info.lastPostId = 20;
        info.lastTime = 777777;
        info.isSticky = true;

        mock.expectCall("getInfo(55,69,105)");
        mock.provideReturnValue<Info_t*>(new Info_t(info));
        mock.provideReturnValue<Info_t*>(0);
        info.subject = "Su2";
        mock.provideReturnValue<Info_t*>(new Info_t(info));

        afl::container::PtrVector<Info_t> result;
        static const int32_t threadIds[] = {55,69,105};
        level4.getInfo(threadIds, result);

        TS_ASSERT_EQUALS(result.size(), 3U);
        TS_ASSERT(result[0] != 0);
        TS_ASSERT(result[1] == 0);
        TS_ASSERT(result[2] != 0);
        TS_ASSERT_EQUALS(result[0]->subject, "Su1");
        TS_ASSERT_EQUALS(result[2]->subject, "Su2");
    }

    // getPosts
    {
        mock.expectCall("getPosts(12,all)");
        mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(3));
        std::auto_ptr<afl::data::Value> result(level4.getPosts(12, ListParameters_t()));
        TS_ASSERT_EQUALS(server::toInteger(result.get()), 3);
    }
    {
        mock.expectCall("getPosts(12,all,sort(EDITTIME))");
        mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(5));
        ListParameters_t params;
        params.sortKey = "EDITTIME";
        std::auto_ptr<afl::data::Value> result(level4.getPosts(12, params));
        TS_ASSERT_EQUALS(server::toInteger(result.get()), 5);
    }
    {
        mock.expectCall("getPosts(12,size)");
        mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(15));
        ListParameters_t params;
        params.mode = params.WantSize;
        std::auto_ptr<afl::data::Value> result(level4.getPosts(12, params));
        TS_ASSERT_EQUALS(server::toInteger(result.get()), 15);
    }

    // setSticky
    mock.expectCall("setSticky(13,1)");
    level4.setSticky(13, true);

    // getPermissions
    mock.expectCall("getPermissions(6)");
    mock.provideReturnValue<int>(0);
    TS_ASSERT_EQUALS(level4.getPermissions(6, afl::base::Nothing), 0);

    {
        String_t perms[] = {"r","w","x"};
        mock.expectCall("getPermissions(6,r,w,x)");
        mock.provideReturnValue<int>(5);
        TS_ASSERT_EQUALS(level4.getPermissions(6, perms), 5);
    }

    // moveToForum
    mock.expectCall("moveToForum(100,3)");
    level4.moveToForum(100, 3);

    // remove
    mock.expectCall("remove(78)");
    mock.provideReturnValue<bool>(true);
    TS_ASSERT_EQUALS(level4.remove(78), true);

    mock.expectCall("remove(79)");
    mock.provideReturnValue<bool>(false);
    TS_ASSERT_EQUALS(level4.remove(79), false);

    mock.checkFinish();
}

