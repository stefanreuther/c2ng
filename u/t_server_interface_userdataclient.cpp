/**
  *  \file u/t_server_interface_userdataclient.cpp
  *  \brief Test for server::interface::UserDataClient
  */

#include "server/interface/userdataclient.hpp"

#include "t_server_interface.hpp"
#include "afl/test/commandhandler.hpp"
#include "server/types.hpp"

/** Test all commands. */
void
TestServerInterfaceUserDataClient::testIt()
{
    afl::test::CommandHandler mock("TestServerInterfaceUserDataClient::testIt");
    server::interface::UserDataClient testee(mock);

    mock.expectCall("UGET, ua, ka");
    mock.provideNewResult(server::makeStringValue("va"));
    TS_ASSERT_EQUALS(testee.get("ua", "ka"), "va");

    mock.expectCall("USET, ub, kb, vb");
    mock.provideNewResult(server::makeStringValue("OK"));
    TS_ASSERT_THROWS_NOTHING(testee.set("ub", "kb", "vb"));

    mock.checkFinish();
}

