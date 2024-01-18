/**
  *  \file test/server/talk/linkformattertest.cpp
  *  \brief Test for server::talk::LinkFormatter
  */

#include "server/talk/linkformatter.hpp"
#include "afl/test/testrunner.hpp"

/** Simple tests. */
AFL_TEST("server.talk.LinkFormatter", a)
{
    server::talk::LinkFormatter t;

    // Normal cases
    a.checkEqual("01", t.makeGameUrl(42, "Meaning of Life"), "host/game.cgi/42-Meaning-of-Life");
    a.checkEqual("02", t.makeForumUrl(5, "Five"), "talk/forum.cgi/5-Five");
    a.checkEqual("03", t.makePostUrl(150, "The Topic", 2501), "talk/thread.cgi/150-The-Topic#p2501");
    a.checkEqual("04", t.makeTopicUrl(150, "The Topic"), "talk/thread.cgi/150-The-Topic");
    a.checkEqual("05", t.makeUserUrl("admin"), "userinfo.cgi/admin");

    // Special cases
    a.checkEqual("11", t.makeGameUrl(42, ""), "host/game.cgi/42");
    a.checkEqual("12", t.makeGameUrl(42, "   "), "host/game.cgi/42");

    a.checkEqual("21", t.makeGameUrl(1, "Let's Rock"), "host/game.cgi/1-Let-s-Rock");
    a.checkEqual("22", t.makeGameUrl(1, "bl\xc3\xb6t"), "host/game.cgi/1-bl-t");
}
