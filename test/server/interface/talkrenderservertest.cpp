/**
  *  \file test/server/interface/talkrenderservertest.cpp
  *  \brief Test for server::interface::TalkRenderServer
  */

#include "server/interface/talkrenderserver.hpp"

#include "afl/data/access.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"
#include "server/interface/talkrender.hpp"
#include "server/interface/talkrenderclient.hpp"
#include "server/types.hpp"
#include <memory>
#include <stdexcept>

using server::interface::TalkRender;

namespace {
    class TalkRenderMock : public TalkRender, public afl::test::CallReceiver {
     public:
        TalkRenderMock(afl::test::Assert a)
            : CallReceiver(a)
            { }
        virtual void setOptions(const Options& opts)
            {
                checkCall(afl::string::Format("setOptions(%s,%s)", opts.baseUrl.orElse("none"), opts.format.orElse("none")));
            }
        virtual String_t render(const String_t& text, const Options& opts)
            {
                checkCall(afl::string::Format("render(%s,%s,%s)", text, opts.baseUrl.orElse("none"), opts.format.orElse("none")));
                return consumeReturnValue<String_t>();
            }
        virtual void check(const String_t& text, std::vector<TalkRender::Warning>& out)
            {
                checkCall(afl::string::Format("check(%s)", text));
                int n = consumeReturnValue<int>();
                for (int i = 0; i < n; ++i) {
                    out.push_back(consumeReturnValue<TalkRender::Warning>());
                }
            }
    };

    TalkRender::Warning makeWarning(String_t type, String_t token, String_t extra, int pos)
    {
        TalkRender::Warning w;
        w.type  = type;
        w.token = token;
        w.extra = extra;
        w.pos   = pos;
        return w;
    }
}

AFL_TEST("server.interface.TalkRenderServer:commands", a)
{
    using afl::data::Segment;
    TalkRenderMock mock(a);
    server::interface::TalkRenderServer testee(mock);

    // RENDEROPTION in a bajillion forms
    mock.expectCall("setOptions(none,none)");
    testee.callVoid(Segment().pushBackString("RENDEROPTION"));

    mock.expectCall("setOptions(/url/,none)");
    testee.callVoid(Segment().pushBackString("RENDEROPTION").pushBackString("BASEURL").pushBackString("/url/"));

    mock.expectCall("setOptions(none,text)");
    testee.callVoid(Segment().pushBackString("RENDEROPTION").pushBackString("FORMAT").pushBackString("text"));

    mock.expectCall("setOptions(/url/,text)");
    testee.callVoid(Segment().pushBackString("RENDEROPTION").pushBackString("FORMAT").pushBackString("text").pushBackString("BASEURL").pushBackString("/url/"));

    mock.expectCall("setOptions(/url/,text)");
    testee.callVoid(Segment().pushBackString("RENDEROPTION").pushBackString("BASEURL").pushBackString("/url/").pushBackString("FORMAT").pushBackString("text"));

    mock.expectCall("setOptions(/URL/,none)");
    testee.callVoid(Segment().pushBackString("renderoption").pushBackString("baseurl").pushBackString("/URL/"));
    mock.checkFinish();

    // RENDER
    {
        mock.expectCall("render(text-to-render,none,none)");
        mock.provideReturnValue(String_t("result"));
        std::auto_ptr<afl::data::Value> result(testee.call(Segment().pushBackString("RENDER").pushBackString("text-to-render")));
        a.checkEqual("01. render", server::toString(result.get()), "result");
    }
    {
        mock.expectCall("render(text-to-render,/url/,none)");
        mock.provideReturnValue(String_t("result"));
        std::auto_ptr<afl::data::Value> result(testee.call(Segment().pushBackString("RENDER").pushBackString("text-to-render").pushBackString("baseurl").pushBackString("/url/")));
        a.checkEqual("02. render", server::toString(result.get()), "result");
    }
    mock.checkFinish();

    // RENDERCHECK
    {
        mock.expectCall("check(text-to-check)");
        mock.provideReturnValue(3);
        mock.provideReturnValue(makeWarning("one",   "t1", "x1", 1));
        mock.provideReturnValue(makeWarning("two",   "t2", "x2", 22));
        mock.provideReturnValue(makeWarning("three", "t3", "x3", 333));

        std::auto_ptr<afl::data::Value> result(testee.call(Segment().pushBackString("RENDERCHECK").pushBackString("text-to-check")));
        afl::data::Access aa(result.get());
        a.checkEqual("11. size", aa.getArraySize(), 3U);
        a.checkEqual("12a. type",  aa[0]("type").toString(),  "one");
        a.checkEqual("12b. token", aa[0]("token").toString(), "t1");
        a.checkEqual("12c. extra", aa[0]("extra").toString(), "x1");
        a.checkEqual("12d. pos",   aa[0]("pos").toInteger(),  1);
        a.checkEqual("13a. type",  aa[1]("type").toString(),  "two");
        a.checkEqual("13b. token", aa[1]("token").toString(), "t2");
        a.checkEqual("13c. extra", aa[1]("extra").toString(), "x2");
        a.checkEqual("13d. pos",   aa[1]("pos").toInteger(),  22);
        a.checkEqual("14a. type",  aa[2]("type").toString(),  "three");
        a.checkEqual("14b. token", aa[2]("token").toString(), "t3");
        a.checkEqual("14c. extra", aa[2]("extra").toString(), "x3");
        a.checkEqual("14d. pos",   aa[2]("pos").toInteger(),  333);
    }

    // Errors
    AFL_CHECK_THROWS(a("111. bad verb"),       testee.callVoid(Segment().pushBackString("RENDERFOO").pushBackString("foo")), std::exception);
    AFL_CHECK_THROWS(a("112. missing option"), testee.callVoid(Segment().pushBackString("RENDEROPTION").pushBackString("BASEURL")), std::exception);
    AFL_CHECK_THROWS(a("113. missing option"), testee.callVoid(Segment().pushBackString("RENDEROPTION").pushBackString("FORMAT")), std::exception);
    AFL_CHECK_THROWS(a("114. bad option"),     testee.callVoid(Segment().pushBackString("RENDEROPTION").pushBackString("FOO").pushBackString("val")), std::exception);
    AFL_CHECK_THROWS(a("115. missing arg"),    testee.callVoid(Segment().pushBackString("RENDER")), std::exception);
    AFL_CHECK_THROWS(a("116. bad arg"),        testee.callVoid(Segment().pushBackString("RENDER").pushBackString("foo").pushBackString("BASEURL")), std::exception);

    Segment empty;
    interpreter::Arguments args(empty, 0, 0);
    std::auto_ptr<afl::data::Value> p;
    a.checkEqual("121. bad verb", testee.handleCommand("huhu", args, p), false);
}

/** Test roundtrip behaviour. */
AFL_TEST("server.interface.TalkRenderServer:roundtrip", a)
{
    TalkRenderMock mock(a);
    server::interface::TalkRenderServer level1(mock);
    server::interface::TalkRenderClient level2(level1);
    server::interface::TalkRenderServer level3(level2);
    server::interface::TalkRenderClient level4(level3);

    // No options
    mock.expectCall("setOptions(none,none)");
    level4.setOptions(TalkRender::Options());
    mock.checkFinish();

    mock.expectCall("render(text,none,none)");
    mock.provideReturnValue(String_t("result"));
    a.checkEqual("01. render", level4.render("text", TalkRender::Options()), "result");
    mock.checkFinish();

    // Full options
    {
        TalkRender::Options opts;
        opts.baseUrl = "/url";
        opts.format = "fmt";
        mock.expectCall("setOptions(/url,fmt)");
        level4.setOptions(opts);
        mock.checkFinish();

        mock.expectCall("render(what,/url,fmt)");
        mock.provideReturnValue(String_t("why"));
        a.checkEqual("11. render", level4.render("what", opts), "why");
        mock.checkFinish();
    }

    // Check
    {
        mock.expectCall("check(warnable-text)");
        mock.provideReturnValue(1);
        mock.provideReturnValue(makeWarning("w", "t", "x", 42));

        std::vector<TalkRender::Warning> ws;
        level4.check("warnable-text", ws);
        mock.checkFinish();

        a.checkEqual("21. size",  ws.size(),   1U);
        a.checkEqual("22. type",  ws[0].type,  "w");
        a.checkEqual("23. token", ws[0].token, "t");
        a.checkEqual("24. extra", ws[0].extra, "x");
        a.checkEqual("25. pos",   ws[0].pos,   42);
    }
}
