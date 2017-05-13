/**
  *  \file u/t_server_talk_render_context.cpp
  *  \brief Test for server::talk::render::Context
  */

#include "server/talk/render/context.hpp"

#include "t_server_talk_render.hpp"

/** Simple test. */
void
TestServerTalkRenderContext::testIt()
{
    server::talk::render::Context testee("u");

    // Initial state
    TS_ASSERT_EQUALS(testee.getUser(), "u");
    TS_ASSERT_EQUALS(testee.getMessageId(), 0);
    TS_ASSERT_EQUALS(testee.getMessageAuthor(), "");

    // Message Id
    testee.setMessageId(42);
    TS_ASSERT_EQUALS(testee.getUser(), "u");
    TS_ASSERT_EQUALS(testee.getMessageId(), 42);
    TS_ASSERT_EQUALS(testee.getMessageAuthor(), "");

    // Message Author
    testee.setMessageAuthor("a");
    TS_ASSERT_EQUALS(testee.getUser(), "u");
    TS_ASSERT_EQUALS(testee.getMessageId(), 0);  // reset!
    TS_ASSERT_EQUALS(testee.getMessageAuthor(), "a");

    // Id again
    testee.setMessageId(13);
    TS_ASSERT_EQUALS(testee.getUser(), "u");
    TS_ASSERT_EQUALS(testee.getMessageId(), 13);
    TS_ASSERT_EQUALS(testee.getMessageAuthor(), ""); // reset!
}

