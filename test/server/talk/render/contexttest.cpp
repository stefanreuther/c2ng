/**
  *  \file test/server/talk/render/contexttest.cpp
  *  \brief Test for server::talk::render::Context
  */

#include "server/talk/render/context.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test. */
AFL_TEST("server.talk.render.Context", a)
{
    server::talk::render::Context testee("u");

    // Initial state
    a.checkEqual("01. getUser",          testee.getUser(), "u");
    a.checkEqual("02. getMessageId",     testee.getMessageId(), 0);
    a.checkEqual("03. getMessageAuthor", testee.getMessageAuthor(), "");

    // Message Id
    testee.setMessageId(42);
    a.checkEqual("11. getUser",          testee.getUser(), "u");
    a.checkEqual("12. getMessageId",     testee.getMessageId(), 42);
    a.checkEqual("13. getMessageAuthor", testee.getMessageAuthor(), "");

    // Message Author
    testee.setMessageAuthor("a");
    a.checkEqual("21. getUser",          testee.getUser(), "u");
    a.checkEqual("22. getMessageId",     testee.getMessageId(), 0);  // reset!
    a.checkEqual("23. getMessageAuthor", testee.getMessageAuthor(), "a");

    // Id again
    testee.setMessageId(13);
    a.checkEqual("31. getUser",          testee.getUser(), "u");
    a.checkEqual("32. getMessageId",     testee.getMessageId(), 13);
    a.checkEqual("33. getMessageAuthor", testee.getMessageAuthor(), ""); // reset!
}
