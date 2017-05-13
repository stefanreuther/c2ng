/**
  *  \file u/t_server_interface_talkrenderclient.cpp
  *  \brief Test for server::interface::TalkRenderClient
  */

#include "server/interface/talkrenderclient.hpp"

#include "t_server_interface.hpp"
#include "u/helper/commandhandlermock.hpp"
#include "server/types.hpp"

/** Test setOptions(). */
void
TestServerInterfaceTalkRenderClient::testIt()
{
    CommandHandlerMock mock;
    server::interface::TalkRenderClient testee(mock);

    // With no options
    mock.expectCall("RENDEROPTION");
    mock.provideReturnValue(0);
    testee.setOptions(server::interface::TalkRender::Options());
    mock.checkFinish();

    mock.expectCall("RENDER|some text");
    mock.provideReturnValue(server::makeStringValue("some result"));
    TS_ASSERT_EQUALS(testee.render("some text", server::interface::TalkRender::Options()), "some result");
    mock.checkFinish();

    // ...with one parameter
    {
        mock.expectCall("RENDEROPTION|BASEURL|/foo/");
        mock.provideReturnValue(0);
        server::interface::TalkRender::Options opts;
        opts.baseUrl = "/foo/";
        testee.setOptions(opts);
        mock.checkFinish();

        mock.expectCall("RENDER|more text|BASEURL|/foo/");
        mock.provideReturnValue(server::makeStringValue("more result"));
        TS_ASSERT_EQUALS(testee.render("more text", opts), "more result");
        mock.checkFinish();
    }

    {
        mock.expectCall("RENDEROPTION|FORMAT|text");
        mock.provideReturnValue(0);
        server::interface::TalkRender::Options opts;
        opts.format = "text";
        testee.setOptions(opts);
        mock.checkFinish();

        mock.expectCall("RENDER|even more text|FORMAT|text");
        mock.provideReturnValue(server::makeStringValue("even more result"));
        TS_ASSERT_EQUALS(testee.render("even more text", opts), "even more result");
        mock.checkFinish();
    }
}
