/**
  *  \file u/t_server_talk_render_options.cpp
  *  \brief Test for server::talk::render::Options
  */

#include "server/talk/render/options.hpp"

#include "t_server_talk_render.hpp"

/** Simple test. */
void
TestServerTalkRenderOptions::testIt()
{
    server::talk::render::Options testee;

    // Initial state
    TS_ASSERT_EQUALS(testee.getBaseUrl(), "");
    TS_ASSERT_EQUALS(testee.getFormat(), "raw");

    // Set/get
    testee.setBaseUrl("/test/");
    TS_ASSERT_EQUALS(testee.getBaseUrl(), "/test/");
    testee.setFormat("html");
    TS_ASSERT_EQUALS(testee.getFormat(), "html");

    // Update from empty
    testee.updateFrom(server::interface::TalkRender::Options());
    TS_ASSERT_EQUALS(testee.getBaseUrl(), "/test/");
    TS_ASSERT_EQUALS(testee.getFormat(), "html");

    // Update parts
    {
        server::interface::TalkRender::Options o;
        o.baseUrl = "/base/";
        testee.updateFrom(o);
        TS_ASSERT_EQUALS(testee.getBaseUrl(), "/base/");
        TS_ASSERT_EQUALS(testee.getFormat(), "html");
    }
    {
        server::interface::TalkRender::Options o;
        o.format = "quote:forum";
        testee.updateFrom(o);
        TS_ASSERT_EQUALS(testee.getBaseUrl(), "/base/");
        TS_ASSERT_EQUALS(testee.getFormat(), "quote:forum");
    }
}
