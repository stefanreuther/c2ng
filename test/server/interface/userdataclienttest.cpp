/**
  *  \file test/server/interface/userdataclienttest.cpp
  *  \brief Test for server::interface::UserDataClient
  */

#include "server/interface/userdataclient.hpp"

#include "afl/test/commandhandler.hpp"
#include "afl/test/testrunner.hpp"
#include "server/types.hpp"

/** Test all commands. */
AFL_TEST("server.interface.UserDataClient", a)
{
    afl::test::CommandHandler mock(a);
    server::interface::UserDataClient testee(mock);

    mock.expectCall("UGET, ua, ka");
    mock.provideNewResult(server::makeStringValue("va"));
    a.checkEqual("01. get", testee.get("ua", "ka"), "va");

    mock.expectCall("USET, ub, kb, vb");
    mock.provideNewResult(server::makeStringValue("OK"));
    AFL_CHECK_SUCCEEDS(a("11. set"), testee.set("ub", "kb", "vb"));

    mock.checkFinish();
}
