/**
  *  \file test/server/interface/talkrenderclienttest.cpp
  *  \brief Test for server::interface::TalkRenderClient
  */

#include "server/interface/talkrenderclient.hpp"

#include "afl/test/commandhandler.hpp"
#include "afl/test/testrunner.hpp"
#include "server/types.hpp"
#include "util/io.hpp"

AFL_TEST("server.interface.TalkRenderClient", a)
{
    afl::test::CommandHandler mock(a);
    server::interface::TalkRenderClient testee(mock);

    // With no options
    mock.expectCall("RENDEROPTION");
    mock.provideNewResult(0);
    testee.setOptions(server::interface::TalkRender::Options());
    mock.checkFinish();

    mock.expectCall("RENDER, some text");
    mock.provideNewResult(server::makeStringValue("some result"));
    a.checkEqual("01. render", testee.render("some text", server::interface::TalkRender::Options()), "some result");
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
        a.checkEqual("11. render", testee.render("more text", opts), "more result");
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
        a.checkEqual("21. render", testee.render("even more text", opts), "even more result");
        mock.checkFinish();
    }

    // Warning - JSON format
    {
        mock.expectCall("RENDERCHECK, base:text");
        mock.provideNewResult(util::parseJSON(afl::string::toBytes("[{\"type\":\"First\",\"token\":\"t\",\"extra\":\"e\",\"pos\":3},"
                                                                   "{\"type\":\"Second\",\"token\":\"s\",\"extra\":\"x\",\"pos\":7}]")).release());

        std::vector<server::interface::TalkRender::Warning> out;
        testee.check("base:text", out);

        a.checkEqual("31. size",   out.size(), 2U);
        a.checkEqual("32a. type",  out[0].type,  "First");
        a.checkEqual("32b. token", out[0].token, "t");
        a.checkEqual("32c. extra", out[0].extra, "e");
        a.checkEqual("32d. pos",   out[0].pos,   3);
        a.checkEqual("33a. type",  out[1].type,  "Second");
        a.checkEqual("33b. token", out[1].token, "s");
        a.checkEqual("33c. extra", out[1].extra, "x");
        a.checkEqual("33d. pos",   out[1].pos,   7);
    }

    // Warning - flat format
    {
        mock.expectCall("RENDERCHECK, test:tx");
        mock.provideNewResult(util::parseJSON(afl::string::toBytes("[[\"type\",\"F\",\"token\",\"tt\",\"extra\",\"ee\",\"pos\",2],"
                                                                   "[\"type\",\"S\",\"token\",\"ss\",\"extra\",\"xx\",\"pos\",77]]")).release());

        std::vector<server::interface::TalkRender::Warning> out;
        testee.check("test:tx", out);

        a.checkEqual("41. size",   out.size(), 2U);
        a.checkEqual("42a. type",  out[0].type,  "F");
        a.checkEqual("42b. token", out[0].token, "tt");
        a.checkEqual("42c. extra", out[0].extra, "ee");
        a.checkEqual("42d. pos",   out[0].pos,   2);
        a.checkEqual("43a. type",  out[1].type,  "S");
        a.checkEqual("43b. token", out[1].token, "ss");
        a.checkEqual("43c. extra", out[1].extra, "xx");
        a.checkEqual("43d. pos",   out[1].pos,   77);
    }
}
