/**
  *  \file u/t_server_interface_talkpostclient.cpp
  *  \brief Test for server::interface::TalkPostClient
  */

#include "server/interface/talkpostclient.hpp"

#include "t_server_interface.hpp"
#include "u/helper/commandhandlermock.hpp"
#include "server/types.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/segment.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"

using server::makeIntegerValue;
using server::makeStringValue;

using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Vector;
using afl::data::VectorValue;
using afl::data::Segment;

/** Test it. */
void
TestServerInterfaceTalkPostClient::testIt()
{
    CommandHandlerMock mock;
    server::interface::TalkPostClient testee(mock);

    // POSTNEW
    mock.expectCall("POSTNEW|3|subject|text");
    mock.provideReturnValue(makeIntegerValue(99));
    TS_ASSERT_EQUALS(testee.create(3, "subject", "text", server::interface::TalkPost::CreateOptions()), 99);

    {
        server::interface::TalkPost::CreateOptions opts;
        opts.userId = "1001";
        opts.readPermissions = "g:9";
        opts.answerPermissions = "-all";
        mock.expectCall("POSTNEW|4|title|body|USER|1001|READPERM|g:9|ANSWERPERM|-all");
        mock.provideReturnValue(makeIntegerValue(100));
        TS_ASSERT_EQUALS(testee.create(4, "title", "body", opts), 100);
    }

    // POSTREPLY
    mock.expectCall("POSTREPLY|100|reply-title|reply-body");
    mock.provideReturnValue(makeIntegerValue(105));
    TS_ASSERT_EQUALS(testee.reply(100, "reply-title", "reply-body", server::interface::TalkPost::ReplyOptions()), 105);

    {
        server::interface::TalkPost::ReplyOptions opts;
        opts.userId = "1002";
        mock.expectCall("POSTREPLY|100|reply1|reply2|USER|1002");
        mock.provideReturnValue(makeIntegerValue(107));
        TS_ASSERT_EQUALS(testee.reply(100, "reply1", "reply2", opts), 107);
    }

    // POSTEDIT
    mock.expectCall("POSTEDIT|100|new-title|new-body");
    mock.provideReturnValue(0);
    testee.edit(100, "new-title", "new-body");

    // POSTRENDER
    mock.expectCall("POSTRENDER|3");
    mock.provideReturnValue(makeStringValue("content"));
    TS_ASSERT_EQUALS(testee.render(3, server::interface::TalkRender::Options()), "content");

    {
        server::interface::TalkRender::Options opts;
        opts.baseUrl = "/url";
        opts.format = "html";
        mock.expectCall("POSTRENDER|4|BASEURL|/url|FORMAT|html");
        mock.provideReturnValue(makeStringValue("<html>content</html>"));
        TS_ASSERT_EQUALS(testee.render(4, opts), "<html>content</html>");
    }

    // POSTMRENDER
    {
        mock.expectCall("POSTMRENDER|3|4|5");
        mock.provideReturnValue(new VectorValue(Vector::create(Segment().pushBackString("post3").pushBack(0).pushBackString("post5"))));
        static const int32_t postNrs[] = {3,4,5};
        afl::data::StringList_t result;
        testee.render(postNrs, result);
        TS_ASSERT_EQUALS(result.size(), 3U);
        TS_ASSERT_EQUALS(result[0], "post3");
        TS_ASSERT_EQUALS(result[1], "");
        TS_ASSERT_EQUALS(result[2], "post5");
    }

    // POSTSTAT
    Hash::Ref_t providedResult = Hash::create();
    providedResult->setNew("thread", makeIntegerValue(9));
    providedResult->setNew("parent", makeIntegerValue(2));
    providedResult->setNew("time", makeIntegerValue(200033));
    // omitting edittime
    providedResult->setNew("author", makeStringValue("1002"));
    providedResult->setNew("subject", makeStringValue("title"));
    providedResult->setNew("msgid", makeStringValue("id@host"));
    {
        mock.expectCall("POSTSTAT|3");
        mock.provideReturnValue(new HashValue(providedResult));

        server::interface::TalkPost::Info result = testee.getInfo(3);
        TS_ASSERT_EQUALS(result.threadId, 9);
        TS_ASSERT_EQUALS(result.parentPostId, 2);
        TS_ASSERT_EQUALS(result.postTime, 200033);
        TS_ASSERT_EQUALS(result.editTime, 0);
        TS_ASSERT_EQUALS(result.author, "1002");
        TS_ASSERT_EQUALS(result.subject, "title");
        TS_ASSERT_EQUALS(result.rfcMessageId, "id@host");
    }

    // POSTMSTAT
    {
        mock.expectCall("POSTMSTAT|7|8|9");
        mock.provideReturnValue(new VectorValue(Vector::create(Segment()
                                                               .pushBackNew(new HashValue(providedResult))
                                                               .pushBackNew(0)
                                                               .pushBackNew(new HashValue(providedResult)))));

        static const int32_t postNrs[] = {7,8,9};
        afl::container::PtrVector<server::interface::TalkPost::Info> results;
        testee.getInfo(postNrs, results);
        TS_ASSERT_EQUALS(results.size(), 3U);
        TS_ASSERT(results[0] != 0);
        TS_ASSERT(results[1] == 0);
        TS_ASSERT(results[2] != 0);
        TS_ASSERT_EQUALS(results[0]->author, "1002");
        TS_ASSERT_EQUALS(results[2]->subject, "title");
    }

    // POSTGET
    mock.expectCall("POSTGET|42|edittime");
    mock.provideReturnValue(makeStringValue("934"));
    TS_ASSERT_EQUALS(testee.getHeaderField(42, "edittime"), "934");

    // POSTRENDER
    mock.expectCall("POSTRM|43");
    mock.provideReturnValue(makeIntegerValue(0));
    TS_ASSERT(!testee.remove(43));

    mock.expectCall("POSTRM|44");
    mock.provideReturnValue(makeIntegerValue(1));
    TS_ASSERT(testee.remove(44));

    // POSTLSNEW
    {
        mock.expectCall("POSTLSNEW|5");
        mock.provideReturnValue(new VectorValue(Vector::create(Segment().pushBackInteger(30).pushBackInteger(34).pushBackInteger(35).pushBackInteger(36))));
        afl::data::IntegerList_t result;
        testee.getNewest(5, result);
        TS_ASSERT_EQUALS(result.size(), 4U);
        TS_ASSERT_EQUALS(result[0], 30);
        TS_ASSERT_EQUALS(result[1], 34);
        TS_ASSERT_EQUALS(result[2], 35);
        TS_ASSERT_EQUALS(result[3], 36);
    }

    mock.checkFinish();
}
