/**
  *  \file u/t_server_interface_talkrenderserver.cpp
  *  \brief Test for server::interface::TalkRenderServer
  */

#include "server/interface/talkrenderserver.hpp"

#include <memory>
#include <stdexcept>
#include "t_server_interface.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "server/interface/talkrender.hpp"
#include "server/interface/talkrenderclient.hpp"
#include "server/types.hpp"

namespace {
    class TalkRenderMock : public server::interface::TalkRender, public afl::test::CallReceiver {
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
    };
}

void
TestServerInterfaceTalkRenderServer::testServer()
{
    using afl::data::Segment;
    TalkRenderMock mock("testServer");
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
        TS_ASSERT_EQUALS(server::toString(result.get()), "result");
    }
    {
        mock.expectCall("render(text-to-render,/url/,none)");
        mock.provideReturnValue(String_t("result"));
        std::auto_ptr<afl::data::Value> result(testee.call(Segment().pushBackString("RENDER").pushBackString("text-to-render").pushBackString("baseurl").pushBackString("/url/")));
        TS_ASSERT_EQUALS(server::toString(result.get()), "result");
    }
    mock.checkFinish();

    // Errors
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("RENDERFOO").pushBackString("foo")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("RENDEROPTION").pushBackString("BASEURL")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("RENDEROPTION").pushBackString("FORMAT")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("RENDEROPTION").pushBackString("FOO").pushBackString("val")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("RENDER")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("RENDER").pushBackString("foo").pushBackString("BASEURL")), std::exception);

    Segment empty;
    interpreter::Arguments args(empty, 0, 0);
    std::auto_ptr<afl::data::Value> p;
    TS_ASSERT_EQUALS(testee.handleCommand("huhu", args, p), false);
}

/** Test roundtrip behaviour. */
void
TestServerInterfaceTalkRenderServer::testRoundtrip()
{
    TalkRenderMock mock("testRoundtrip");
    server::interface::TalkRenderServer level1(mock);
    server::interface::TalkRenderClient level2(level1);
    server::interface::TalkRenderServer level3(level2);
    server::interface::TalkRenderClient level4(level3);

    // No options
    mock.expectCall("setOptions(none,none)");
    level4.setOptions(server::interface::TalkRender::Options());
    mock.checkFinish();

    mock.expectCall("render(text,none,none)");
    mock.provideReturnValue(String_t("result"));
    TS_ASSERT_EQUALS(level4.render("text", server::interface::TalkRender::Options()), "result");
    mock.checkFinish();

    // Full options
    {
        server::interface::TalkRender::Options opts;
        opts.baseUrl = "/url";
        opts.format = "fmt";
        mock.expectCall("setOptions(/url,fmt)");
        level4.setOptions(opts);
        mock.checkFinish();

        mock.expectCall("render(what,/url,fmt)");
        mock.provideReturnValue(String_t("why"));
        TS_ASSERT_EQUALS(level4.render("what", opts), "why");
        mock.checkFinish();
    }
}

