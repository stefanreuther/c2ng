/**
  *  \file u/t_server_interface_baseclient.cpp
  *  \brief Test for server::interface::BaseClient
  */

#include "server/interface/baseclient.hpp"

#include "t_server_interface.hpp"
#include "u/helper/commandhandlermock.hpp"
#include "server/types.hpp"

/** Test it. */
void
TestServerInterfaceBaseClient::testIt()
{
    CommandHandlerMock mock;
    server::interface::BaseClient testee(mock);

    mock.expectCall("PING");
    mock.provideReturnValue(server::makeStringValue("PONG"));
    TS_ASSERT_EQUALS(testee.ping(), "PONG");

    mock.expectCall("USER|1023");
    mock.provideReturnValue(0);
    testee.setUserContext("1023");

    mock.expectCall("USER|");
    mock.provideReturnValue(0);
    testee.setUserContext(String_t());

    mock.checkFinish();
}

