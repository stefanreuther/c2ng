/**
  *  \file test/server/interface/baseclienttest.cpp
  *  \brief Test for server::interface::BaseClient
  */

#include "server/interface/baseclient.hpp"

#include "afl/test/commandhandler.hpp"
#include "afl/test/testrunner.hpp"
#include "server/types.hpp"

/** Test it. */
AFL_TEST("server.interface.BaseClient", a)
{
    afl::test::CommandHandler mock(a);
    server::interface::BaseClient testee(mock);

    mock.expectCall("PING");
    mock.provideNewResult(server::makeStringValue("PONG"));
    a.checkEqual("ping", testee.ping(), "PONG");

    mock.expectCall("USER, 1023");
    mock.provideNewResult(0);
    testee.setUserContext("1023");

    mock.expectCall("USER, ");
    mock.provideNewResult(0);
    testee.setUserContext(String_t());

    mock.checkFinish();
}
