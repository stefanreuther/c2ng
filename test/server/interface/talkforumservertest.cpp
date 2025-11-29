/**
  *  \file test/server/interface/talkforumservertest.cpp
  *  \brief Test for server::interface::TalkForumServer
  */

#include "server/interface/talkforumserver.hpp"

#include "afl/data/access.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"
#include "server/interface/talkforum.hpp"
#include "server/interface/talkforumclient.hpp"
#include "server/types.hpp"
#include <stdexcept>

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
        virtual int32_t findForum(String_t key)
            {
                checkCall(Format("findForum(%d)", key));
                return consumeReturnValue<int>();
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
AFL_TEST("server.interface.TalkForumServer:commands", a)
{
    TalkForumMock mock(a);
    server::interface::TalkForumServer testee(mock);

    // add/FORUMADD
    mock.expectCall("add()");
    mock.provideReturnValue<int32_t>(7);
    a.checkEqual("01. forumadd", testee.callInt(Segment().pushBackString("FORUMADD")), 7);

    mock.expectCall("add(name,New Forum,description,More info...)");
    mock.provideReturnValue<int32_t>(8);
    a.checkEqual("11. forumadd", testee.callInt(Segment().pushBackString("FORUMADD").pushBackString("name").pushBackString("New Forum").pushBackString("description").pushBackString("More info...")), 8);

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
        a.checkNull("21. forumget", p.get());
    }

    mock.expectCall("getValue(13,vv2)");
    mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(47));
    a.checkEqual("31. forumget", testee.callInt(Segment().pushBackString("FORUMGET").pushBackInteger(13).pushBackString("vv2")), 47);

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
        a.checkNonNull("41. forumstat", p.get());

        afl::data::Access ap(p);
        a.checkEqual("51. name",        ap("name").toString(), "theName");
        a.checkEqual("52. parent",      ap("parent").toString(), "theGroup");
        a.checkEqual("53. description", ap("description").toString(), "theDescription");
        a.checkEqual("54. newsgroup",   ap("newsgroup").toString(), "theNewsgroup");
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
        a.checkNonNull("61. forumstat", p.get());

        afl::data::Access ap(p);
        a.checkEqual("71. getArraySize", ap.getArraySize(), 3U);
        a.checkEqual("72. name",      ap[0]("name").toString(), "theName");
        a.checkNull("73. value",      ap[1].getValue());
        a.checkEqual("74. otherName", ap[2]("name").toString(), "otherName");
    }

    // getPermissions/FORUMPERMS
    mock.expectCall("getPermissions(3,read)");
    mock.provideReturnValue<int32_t>(7);
    a.checkEqual("81. forumperms", testee.callInt(Segment().pushBackString("FORUMPERMS").pushBackInteger(3).pushBackString("read")), 7);

    // getSize/FORUMSIZE
    {
        server::interface::TalkForum::Size sz;
        sz.numThreads = 3;
        sz.numStickyThreads = 1;
        sz.numMessages = 33;
        mock.expectCall("getSize(6)");
        mock.provideReturnValue(sz);

        std::auto_ptr<afl::data::Value> p(testee.call(Segment().pushBackString("FORUMSIZE").pushBackInteger(6)));
        a.checkNonNull("91. forumsize", p.get());

        afl::data::Access ap(p);
        a.checkEqual("101. threads",       ap("threads").toInteger(), 3);
        a.checkEqual("102. stickythreads", ap("stickythreads").toInteger(), 1);
        a.checkEqual("103. messages",      ap("messages").toInteger(), 33);
    }

    // getThreads/FORUMLSTHREAD
    mock.expectCall("getThreads(6,all)");
    mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(9));
    a.checkEqual("111. forumlsthread", testee.callInt(Segment().pushBackString("FORUMLSTHREAD").pushBackInteger(6)), 9);

    mock.expectCall("getThreads(6,all,sort(TIME))");
    mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(9));
    a.checkEqual("121. forumlsthread", testee.callInt(Segment().pushBackString("FORUMLSTHREAD").pushBackInteger(6).pushBackString("SORT").pushBackString("time")), 9);

    mock.expectCall("getThreads(6,range(10,20),sort(TIME))");
    mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(9));
    a.checkEqual("131. forumlsthread", testee.callInt(Segment().pushBackString("FORUMLSTHREAD").pushBackInteger(6).pushBackString("SORT").pushBackString("time").pushBackString("LIMIT").pushBackInteger(10).pushBackInteger(20)), 9);

    mock.expectCall("getThreads(6,member(9))");
    mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(1));
    a.checkEqual("141. forumlsthread", testee.callInt(Segment().pushBackString("FORUMLSTHREAD").pushBackInteger(6).pushBackString("CONTAINS").pushBackInteger(9)), 1);

    mock.expectCall("getThreads(6,size)");
    mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(71));
    a.checkEqual("151. forumlsthread", testee.callInt(Segment().pushBackString("FORUMLSTHREAD").pushBackInteger(6).pushBackString("SIZE")), 71);

    // getStickyThreads/FORUMLSSTICKY
    mock.expectCall("getStickyThreads(6,all)");
    mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(9));
    a.checkEqual("161. forumlssticky", testee.callInt(Segment().pushBackString("FORUMLSSTICKY").pushBackInteger(6)), 9);

    mock.expectCall("getStickyThreads(6,range(10,20),sort(TIME))");
    mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(9));
    a.checkEqual("171. forumlssticky", testee.callInt(Segment().pushBackString("FORUMLSSTICKY").pushBackInteger(6).pushBackString("SORT").pushBackString("time").pushBackString("LIMIT").pushBackInteger(10).pushBackInteger(20)), 9);

    // getPosts/FORUMLSPOST
    mock.expectCall("getPosts(6,all)");
    mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(9));
    a.checkEqual("181. forumlspost", testee.callInt(Segment().pushBackString("FORUMLSPOST").pushBackInteger(6)), 9);

    mock.expectCall("getPosts(6,range(10,20),sort(TIME))");
    mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(9));
    a.checkEqual("191. forumlspost", testee.callInt(Segment().pushBackString("FORUMLSPOST").pushBackInteger(6).pushBackString("SORT").pushBackString("time").pushBackString("LIMIT").pushBackInteger(10).pushBackInteger(20)), 9);

    // findForum
    mock.expectCall("findForum(talk)");
    mock.provideReturnValue(45);
    a.checkEqual("201. forumbyname", testee.callInt(Segment().pushBackString("FORUMBYNAME").pushBackString("talk")), 45);

    // Variations
    mock.expectCall("add()");
    mock.provideReturnValue<int32_t>(9);
    a.checkEqual("211. forumadd", testee.callInt(Segment().pushBackString("forumAdd")), 9);

    mock.expectCall("getStickyThreads(6,range(10,20),sort(TIME))");
    mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(9));
    a.checkEqual("221. forumlssticky", testee.callInt(Segment().pushBackString("forumlssticky").pushBackInteger(6).pushBackString("sort").pushBackString("Time").pushBackString("limit").pushBackInteger(10).pushBackInteger(20)), 9);

    mock.checkFinish();
}

/** Test erroneous calls. */
AFL_TEST("server.interface.TalkForumServer:errors", a)
{
    TalkForumMock mock(a);
    server::interface::TalkForumServer testee(mock);

    Segment empty;    // g++-3.4 sees an invocation of a copy constructor if I construct this object in-place.
    AFL_CHECK_THROWS(a("01. bad verb"),       testee.callVoid(Segment().pushBackString("UNKNOWN")), std::exception);
    AFL_CHECK_THROWS(a("02. no verb"),        testee.callVoid(empty), std::exception);
    AFL_CHECK_THROWS(a("03. missing arg"),    testee.callVoid(Segment().pushBackString("FORUMLSSTICKY")), std::exception);
    AFL_CHECK_THROWS(a("04. bad type"),       testee.callVoid(Segment().pushBackString("FORUMLSSTICKY").pushBackString("boom")), std::exception);
    AFL_CHECK_THROWS(a("05. missing option"), testee.callVoid(Segment().pushBackString("FORUMLSSTICKY").pushBackInteger(6).pushBackString("sort")), std::exception);
    AFL_CHECK_THROWS(a("06. bad option"),     testee.callVoid(Segment().pushBackString("FORUMLSSTICKY").pushBackInteger(6).pushBackString("limit").pushBackInteger(10)), std::exception);

    interpreter::Arguments args(empty, 0, 0);
    std::auto_ptr<afl::data::Value> p;
    a.checkEqual("11. bad verb", testee.handleCommand("huhu", args, p), false);
}

/** Test roundtrip behaviour. */
AFL_TEST("server.interface.TalkForumServer:roundtrip", a)
{
    TalkForumMock mock(a);
    server::interface::TalkForumServer level1(mock);
    server::interface::TalkForumClient level2(level1);
    server::interface::TalkForumServer level3(level2);
    server::interface::TalkForumClient level4(level3);

    // add/FORUMADD
    mock.expectCall("add()");
    mock.provideReturnValue<int32_t>(7);
    a.checkEqual("01. add", level4.add(afl::base::Nothing), 7);
    {
        mock.expectCall("add(name,New Forum,description,More info...)");
        mock.provideReturnValue<int32_t>(8);
        const String_t args[] = {"name","New Forum","description","More info..."};
        a.checkEqual("02. add", level4.add(args), 8);
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
        a.checkNull("11. getValue", p.get());
    }

    mock.expectCall("getValue(13,vv2)");
    mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(47));
    a.checkEqual("21. getIntegerValue", level4.getIntegerValue(13, "vv2"), 47);

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
        a.checkEqual("31. name",          out.name, "theName");
        a.checkEqual("32. parentGroup",   out.parentGroup, "theGroup");
        a.checkEqual("33. description",   out.description, "theDescription");
        a.checkEqual("34. newsgroupName", out.newsgroupName, "theNewsgroup");
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

        a.checkEqual  ("41. size",  out.size(), 3U);
        a.checkNonNull("42. value", out[0]);
        a.checkNull   ("43. value", out[1]);
        a.checkNonNull("44. value", out[2]);
        a.checkEqual  ("45. name",  out[0]->name, "theName");
        a.checkEqual  ("46. name",  out[2]->name, "otherName");
    }

    // getPermissions/FORUMPERMS
    {
        const String_t perms[] = {"read","write","delete"};
        mock.expectCall("getPermissions(3,read,write,delete)");
        mock.provideReturnValue<int32_t>(7);
        a.checkEqual("51. getPermissions", level4.getPermissions(3, perms), 7);
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
        a.checkEqual("61. numThreads",       out.numThreads, 3);
        a.checkEqual("62. numStickyThreads", out.numStickyThreads, 1);
        a.checkEqual("63. numMessages",      out.numMessages, 33);
    }

    // getThreads/FORUMLSTHREAD
    {
        mock.expectCall("getThreads(6,all)");
        mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(9));
        std::auto_ptr<afl::data::Value> p(level4.getThreads(6, server::interface::TalkForum::ListParameters()));
        a.checkEqual("71. getThreads", server::toInteger(p.get()), 9);
    }

    {
        mock.expectCall("getThreads(6,all,sort(TIME))");
        mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(9));
        server::interface::TalkForum::ListParameters param;
        param.sortKey = "TIME";
        std::auto_ptr<afl::data::Value> p(level4.getThreads(6, param));
        a.checkEqual("81. getThreads", server::toInteger(p.get()), 9);
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
        a.checkEqual("91. getThreads", server::toInteger(p.get()), 9);
    }

    {
        mock.expectCall("getThreads(6,member(9))");
        mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(9));
        server::interface::TalkForum::ListParameters param;
        param.mode = param.WantMemberCheck;
        param.item = 9;
        std::auto_ptr<afl::data::Value> p(level4.getThreads(6, param));
        a.checkEqual("101. getThreads", server::toInteger(p.get()), 9);
    }

    {
        mock.expectCall("getThreads(6,size)");
        mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(9));
        server::interface::TalkForum::ListParameters param;
        param.mode = param.WantSize;
        std::auto_ptr<afl::data::Value> p(level4.getThreads(6, param));
        a.checkEqual("111. getThreads", server::toInteger(p.get()), 9);
    }

    // getStickyThreads/FORUMLSSTICKY
    {
        mock.expectCall("getStickyThreads(6,all)");
        mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(9));
        std::auto_ptr<afl::data::Value> p(level4.getStickyThreads(6, server::interface::TalkForum::ListParameters()));
        a.checkEqual("121. getStickyThreads", server::toInteger(p.get()), 9);
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
        a.checkEqual("131. getStickyThreads", server::toInteger(p.get()), 9);
    }

    // getPosts/FORUMLSPOST
    {
        mock.expectCall("getPosts(6,all)");
        mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(9));
        std::auto_ptr<afl::data::Value> p(level4.getPosts(6, server::interface::TalkForum::ListParameters()));
        a.checkEqual("141. getPosts", server::toInteger(p.get()), 9);
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
        a.checkEqual("151. getPosts", server::toInteger(p.get()), 9);
    }

    // findForum
    mock.expectCall("findForum(bugs)");
    mock.provideReturnValue(23);
    a.checkEqual("161. findForum", level4.findForum("bugs"), 23);

    mock.checkFinish();
}
