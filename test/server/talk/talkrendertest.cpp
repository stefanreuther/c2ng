/**
  *  \file test/server/talk/talkrendertest.cpp
  *  \brief Test for server::talk::TalkRender
  */

#include "server/talk/talkrender.hpp"

#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringkey.hpp"
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

    // Create a user for link verification
    afl::net::redis::StringKey(db, "uid:fred").set("2000");
    afl::net::redis::StringKey(db, "user:2000:name").set("fred");

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

    // check
    // - unsupported
    {
        std::vector<server::interface::TalkRender::Warning> ws;
        testee.check("unsupported:text", ws);
        a.checkEqual("21. size", ws.size(), 1U);
        a.checkEqual("22. type", ws[0].type, "Unsupported");
    }
    // - supported but warns
    {
        std::vector<server::interface::TalkRender::Warning> ws;
        testee.check("forum:[quote]q", ws);
        a.checkEqual("21. size", ws.size(), 2U);
        a.checkEqual("22. type", ws[0].type, "MissingClose");
        a.checkEqual("23. type", ws[1].type, "NoOwnText");
    }
    // - link to valid user, does not warn
    {
        std::vector<server::interface::TalkRender::Warning> ws;
        testee.check("forum:@fred", ws);
        a.checkEqual("31. size", ws.size(), 0U);
    }
    // - link to invalid user, warns (verifies proper integration with database)
    {
        std::vector<server::interface::TalkRender::Warning> ws;
        testee.check("forum:@barney", ws);
        a.checkEqual("41. size", ws.size(), 1U);
        a.checkEqual("42. size", ws[0].type, "BadLink");
        a.checkEqual("43. size", ws[0].extra, "barney");
    }
    // - no own text, standard case
    {
        std::vector<server::interface::TalkRender::Warning> ws;
        testee.check("forum:", ws);
        a.checkEqual("51. size", ws.size(), 1U);
        a.checkEqual("52. size", ws[0].type, "NoOwnText");
    }
    // - no own text, standard case with quote
    {
        std::vector<server::interface::TalkRender::Warning> ws;
        testee.check("forum:[quote]foo[/quote]", ws);
        a.checkEqual("61. size", ws.size(), 1U);
        a.checkEqual("62. size", ws[0].type, "NoOwnText");
    }
}
