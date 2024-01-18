/**
  *  \file test/server/talk/render/optionstest.cpp
  *  \brief Test for server::talk::render::Options
  */

#include "server/talk/render/options.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test. */
AFL_TEST("server.talk.render.Options", a)
{
    server::talk::render::Options testee;

    // Initial state
    a.checkEqual("01. getBaseUrl", testee.getBaseUrl(), "");
    a.checkEqual("02. getFormat", testee.getFormat(), "raw");

    // Set/get
    testee.setBaseUrl("/test/");
    a.checkEqual("11. getBaseUrl", testee.getBaseUrl(), "/test/");
    testee.setFormat("html");
    a.checkEqual("12. getFormat", testee.getFormat(), "html");

    // Update from empty
    testee.updateFrom(server::interface::TalkRender::Options());
    a.checkEqual("21. getBaseUrl", testee.getBaseUrl(), "/test/");
    a.checkEqual("22. getFormat", testee.getFormat(), "html");

    // Update parts
    {
        server::interface::TalkRender::Options o;
        o.baseUrl = "/base/";
        testee.updateFrom(o);
        a.checkEqual("31. getBaseUrl", testee.getBaseUrl(), "/base/");
        a.checkEqual("32. getFormat", testee.getFormat(), "html");
    }
    {
        server::interface::TalkRender::Options o;
        o.format = "quote:forum";
        testee.updateFrom(o);
        a.checkEqual("33. getBaseUrl", testee.getBaseUrl(), "/base/");
        a.checkEqual("34. getFormat", testee.getFormat(), "quote:forum");
    }
}
