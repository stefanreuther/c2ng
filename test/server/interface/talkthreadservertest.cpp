/**
  *  \file test/server/interface/talkthreadservertest.cpp
  *  \brief Test for server::interface::TalkThreadServer
  */

#include "server/interface/talkthreadserver.hpp"

#include "afl/data/access.hpp"
#include "afl/data/segment.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"
#include "server/interface/talkthread.hpp"
#include "server/interface/talkthreadclient.hpp"
#include <memory>
#include <stdexcept>

using afl::data::Segment;
using afl::string::Format;

namespace {
    class TalkThreadMock : public server::interface::TalkThread, public afl::test::CallReceiver {
     public:
        TalkThreadMock(afl::test::Assert a)
            : CallReceiver(a)
            { }
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

AFL_TEST("server.interface.TalkThreadServer:commands", a)
{
    TalkThreadMock mock(a);
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
        info.alsoPostedTo.push_back(32);
        info.alsoPostedTo.push_back(27);
        mock.expectCall("getInfo(1221)");
        mock.provideReturnValue(info);

        std::auto_ptr<afl::data::Value> p(testee.call(Segment().pushBackString("THREADSTAT").pushBackInteger(1221)));
        a.checkNonNull("01", p.get());

        afl::data::Access ap(p);
        a.checkEqual("11. subject",   ap("subject").toString(), "Su");
        a.checkEqual("12. forum",     ap("forum").toInteger(), 6);
        a.checkEqual("13. firstpost", ap("firstpost").toInteger(), 1);
        a.checkEqual("14. lastpost",  ap("lastpost").toInteger(), 20);
        a.checkEqual("15. lasttime",  ap("lasttime").toInteger(), 777777);
        a.checkEqual("16. sticky",    ap("sticky").toInteger(), 1);
        a.checkEqual("17. also",      ap("also").getArraySize(), 2U);
        a.checkEqual("17a. also",     ap("also")[0].toInteger(), 32);
        a.checkEqual("17b. also",     ap("also")[1].toInteger(), 27);
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
        a.checkNonNull("21", p.get());

        afl::data::Access ap(p);
        a.checkEqual  ("31. getArraySize", ap.getArraySize(), 3U);
        a.checkNonNull("32. entry",   ap[0].getValue());
        a.checkNull   ("33. entry",   ap[1].getValue());
        a.checkNonNull("34. entry",   ap[2].getValue());
        a.checkEqual  ("35. subject", ap[0]("subject").toString(), "Su1");
        a.checkEqual  ("36. subject", ap[2]("subject").toString(), "Su2");
    }

    // getPosts
    mock.expectCall("getPosts(12,all)");
    mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(3));
    a.checkEqual("41. threadlspost", testee.callInt(Segment().pushBackString("THREADLSPOST").pushBackInteger(12)), 3);

    mock.expectCall("getPosts(12,all,sort(EDITTIME))");
    mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(5));
    a.checkEqual("51. threadlspost", testee.callInt(Segment().pushBackString("THREADLSPOST").pushBackInteger(12).pushBackString("SORT").pushBackString("edittime")), 5);

    mock.expectCall("getPosts(12,size)");
    mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(15));
    a.checkEqual("61. threadlspost", testee.callInt(Segment().pushBackString("THREADLSPOST").pushBackInteger(12).pushBackString("SIZE")), 15);

    // setSticky
    mock.expectCall("setSticky(13,1)");
    testee.callVoid(Segment().pushBackString("THREADSTICKY").pushBackInteger(13).pushBackInteger(1));

    // getPermissions
    mock.expectCall("getPermissions(6)");
    mock.provideReturnValue<int>(0);
    a.checkEqual("71. threadperms", testee.callInt(Segment().pushBackString("THREADPERMS").pushBackInteger(6)), 0);

    mock.expectCall("getPermissions(6,r,w,x)");
    mock.provideReturnValue<int>(5);
    a.checkEqual("81. threadperms", testee.callInt(Segment().pushBackString("THREADPERMS").pushBackInteger(6).pushBackString("r").pushBackString("w").pushBackString("x")), 5);

    // moveToForum
    mock.expectCall("moveToForum(100,3)");
    testee.callVoid(Segment().pushBackString("THREADMV").pushBackInteger(100).pushBackInteger(3));

    // remove
    mock.expectCall("remove(78)");
    mock.provideReturnValue<bool>(true);
    a.checkEqual("91. threadrm", testee.callInt(Segment().pushBackString("THREADRM").pushBackInteger(78)), 1);

    mock.expectCall("remove(79)");
    mock.provideReturnValue<bool>(false);
    a.checkEqual("101. threadrm", testee.callInt(Segment().pushBackString("THREADRM").pushBackInteger(79)), 0);

    // Variations
    mock.expectCall("moveToForum(100,3)");
    testee.callVoid(Segment().pushBackString("threadmv").pushBackInteger(100).pushBackInteger(3));

    mock.expectCall("getPosts(12,all,sort(EDITTIME))");
    mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(5));
    a.checkEqual("111. threadlspost", testee.callInt(Segment().pushBackString("THREADLSPOST").pushBackInteger(12).pushBackString("sort").pushBackString("Edittime")), 5);

    mock.checkFinish();
}

/** Test errors. */
AFL_TEST("server.interface.TalkThreadServer:errors", a)
{
    TalkThreadMock mock(a);
    server::interface::TalkThreadServer testee(mock);

    // Bad command
    Segment empty;    // g++-3.4 sees an invocation of a copy constructor if I construct this object in-place.
    AFL_CHECK_THROWS(a("01. no verb"),  testee.callInt(empty), std::exception);
    AFL_CHECK_THROWS(a("02. bad verb"), testee.callInt(Segment().pushBackString("HUHU")), std::exception);

    // Bad arg count
    AFL_CHECK_THROWS(a("11. missing args"),  testee.callInt(Segment().pushBackString("THREADRM")), std::exception);
    AFL_CHECK_THROWS(a("12. too many args"), testee.callInt(Segment().pushBackString("THREADRM").pushBackInteger(78).pushBackInteger(78)), std::exception);

    // ComposableCommandHandler personality
    interpreter::Arguments args(empty, 0, 0);
    std::auto_ptr<afl::data::Value> p;
    a.checkEqual("21. bad arg", testee.handleCommand("huhu", args, p), false);
}

/** Test roundtrip behaviour. */
AFL_TEST("server.interface.TalkThreadServer:roundtrip", a)
{
    TalkThreadMock mock(a);
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
        info.alsoPostedTo.push_back(47);
        info.alsoPostedTo.push_back(11);
        mock.expectCall("getInfo(1221)");
        mock.provideReturnValue(info);

        Info_t out = level4.getInfo(1221);

        a.checkEqual("01. subject",     out.subject, "Su");
        a.checkEqual("02. forumId",     out.forumId, 6);
        a.checkEqual("03. firstPostId", out.firstPostId, 1);
        a.checkEqual("04. lastPostId",  out.lastPostId, 20);
        a.checkEqual("05. lastTime",    out.lastTime, 777777);
        a.checkEqual("06. isSticky",    out.isSticky, 1);
        a.checkEqual("07. also",        out.alsoPostedTo.size(), 2U);
        a.checkEqual("07a. also",       out.alsoPostedTo[0], 47);
        a.checkEqual("07b. also",       out.alsoPostedTo[1], 11);
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

        a.checkEqual  ("11. size",    result.size(), 3U);
        a.checkNonNull("12. entry",   result[0]);
        a.checkNull   ("13. entry",   result[1]);
        a.checkNonNull("14. entry",   result[2]);
        a.checkEqual  ("15. subject", result[0]->subject, "Su1");
        a.checkEqual  ("16. subject", result[2]->subject, "Su2");
    }

    // getPosts
    {
        mock.expectCall("getPosts(12,all)");
        mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(3));
        std::auto_ptr<afl::data::Value> result(level4.getPosts(12, ListParameters_t()));
        a.checkEqual("21. getPosts", server::toInteger(result.get()), 3);
    }
    {
        mock.expectCall("getPosts(12,all,sort(EDITTIME))");
        mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(5));
        ListParameters_t params;
        params.sortKey = "EDITTIME";
        std::auto_ptr<afl::data::Value> result(level4.getPosts(12, params));
        a.checkEqual("22. getPosts", server::toInteger(result.get()), 5);
    }
    {
        mock.expectCall("getPosts(12,size)");
        mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(15));
        ListParameters_t params;
        params.mode = params.WantSize;
        std::auto_ptr<afl::data::Value> result(level4.getPosts(12, params));
        a.checkEqual("23. getPosts", server::toInteger(result.get()), 15);
    }

    // setSticky
    mock.expectCall("setSticky(13,1)");
    level4.setSticky(13, true);

    // getPermissions
    mock.expectCall("getPermissions(6)");
    mock.provideReturnValue<int>(0);
    a.checkEqual("31. getPermissions", level4.getPermissions(6, afl::base::Nothing), 0);

    {
        String_t perms[] = {"r","w","x"};
        mock.expectCall("getPermissions(6,r,w,x)");
        mock.provideReturnValue<int>(5);
        a.checkEqual("41. getPermissions", level4.getPermissions(6, perms), 5);
    }

    // moveToForum
    mock.expectCall("moveToForum(100,3)");
    level4.moveToForum(100, 3);

    // remove
    mock.expectCall("remove(78)");
    mock.provideReturnValue<bool>(true);
    a.checkEqual("51. remove", level4.remove(78), true);

    mock.expectCall("remove(79)");
    mock.provideReturnValue<bool>(false);
    a.checkEqual("61. remove", level4.remove(79), false);

    mock.checkFinish();
}
