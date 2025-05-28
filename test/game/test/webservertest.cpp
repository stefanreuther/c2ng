/**
  *  \file test/game/test/webservertest.cpp
  *  \brief Test for game::test::WebServer
  */

#include "game/test/webserver.hpp"

#include "afl/net/http/clientrequest.hpp"
#include "afl/net/http/page.hpp"
#include "afl/net/http/pagerequest.hpp"
#include "afl/net/http/pageresponse.hpp"
#include "afl/net/http/simpledownloadlistener.hpp"
#include "afl/net/internalnetworkstack.hpp"
#include "afl/net/url.hpp"
#include "afl/test/testrunner.hpp"

using afl::base::Ref;
using afl::net::InternalNetworkStack;
using afl::net::Url;
using afl::net::http::ClientRequest;
using afl::net::http::Page;
using afl::net::http::PageRequest;
using afl::net::http::PageResponse;
using afl::net::http::SimpleDownloadListener;
using game::test::WebServer;

namespace {
    class TestPage : public Page {
     public:
        virtual bool isValidMethod(const String_t& method) const
            { return method == "GET"; }
        virtual bool isValidPath() const
            { return false; }
        virtual void handleRequest(PageRequest& /*in*/, PageResponse& out)
            {
                out.body().handleFullData(afl::string::toBytes("text"));
                out.headers().add("Content-Type", "text/plain");
            }
    };
}

/* Test success case.
   A: set up single page. Download it.
   E: correct result */
AFL_TEST("game.test.WebServer:success", a)
{
    Ref<InternalNetworkStack> stack = InternalNetworkStack::create();
    WebServer testee(*stack);
    testee.addNewPage("host", "/page", new TestPage());

    Url u;
    a.check("parse", u.parse("http://host/page"));
    SimpleDownloadListener sdl;
    testee.manager().getFile(u, sdl);
    SimpleDownloadListener::Status st = sdl.wait();

    a.checkEqual("status", st, SimpleDownloadListener::Succeeded);
    a.checkEqual("statusCode", sdl.getStatusCode(), 200);
    a.checkEqual("content", afl::string::fromBytes(sdl.getResponseData()), "text");
}

/* Test bad path.
   A: set up single page. Download another page.
   E: "404" result */
AFL_TEST("game.test.WebServer:bad-path", a)
{
    Ref<InternalNetworkStack> stack = InternalNetworkStack::create();
    WebServer testee(*stack);
    testee.addNewPage("host", "/page", new TestPage());

    Url u;
    a.check("parse", u.parse("http://host/other-page"));
    SimpleDownloadListener sdl;
    testee.manager().getFile(u, sdl);
    SimpleDownloadListener::Status st = sdl.wait();

    a.checkEqual("status", st, SimpleDownloadListener::Succeeded);
    a.checkEqual("statusCode", sdl.getStatusCode(), 404);
    a.checkDifferent("content", afl::string::fromBytes(sdl.getResponseData()), "text");
}

/* Test bad host.
   A: set up single page. Download from another host.
   E: connection failed result */
AFL_TEST("game.test.WebServer:bad-host", a)
{
    Ref<InternalNetworkStack> stack = InternalNetworkStack::create();
    WebServer testee(*stack);
    testee.addNewPage("host", "/page", new TestPage());

    Url u;
    a.check("parse", u.parse("http://other-host/other"));
    SimpleDownloadListener sdl;
    testee.manager().getFile(u, sdl);
    SimpleDownloadListener::Status st = sdl.wait();

    a.checkEqual("status", st, SimpleDownloadListener::Failed);
    a.checkEqual("failure", sdl.getFailureReason(), ClientRequest::ConnectionFailed);
}

/* Test multiple paths.
   A: set up multiple pages on single host
   E: both paths can be downloaded */
AFL_TEST("game.test.WebServer:multiple-paths", a)
{
    Ref<InternalNetworkStack> stack = InternalNetworkStack::create();
    WebServer testee(*stack);
    testee.addNewPage("host", "/page", new TestPage());
    testee.addNewPage("host", "/other", new TestPage());

    // Download second path
    {
        Url u;
        a.check("01. parse", u.parse("http://host/other"));
        SimpleDownloadListener sdl;
        testee.manager().getFile(u, sdl);
        SimpleDownloadListener::Status st = sdl.wait();

        a.checkEqual("02. status", st, SimpleDownloadListener::Succeeded);
        a.checkEqual("03. statusCode", sdl.getStatusCode(), 200);
    }

    // Download first path
    {
        Url u;
        a.check("11. parse", u.parse("http://host/page"));
        SimpleDownloadListener sdl;
        testee.manager().getFile(u, sdl);
        SimpleDownloadListener::Status st = sdl.wait();

        a.checkEqual("12. status", st, SimpleDownloadListener::Succeeded);
        a.checkEqual("13. statusCode", sdl.getStatusCode(), 200);
    }
}

/* Test multiple hosts.
   A: set up multiple pages on multiple hosts
   E: correct result according to host/path mapping */
AFL_TEST("game.test.WebServer:multiple-hosts", a)
{
    Ref<InternalNetworkStack> stack = InternalNetworkStack::create();
    WebServer testee(*stack);
    testee.addNewPage("host", "/page", new TestPage());
    testee.addNewPage("other", "/other", new TestPage());

    // Download second path, second host
    {
        Url u;
        a.check("01. parse", u.parse("http://other/other"));
        SimpleDownloadListener sdl;
        testee.manager().getFile(u, sdl);
        SimpleDownloadListener::Status st = sdl.wait();

        a.checkEqual("02. status", st, SimpleDownloadListener::Succeeded);
        a.checkEqual("03. statusCode", sdl.getStatusCode(), 200);
    }

    // Download first path, second host (fails)
    {
        Url u;
        a.check("11. parse", u.parse("http://other/page"));
        SimpleDownloadListener sdl;
        testee.manager().getFile(u, sdl);
        SimpleDownloadListener::Status st = sdl.wait();

        a.checkEqual("12. status", st, SimpleDownloadListener::Succeeded);
        a.checkEqual("13. statusCode", sdl.getStatusCode(), 404);
    }
}

/* Test reset.
   A: configure a page, call reset()
   E: download correctly rejected */
AFL_TEST("game.test.WebServer:reset", a)
{
    Ref<InternalNetworkStack> stack = InternalNetworkStack::create();
    WebServer testee(*stack);
    testee.addNewPage("host", "/page", new TestPage());
    testee.reset();

    Url u;
    a.check("parse", u.parse("http://host/page"));
    SimpleDownloadListener sdl;
    testee.manager().getFile(u, sdl);
    SimpleDownloadListener::Status st = sdl.wait();

    a.checkEqual("status", st, SimpleDownloadListener::Failed);
    a.checkEqual("failure", sdl.getFailureReason(), ClientRequest::ConnectionFailed);
}
