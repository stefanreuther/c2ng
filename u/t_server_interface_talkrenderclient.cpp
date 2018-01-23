/**
  *  \file u/t_server_interface_talkrenderclient.cpp
  *  \brief Test for server::interface::TalkRenderClient
  */

#include "server/interface/talkrenderclient.hpp"

#include "t_server_interface.hpp"
#include "afl/test/commandhandler.hpp"
#include "server/types.hpp"

/** Test setOptions(). */
void
TestServerInterfaceTalkRenderClient::testIt()
{
    afl::test::CommandHandler mock("testIt");
    server::interface::TalkRenderClient testee(mock);

    // With no options
    mock.expectCall("RENDEROPTION");
    mock.provideNewResult(0);
    testee.setOptions(server::interface::TalkRender::Options());
    mock.checkFinish();

    mock.expectCall("RENDER, some text");
    mock.provideNewResult(server::makeStringValue("some result"));
    TS_ASSERT_EQUALS(testee.render("some text", server::interface::TalkRender::Options()), "some result");
    mock.checkFinish();

    // ...with one parameter
    {
        mock.expectCall("RENDEROPTION, BASEURL, /foo/");
        mock.provideNewResult(0);
        server::interface::TalkRender::Options opts;
        opts.baseUrl = "/foo/";
        testee.setOptions(opts);
        mock.checkFinish();

        mock.expectCall("RENDER, more text, BASEURL, /foo/");
        mock.provideNewResult(server::makeStringValue("more result"));
        TS_ASSERT_EQUALS(testee.render("more text", opts), "more result");
        mock.checkFinish();
    }

    {
        mock.expectCall("RENDEROPTION, FORMAT, text");
        mock.provideNewResult(0);
        server::interface::TalkRender::Options opts;
        opts.format = "text";
        testee.setOptions(opts);
        mock.checkFinish();

        mock.expectCall("RENDER, even more text, FORMAT, text");
        mock.provideNewResult(server::makeStringValue("even more result"));
        TS_ASSERT_EQUALS(testee.render("even more text", opts), "even more result");
        mock.checkFinish();
    }
}
