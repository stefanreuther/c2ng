/**
  *  \file test/server/talk/talkrendertest.cpp
  *  \brief Test for server::talk::TalkRender
  */

#include "server/talk/talkrender.hpp"

#include "afl/net/redis/internaldatabase.hpp"
#include "afl/test/testrunner.hpp"
#include "server/talk/root.hpp"
#include "server/talk/session.hpp"

/** Simple test. */
AFL_TEST("server.talk.TalkRender", a)
{
    // Environment
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, server::talk::Configuration());
    server::talk::Session session;
    session.renderOptions().setFormat("raw");
    session.renderOptions().setBaseUrl("u");

    // Testee
    server::talk::TalkRender testee(session, root);

    // setOptions: modifies the configuration
    {
        server::interface::TalkRender::Options opts;
        opts.baseUrl = "z";
        testee.setOptions(opts);
    }
    a.checkEqual("01. getBaseUrl", session.renderOptions().getBaseUrl(), "z");
    a.checkEqual("02. getFormat", session.renderOptions().getFormat(), "raw");

    // render: renders, but does not modify the configuration
    {
        server::interface::TalkRender::Options opts;
        opts.format = "html";
        a.checkEqual("11. render", testee.render("text:hi", opts), "<p>hi</p>\n");
    }
    a.checkEqual("12. getBaseUrl", session.renderOptions().getBaseUrl(), "z");
    a.checkEqual("13. getFormat", session.renderOptions().getFormat(), "raw");
}
