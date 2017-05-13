/**
  *  \file u/t_server_talk_linkformatter.cpp
  *  \brief Test for server::talk::LinkFormatter
  */

#include "server/talk/linkformatter.hpp"

#include "t_server_talk.hpp"

/** Simple tests. */
void
TestServerTalkLinkFormatter::testIt()
{
    server::talk::LinkFormatter t;

    // Normal cases
    TS_ASSERT_EQUALS(t.makeGameUrl(42, "Meaning of Life"), "host/game.cgi/42-Meaning-of-Life");
    TS_ASSERT_EQUALS(t.makeForumUrl(5, "Five"), "talk/forum.cgi/5-Five");
    TS_ASSERT_EQUALS(t.makePostUrl(150, "The Topic", 2501), "talk/thread.cgi/150-The-Topic#p2501");
    TS_ASSERT_EQUALS(t.makeTopicUrl(150, "The Topic"), "talk/thread.cgi/150-The-Topic");
    TS_ASSERT_EQUALS(t.makeUserUrl("admin"), "userinfo.cgi/admin");

    // Special cases
    TS_ASSERT_EQUALS(t.makeGameUrl(42, ""), "host/game.cgi/42");
    TS_ASSERT_EQUALS(t.makeGameUrl(42, "   "), "host/game.cgi/42");

    TS_ASSERT_EQUALS(t.makeGameUrl(1, "Let's Rock"), "host/game.cgi/1-Let-s-Rock");
    TS_ASSERT_EQUALS(t.makeGameUrl(1, "bl\xc3\xb6t"), "host/game.cgi/1-bl-t");
}

