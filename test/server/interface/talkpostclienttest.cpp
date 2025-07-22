/**
  *  \file test/server/interface/talkpostclienttest.cpp
  *  \brief Test for server::interface::TalkPostClient
  */

#include "server/interface/talkpostclient.hpp"

#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/segment.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/test/commandhandler.hpp"
#include "afl/test/testrunner.hpp"
#include "server/types.hpp"

using server::makeIntegerValue;
using server::makeStringValue;

using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Segment;
using afl::data::Vector;
using afl::data::VectorValue;

/** Test it. */
AFL_TEST("server.interface.TalkPostClient", a)
{
    afl::test::CommandHandler mock(a);
    server::interface::TalkPostClient testee(mock);

    // POSTNEW
    mock.expectCall("POSTNEW, 3, subject, text");
    mock.provideNewResult(makeIntegerValue(99));
    a.checkEqual("01. create", testee.create(3, "subject", "text", server::interface::TalkPost::CreateOptions()), 99);

    {
        server::interface::TalkPost::CreateOptions opts;
        opts.userId = "1001";
        opts.readPermissions = "g:9";
        opts.answerPermissions = "-all";
        mock.expectCall("POSTNEW, 4, title, body, USER, 1001, READPERM, g:9, ANSWERPERM, -all");
        mock.provideNewResult(makeIntegerValue(100));
        a.checkEqual("11. create", testee.create(4, "title", "body", opts), 100);
    }

    {
        server::interface::TalkPost::CreateOptions opts;
        opts.userId = "1001";
        opts.readPermissions = "g:9";
        opts.alsoPostTo.push_back(3);
        opts.alsoPostTo.push_back(8);
        opts.alsoPostTo.push_back(13);
        mock.expectCall("POSTNEW, 4, title, body, USER, 1001, READPERM, g:9, ALSO, 3, ALSO, 8, ALSO, 13");
        mock.provideNewResult(makeIntegerValue(100));
        a.checkEqual("12. create", testee.create(4, "title", "body", opts), 100);
    }

    // POSTREPLY
    mock.expectCall("POSTREPLY, 100, reply-title, reply-body");
    mock.provideNewResult(makeIntegerValue(105));
    a.checkEqual("21. reply", testee.reply(100, "reply-title", "reply-body", server::interface::TalkPost::ReplyOptions()), 105);

    {
        server::interface::TalkPost::ReplyOptions opts;
        opts.userId = "1002";
        mock.expectCall("POSTREPLY, 100, reply1, reply2, USER, 1002");
        mock.provideNewResult(makeIntegerValue(107));
        a.checkEqual("31. reply", testee.reply(100, "reply1", "reply2", opts), 107);
    }

    // POSTEDIT
    mock.expectCall("POSTEDIT, 100, new-title, new-body");
    mock.provideNewResult(0);
    testee.edit(100, "new-title", "new-body");

    // POSTRENDER
    mock.expectCall("POSTRENDER, 3");
    mock.provideNewResult(makeStringValue("content"));
    a.checkEqual("41. render", testee.render(3, server::interface::TalkRender::Options()), "content");

    {
        server::interface::TalkRender::Options opts;
        opts.baseUrl = "/url";
        opts.format = "html";
        mock.expectCall("POSTRENDER, 4, BASEURL, /url, FORMAT, html");
        mock.provideNewResult(makeStringValue("<html>content</html>"));
        a.checkEqual("51. render", testee.render(4, opts), "<html>content</html>");
    }

    // POSTMRENDER
    {
        mock.expectCall("POSTMRENDER, 3, 4, 5");
        mock.provideNewResult(new VectorValue(Vector::create(Segment().pushBackString("post3").pushBack(0).pushBackString("post5"))));
        static const int32_t postNrs[] = {3,4,5};
        afl::data::StringList_t result;
        testee.render(postNrs, result);
        a.checkEqual("61. size", result.size(), 3U);
        a.checkEqual("62. result", result[0], "post3");
        a.checkEqual("63. result", result[1], "");
        a.checkEqual("64. result", result[2], "post5");
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
        mock.expectCall("POSTSTAT, 3");
        mock.provideNewResult(new HashValue(providedResult));

        server::interface::TalkPost::Info result = testee.getInfo(3);
        a.checkEqual("71. threadId",     result.threadId, 9);
        a.checkEqual("72. parentPostId", result.parentPostId, 2);
        a.checkEqual("73. postTime",     result.postTime, 200033);
        a.checkEqual("74. editTime",     result.editTime, 0);
        a.checkEqual("75. author",       result.author, "1002");
        a.checkEqual("76. subject",      result.subject, "title");
        a.checkEqual("77. rfcMessageId", result.rfcMessageId, "id@host");
    }

    // POSTMSTAT
    {
        mock.expectCall("POSTMSTAT, 7, 8, 9");
        mock.provideNewResult(new VectorValue(Vector::create(Segment()
                                                               .pushBackNew(new HashValue(providedResult))
                                                               .pushBackNew(0)
                                                               .pushBackNew(new HashValue(providedResult)))));

        static const int32_t postNrs[] = {7,8,9};
        afl::container::PtrVector<server::interface::TalkPost::Info> results;
        testee.getInfo(postNrs, results);
        a.checkEqual  ("81. size",    results.size(), 3U);
        a.checkNonNull("82. result",  results[0]);
        a.checkNull   ("83. result",  results[1]);
        a.checkNonNull("84. result",  results[2]);
        a.checkEqual  ("85. author",  results[0]->author, "1002");
        a.checkEqual  ("86. subject", results[2]->subject, "title");
    }

    // POSTGET
    mock.expectCall("POSTGET, 42, edittime");
    mock.provideNewResult(makeStringValue("934"));
    a.checkEqual("91. getHeaderField", testee.getHeaderField(42, "edittime"), "934");

    // POSTRENDER
    mock.expectCall("POSTRM, 43");
    mock.provideNewResult(makeIntegerValue(0));
    a.check("101. remove", !testee.remove(43));

    mock.expectCall("POSTRM, 44");
    mock.provideNewResult(makeIntegerValue(1));
    a.check("111. remove", testee.remove(44));

    // POSTLSNEW
    {
        mock.expectCall("POSTLSNEW, 5");
        mock.provideNewResult(new VectorValue(Vector::create(Segment().pushBackInteger(30).pushBackInteger(34).pushBackInteger(35).pushBackInteger(36))));
        afl::data::IntegerList_t result;
        testee.getNewest(5, result);
        a.checkEqual("121. size", result.size(), 4U);
        a.checkEqual("122. result", result[0], 30);
        a.checkEqual("123. result", result[1], 34);
        a.checkEqual("124. result", result[2], 35);
        a.checkEqual("125. result", result[3], 36);
    }

    mock.checkFinish();
}
