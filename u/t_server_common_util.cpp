/**
  *  \file u/t_server_common_util.cpp
  *  \brief Test for server::common::util
  */

#include "server/common/util.hpp"

#include "t_server_common.hpp"

/** Test simplifyUserName. */
void
TestServerCommonutil::testSimplifyUserName()
{
    TS_ASSERT_EQUALS(server::common::simplifyUserName("fred"), "fred");
    TS_ASSERT_EQUALS(server::common::simplifyUserName("Fred"), "fred");
    TS_ASSERT_EQUALS(server::common::simplifyUserName("J.R.Luser"), "j_r_luser");
    TS_ASSERT_EQUALS(server::common::simplifyUserName("-=me=-"), "me");
    TS_ASSERT_EQUALS(server::common::simplifyUserName("H\xc2\x80Y"), "h_y");
    TS_ASSERT_EQUALS(server::common::simplifyUserName("-=\xf3=-"), "");
}

