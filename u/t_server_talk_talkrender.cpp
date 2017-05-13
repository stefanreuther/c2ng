/**
  *  \file u/t_server_talk_talkrender.cpp
  *  \brief Test for server::talk::TalkRender
  */

#include "server/talk/talkrender.hpp"

#include "t_server_talk.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "server/talk/root.hpp"
#include "server/talk/session.hpp"

/** Simple test. */
void
TestServerTalkTalkRender::testIt()
{
    // Environment
    afl::net::redis::InternalDatabase db;
    afl::net::NullCommandHandler mail;
    server::talk::Root root(db, mail, server::talk::Configuration());
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
    TS_ASSERT_EQUALS(session.renderOptions().getBaseUrl(), "z");
    TS_ASSERT_EQUALS(session.renderOptions().getFormat(), "raw");

    // render: renders, but does not modify the configuration
    {
        server::interface::TalkRender::Options opts;
        opts.format = "html";
        TS_ASSERT_EQUALS(testee.render("text:hi", opts), "<p>hi</p>\n");
    }
    TS_ASSERT_EQUALS(session.renderOptions().getBaseUrl(), "z");
    TS_ASSERT_EQUALS(session.renderOptions().getFormat(), "raw");
}
