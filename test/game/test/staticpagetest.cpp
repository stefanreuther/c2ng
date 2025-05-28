/**
  *  \file test/game/test/staticpagetest.cpp
  *  \brief Test for game::test::StaticPage
  */

#include "game/test/staticpage.hpp"

#include "afl/net/http/pagerequest.hpp"
#include "afl/net/http/pageresponse.hpp"
#include "afl/test/testrunner.hpp"

using afl::net::HeaderField;
using afl::net::http::PageRequest;
using afl::net::http::PageResponse;
using game::test::StaticPage;

AFL_TEST("game.test.StaticPage", a)
{
    StaticPage testee("text/plain", afl::string::toBytes("hello, world\n"));
    PageRequest req("/", "/", "/");
    req.finish();
    PageResponse resp;

    a.check("01. isValidMethod",  testee.isValidMethod("GET"));
    a.check("02. isValidMethod", !testee.isValidMethod("POST"));
    a.check("03. isValidPath",   !testee.isValidPath());

    testee.handleRequest(req, resp);

    HeaderField* ct = resp.headers().get("Content-Type");
    a.checkNonNull("11. Content-Type", ct);
    a.checkEqual("12. Content-Type", ct->getValue(), "text/plain");

    a.checkEqual("21. body content", afl::string::fromBytes(resp.body().getContent()), "hello, world\n");
    a.checkEqual("22. getStatusCode", resp.getStatusCode(), 200);
}
