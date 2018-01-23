/**
  *  \file u/t_server_interface_talkpostserver.cpp
  *  \brief Test for server::interface::TalkPostServer
  */

#include "server/interface/talkpostserver.hpp"

#include <memory>
#include "t_server_interface.hpp"
#include "afl/data/access.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "server/interface/talkpostclient.hpp"
#include "server/types.hpp"

using afl::string::Format;
using afl::data::Segment;

namespace {
    class TalkPostMock : public server::interface::TalkPost, public afl::test::CallReceiver {
     public:
        TalkPostMock(afl::test::Assert a)
            : CallReceiver(a)
            { }
        virtual int32_t create(int32_t forumId, String_t subject, String_t text, const CreateOptions& options)
            {
                checkCall(Format("create(%d,%s,%s,%s,%s,%s)") << forumId << subject << text << options.userId.orElse("no-user") << options.readPermissions.orElse("no-read") << options.answerPermissions.orElse("no-answer"));
                return consumeReturnValue<int32_t>();
            }
        virtual int32_t reply(int32_t parentPostId, String_t subject, String_t text, const ReplyOptions& options)
            {
                checkCall(Format("reply(%d,%s,%s,%s)", parentPostId, subject, text, options.userId.orElse("no-user")));
                return consumeReturnValue<int32_t>();
            }
        virtual void edit(int32_t postId, String_t subject, String_t text)
            {
                checkCall(Format("edit(%d,%s,%s)", postId, subject, text));
            }
        virtual String_t render(int32_t postId, const server::interface::TalkRender::Options& options)
            {
                checkCall(Format("render(%d,%s,%s)", postId, options.baseUrl.orElse("no-url"), options.format.orElse("no-format")));
                return consumeReturnValue<String_t>();
            }
        virtual void render(afl::base::Memory<const int32_t> postIds, afl::data::StringList_t& result)
            {
                String_t tmp = "render(";
                while (const int32_t* p = postIds.eat()) {
                    tmp += Format("%d", *p);
                    if (!postIds.empty()) {
                        tmp += ",";
                    }
                    result.push_back(Format("result-%d", *p));
                }
                tmp += ")";
                checkCall(tmp);
            }
        virtual Info getInfo(int32_t postId)
            {
                checkCall(Format("getInfo(%d)", postId));
                return consumeReturnValue<Info>();
            }
        virtual void getInfo(afl::base::Memory<const int32_t> postIds, afl::container::PtrVector<Info>& result)
            {
                String_t tmp = "getInfo(";
                while (const int32_t* p = postIds.eat()) {
                    tmp += Format("%d", *p);
                    if (!postIds.empty()) {
                        tmp += ",";
                    }
                    result.pushBackNew(consumeReturnValue<Info*>());
                }
                tmp += ")";
                checkCall(tmp);
            }
        virtual String_t getHeaderField(int32_t postId, String_t fieldName)
            {
                checkCall(Format("getHeaderField(%d,%s)", postId, fieldName));
                return consumeReturnValue<String_t>();
            }
        virtual bool remove(int32_t postId)
            {
                checkCall(Format("remove(%d)", postId));
                return consumeReturnValue<bool>();
            }
        virtual void getNewest(int count, afl::data::IntegerList_t& postIds)
            {
                checkCall(Format("getNewest(%d)", count));
                for (int i = 0; i < count; ++i) {
                    postIds.push_back(i+1);
                }
            }
    };
}

/** Test it. */
void
TestServerInterfaceTalkPostServer::testIt()
{
    TalkPostMock mock("testIt");
    server::interface::TalkPostServer testee(mock);

    // POSTNEW
    mock.expectCall("create(5,subj,text,no-user,no-read,no-answer)");
    mock.provideReturnValue<int32_t>(99);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("POSTNEW").pushBackInteger(5).pushBackString("subj").pushBackString("text")), 99);

    mock.expectCall("create(15,SUBJ,TEXT,1005,u:1004,all)");
    mock.provideReturnValue<int32_t>(77);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("POSTNEW").pushBackInteger(15).pushBackString("SUBJ").pushBackString("TEXT")
                                    .pushBackString("ANSWERPERM").pushBackString("all")
                                    .pushBackString("READPERM").pushBackString("u:1004")
                                    .pushBackString("USER").pushBackString("1005")), 77);

    // POSTREPLY
    mock.expectCall("reply(99,replysubj,replytext,no-user)");
    mock.provideReturnValue<int32_t>(88);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("POSTREPLY").pushBackInteger(99).pushBackString("replysubj").pushBackString("replytext")), 88);

    mock.expectCall("reply(99,replysubj,replytext,1007)");
    mock.provideReturnValue<int32_t>(66);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("POSTREPLY").pushBackInteger(99).pushBackString("replysubj").pushBackString("replytext")
                                    .pushBackString("user").pushBackString("1007")), 66);

    // POSTEDIT
    mock.expectCall("edit(32,newsubj,newtext)");
    testee.callVoid(Segment().pushBackString("POSTEDIT").pushBackInteger(32).pushBackString("newsubj").pushBackString("newtext"));

    // POSTEDIT, case variation
    mock.expectCall("edit(32,newsubj,newtext)");
    testee.callVoid(Segment().pushBackString("postedit").pushBackInteger(32).pushBackString("newsubj").pushBackString("newtext"));

    // POSTRENDER
    mock.expectCall("render(1,no-url,no-format)");
    mock.provideReturnValue<String_t>("one");
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("POSTRENDER").pushBackInteger(1)), "one");

    mock.expectCall("render(1,/url,html)");
    mock.provideReturnValue<String_t>("<one>");
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("POSTRENDER").pushBackInteger(1)
                                       .pushBackString("FORMAT").pushBackString("html")
                                       .pushBackString("baseurl").pushBackString("/url")), "<one>");

    // POSTMRENDER
    mock.expectCall("render(3,1,4,1,5)");
    testee.callVoid(Segment().pushBackString("POSTMRENDER").pushBackInteger(3).pushBackInteger(1).pushBackInteger(4).pushBackInteger(1).pushBackInteger(5));

    // POSTSTAT
    server::interface::TalkPost::Info info;
    info.subject = "subj";
    info.author = "author";
    info.postTime = 9;
    info.editTime = 10;
    {

        mock.expectCall("getInfo(12)");
        mock.provideReturnValue(info);

        std::auto_ptr<afl::data::Value> p(testee.call(Segment().pushBackString("POSTSTAT").pushBackInteger(12)));
        afl::data::Access a(p);
        TS_ASSERT_EQUALS(a("time").toInteger(), 9);
        TS_ASSERT_EQUALS(a("edittime").toInteger(), 10);
        TS_ASSERT_EQUALS(a("subject").toString(), "subj");
        TS_ASSERT_EQUALS(a("author").toString(), "author");
    }

    // POSTMSTAT
    mock.expectCall("getInfo(4,2)");
    mock.provideReturnValue(new server::interface::TalkPost::Info(info));
    mock.provideReturnValue(new server::interface::TalkPost::Info(info));
    testee.callVoid(Segment().pushBackString("POSTMSTAT").pushBackInteger(4).pushBackInteger(2));

    // POSTGET
    mock.expectCall("getHeaderField(12,foo)");
    mock.provideReturnValue<String_t>("bar");
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("POSTGET").pushBackInteger(12).pushBackString("foo")), "bar");

    // POSTRM
    mock.expectCall("remove(8)");
    mock.provideReturnValue<bool>(true);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("POSTRM").pushBackInteger(8)), 1);

    // POSTLSNEW
    mock.expectCall("getNewest(9)");
    testee.callVoid(Segment().pushBackString("POSTLSNEW").pushBackInteger(9));

    mock.checkFinish();
}

/** Test some errors. */
void
TestServerInterfaceTalkPostServer::testErrors()
{
    TalkPostMock mock("testErrors");
    server::interface::TalkPostServer testee(mock);

    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("huhu")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("poststat")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("POSTSTAT")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("POSTRM").pushBackString("NOT-A-NUMBER")), std::exception);
    TS_ASSERT_THROWS(testee.callInt(Segment().pushBackString("POSTNEW").pushBackInteger(15).pushBackString("SUBJ").pushBackString("TEXT").pushBackString("ANSWERPERM")), std::exception);
    TS_ASSERT_THROWS(testee.callInt(Segment().pushBackString("POSTNEW").pushBackInteger(15).pushBackString("SUBJ").pushBackString("TEXT").pushBackString("whatever")), std::exception);

    Segment empty;
    interpreter::Arguments args(empty, 0, 0);
    std::auto_ptr<afl::data::Value> p;
    TS_ASSERT_EQUALS(testee.handleCommand("huhu", args, p), false);
}

/** Test roundtrip. */
void
TestServerInterfaceTalkPostServer::testRoundtrip()
{
    TalkPostMock mock("testRoundtrip");
    server::interface::TalkPostServer level1(mock);
    server::interface::TalkPostClient level2(level1);
    server::interface::TalkPostServer level3(level2);
    server::interface::TalkPostClient level4(level3);

    // create
    mock.expectCall("create(9,s,t,no-user,no-read,no-answer)");
    mock.provideReturnValue<int32_t>(33);
    TS_ASSERT_EQUALS(level4.create(9, "s", "t", server::interface::TalkPost::CreateOptions()), 33);

    {
        server::interface::TalkPost::CreateOptions opts;
        opts.userId = "u";
        opts.readPermissions = "r";
        opts.answerPermissions = "a";
        mock.expectCall("create(10,s,t,u,r,a)");
        mock.provideReturnValue<int32_t>(34);
        TS_ASSERT_EQUALS(level4.create(10, "s", "t", opts), 34);
    }

    // reply
    mock.expectCall("reply(10,ss,tt,no-user)");
    mock.provideReturnValue<int32_t>(77);
    TS_ASSERT_EQUALS(level4.reply(10, "ss", "tt", server::interface::TalkPost::ReplyOptions()), 77);

    {
        server::interface::TalkPost::ReplyOptions opts;
        opts.userId = "uu";
        mock.expectCall("reply(11,ss,tt,uu)");
        mock.provideReturnValue<int32_t>(78);
        TS_ASSERT_EQUALS(level4.reply(11, "ss", "tt", opts), 78);
    }

    // edit
    mock.expectCall("edit(12,ns,nt)");
    level4.edit(12, "ns", "nt");

    // render
    mock.expectCall("render(13,no-url,no-format)");
    mock.provideReturnValue<String_t>("result");
    TS_ASSERT_EQUALS(level4.render(13, server::interface::TalkRender::Options()), "result");

    {
        server::interface::TalkRender::Options opts;
        opts.baseUrl = "/url";
        mock.expectCall("render(14,/url,no-format)");
        mock.provideReturnValue<String_t>("result2");
        TS_ASSERT_EQUALS(level4.render(14, opts), "result2");
    }

    // render multiple
    {
        const int32_t ids[] = { 32, 16, 8 };
        afl::data::StringList_t result;
        mock.expectCall("render(32,16,8)");
        level4.render(ids, result);
        TS_ASSERT_EQUALS(result.size(), 3U);
        TS_ASSERT_EQUALS(result[0], "result-32");
        TS_ASSERT_EQUALS(result[1], "result-16");
        TS_ASSERT_EQUALS(result[2], "result-8");
    }

    // getInfo
    {
        server::interface::TalkPost::Info in;
        in.threadId = 33;
        in.parentPostId = 44;
        in.postTime = 55;
        in.editTime = 66;
        in.author = "a";
        in.subject = "s";
        in.rfcMessageId = "r@c";
        mock.expectCall("getInfo(88)");
        mock.provideReturnValue(in);

        server::interface::TalkPost::Info out = level4.getInfo(88);
        TS_ASSERT_EQUALS(out.threadId, 33);
        TS_ASSERT_EQUALS(out.parentPostId, 44);
        TS_ASSERT_EQUALS(out.postTime, 55);
        TS_ASSERT_EQUALS(out.editTime, 66);
        TS_ASSERT_EQUALS(out.author, "a");
        TS_ASSERT_EQUALS(out.subject, "s");
        TS_ASSERT_EQUALS(out.rfcMessageId, "r@c");
    }

    // getInfo multi
    {
        typedef server::interface::TalkPost::Info Info_t;
        Info_t* p = new Info_t();
        p->threadId = 86;
        p->parentPostId = 87;
        p->postTime = 88;
        p->editTime = 89;
        mock.expectCall("getInfo(44,45)");
        mock.provideReturnValue<Info_t*>(0);
        mock.provideReturnValue<Info_t*>(p);

        const int32_t ids[] = { 44, 45 };
        afl::container::PtrVector<Info_t> result;
        level4.getInfo(ids, result);
        TS_ASSERT_EQUALS(result.size(), 2U);
        TS_ASSERT(result[0] == 0);
        TS_ASSERT(result[1] != 0);
        TS_ASSERT_EQUALS(result[1]->threadId, 86);
        TS_ASSERT_EQUALS(result[1]->parentPostId, 87);
        TS_ASSERT_EQUALS(result[1]->postTime, 88);
        TS_ASSERT_EQUALS(result[1]->editTime, 89);
    }

    // getHeaderField
    mock.expectCall("getHeaderField(55,field)");
    mock.provideReturnValue<String_t>("value");
    TS_ASSERT_EQUALS(level4.getHeaderField(55, "field"), "value");

    // remove
    mock.expectCall("remove(56)");
    mock.provideReturnValue<bool>(true);
    TS_ASSERT(level4.remove(56));

    mock.expectCall("remove(57)");
    mock.provideReturnValue<bool>(false);
    TS_ASSERT(!level4.remove(57));

    // getNewest
    {
        mock.expectCall("getNewest(3)");
        afl::data::IntegerList_t result;
        level4.getNewest(3, result);
        TS_ASSERT_EQUALS(result.size(), 3U);
        TS_ASSERT_EQUALS(result[0], 1);
        TS_ASSERT_EQUALS(result[1], 2);
        TS_ASSERT_EQUALS(result[2], 3);
    }

    mock.checkFinish();
}

