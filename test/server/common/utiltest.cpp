/**
  *  \file test/server/common/utiltest.cpp
  *  \brief Test for server::common::Util
  */

#include "server/common/util.hpp"
#include "afl/test/testrunner.hpp"

/** Test simplifyUserName. */
AFL_TEST("server.common.Util:simplifyUserName", a)
{
    a.checkEqual("01", server::common::simplifyUserName("fred"), "fred");
    a.checkEqual("02", server::common::simplifyUserName("Fred"), "fred");
    a.checkEqual("03", server::common::simplifyUserName("J.R.Luser"), "j_r_luser");
    a.checkEqual("04", server::common::simplifyUserName("-=me=-"), "me");
    a.checkEqual("05", server::common::simplifyUserName("H\xc2\x80Y"), "h_y");
    a.checkEqual("06", server::common::simplifyUserName("-=\xf3=-"), "");
}
