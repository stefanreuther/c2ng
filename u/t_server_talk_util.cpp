/**
  *  \file u/t_server_talk_util.cpp
  *  \brief Test for server/talk/util.hpp
  */

#include "server/talk/util.hpp"

#include "t_server_talk.hpp"

/** Test simplifyUserName. */
void
TestServerTalkUtil::testSimplifyUserName()
{
    TS_ASSERT_EQUALS(server::talk::simplifyUserName("fred"), "fred");
    TS_ASSERT_EQUALS(server::talk::simplifyUserName("Fred"), "fred");
    TS_ASSERT_EQUALS(server::talk::simplifyUserName("J.R.Luser"), "j_r_luser");
    TS_ASSERT_EQUALS(server::talk::simplifyUserName("-=me=-"), "me");
    TS_ASSERT_EQUALS(server::talk::simplifyUserName("H\xc2\x80Y"), "h_y");
    TS_ASSERT_EQUALS(server::talk::simplifyUserName("-=\xf3=-"), "");
}

