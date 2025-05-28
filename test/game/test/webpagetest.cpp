/**
  *  \file test/game/test/webpagetest.cpp
  *  \brief Test for game::test::WebPage
  */

#include "game/test/webpage.hpp"

#include "afl/net/http/pagerequest.hpp"
#include "afl/net/http/pageresponse.hpp"
#include "afl/test/testrunner.hpp"

using afl::net::HeaderField;
using afl::net::http::PageRequest;
using afl::net::http::PageResponse;
using game::test::WebPage;

// Simple test
AFL_TEST("game.test.WebPage:simple", a)
{
    WebPage::Response RESP[] = {
        { 0, 0, 0, 0, "text" },
    };
    WebPage testee(RESP);
    a.check("01. isValidMethod", testee.isValidMethod("GET"));
    a.check("02. isValidPath",  !testee.isValidPath());

    PageRequest req("/", "/", "/");
    req.finish();

    PageResponse resp;
    testee.handleRequest(req, resp);

    HeaderField* ct = resp.headers().get("Content-Type");
    a.checkNonNull("11. Content-Type", ct);
    a.checkEqual("12. Content-Type", ct->getValue(), "text/plain");

    a.checkEqual("21. body content", afl::string::fromBytes(resp.body().getContent()), "text");
    a.checkEqual("22. getStatusCode", resp.getStatusCode(), 200);
}

// Nothing configured
AFL_TEST("game.test.WebPage:empty", a)
{
    WebPage testee(afl::base::Nothing);
    a.check("01. isValidMethod", testee.isValidMethod("GET"));
    a.check("02. isValidPath",  !testee.isValidPath());

    PageRequest req("/", "/", "/");
    req.finish();

    PageResponse resp;
    testee.handleRequest(req, resp);

    a.checkEqual("11. getStatusCode", resp.getStatusCode(), 404);
}

// Matching
AFL_TEST("game.test.WebPage:match", a)
{
    WebPage::Response RESP[] = {
        { "UPDATE", 0,               0,             0, "method-match" },
        { 0,        "One:1|Two:2",   0,             0, "two-header-match" },
        { 0,        "One:1",         0,             0, "one-header-match" },
        { 0,        0,               "one:a|two:b", 0, "two-param-match" },
        { 0,        0,               "one:a",       0, "one-param-match" },
        { 0,        0,               0,             0, "general-match" },
    };
    WebPage testee(RESP);

    // Method match
    {
        PageRequest req("/", "/", "/");
        req.finish();
        req.setMethod("UPDATE");

        PageResponse resp;
        testee.handleRequest(req, resp);

        a.checkEqual("Method: body content", afl::string::fromBytes(resp.body().getContent()), "method-match");
        a.checkEqual("Method: status", resp.getStatusCode(), 200);
    }

    // Two header match
    {
        PageRequest req("/", "/", "/");
        req.finish();
        req.setMethod("GET");
        req.headers().set("One", "1");
        req.headers().set("Two", "2");

        PageResponse resp;
        testee.handleRequest(req, resp);

        a.checkEqual("Two-header: body content", afl::string::fromBytes(resp.body().getContent()), "two-header-match");
        a.checkEqual("Two-header: status", resp.getStatusCode(), 200);
    }

    // One header match; second has wrong value
    {
        PageRequest req("/", "/", "/");
        req.finish();
        req.setMethod("GET");
        req.headers().set("One", "1");
        req.headers().set("Two", "3");

        PageResponse resp;
        testee.handleRequest(req, resp);

        a.checkEqual("One-header-wrong: body content", afl::string::fromBytes(resp.body().getContent()), "one-header-match");
        a.checkEqual("One-header-wrong: status", resp.getStatusCode(), 200);
    }

    // One header match; second not present
    {
        PageRequest req("/", "/", "/");
        req.finish();
        req.setMethod("GET");
        req.headers().set("One", "1");

        PageResponse resp;
        testee.handleRequest(req, resp);

        a.checkEqual("One-header-only: body content", afl::string::fromBytes(resp.body().getContent()), "one-header-match");
        a.checkEqual("One-header-only: status", resp.getStatusCode(), 200);
    }

    // Two parameter match
    {
        PageRequest req("/", "/", "/");
        req.finish();
        req.setMethod("GET");
        req.arguments().set("one", "a");
        req.arguments().set("two", "b");

        PageResponse resp;
        testee.handleRequest(req, resp);

        a.checkEqual("Two-param: body content", afl::string::fromBytes(resp.body().getContent()), "two-param-match");
        a.checkEqual("Two-param: status", resp.getStatusCode(), 200);
    }

    // One parameter match, second has wrong value
    {
        PageRequest req("/", "/", "/");
        req.finish();
        req.setMethod("GET");
        req.arguments().set("one", "a");
        req.arguments().set("two", "xx");

        PageResponse resp;
        testee.handleRequest(req, resp);

        a.checkEqual("One-param-wrong: body content", afl::string::fromBytes(resp.body().getContent()), "one-param-match");
        a.checkEqual("One-param-wrong: status", resp.getStatusCode(), 200);
    }

    // One parameter match, second not present
    {
        PageRequest req("/", "/", "/");
        req.finish();
        req.setMethod("GET");
        req.arguments().set("one", "a");

        PageResponse resp;
        testee.handleRequest(req, resp);

        a.checkEqual("One-param-only: body content", afl::string::fromBytes(resp.body().getContent()), "one-param-match");
        a.checkEqual("One-param-only: status", resp.getStatusCode(), 200);
    }

    // General case
    {
        PageRequest req("/", "/", "/");
        req.finish();
        req.setMethod("GET");

        PageResponse resp;
        testee.handleRequest(req, resp);

        a.checkEqual("General: body content", afl::string::fromBytes(resp.body().getContent()), "general-match");
        a.checkEqual("General: status", resp.getStatusCode(), 200);
    }
}

// Simple test
AFL_TEST("game.test.WebPage:result-headers", a)
{
    WebPage::Response RESP[] = {
        { 0, 0, 0, "Content-Type:text/json|Date:2025-05-05", "text" },
    };
    WebPage testee(RESP);

    PageRequest req("/", "/", "/");
    req.finish();

    PageResponse resp;
    testee.handleRequest(req, resp);

    HeaderField* ct = resp.headers().get("Content-Type");
    a.checkNonNull("11. Content-Type", ct);
    a.checkEqual("12. Content-Type", ct->getValue(), "text/json");

    ct = resp.headers().get("Date");
    a.checkNonNull("21. Date", ct);
    a.checkEqual("22. Date", ct->getValue(), "2025-05-05");
}
