/**
  *  \file test/server/talk/nulllinkparsertest.cpp
  *  \brief Test for server::talk::NullLinkParser
  */

#include "server/talk/nulllinkparser.hpp"

#include "afl/test/testrunner.hpp"

AFL_TEST("server.talk.NullLinkParser", a)
{
    server::talk::NullLinkParser testee;
    a.check("01. parseGameLink",    testee.parseGameLink("x").isValid());
    a.check("02. parseForumLink",   testee.parseForumLink("x").isValid());
    a.check("03. parseTopicLink",   testee.parseTopicLink("x").isValid());
    a.check("04. parseMessageLink", testee.parseMessageLink("x").isValid());
    a.check("05. parseUserLink",    testee.parseUserLink("x").isValid());
}
