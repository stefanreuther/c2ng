/**
  *  \file u/t_server_interface_mailqueueclient.cpp
  *  \brief Test for server::interface::MailQueueClient
  */

#include "server/interface/mailqueueclient.hpp"

#include "t_server_interface.hpp"
#include "u/helper/commandhandlermock.hpp"

/** Mail queue client tests. */
void
TestServerInterfaceMailQueueClient::testIt()
{
    CommandHandlerMock mock;
    server::interface::MailQueueClient testee(mock);

    // startMessage/MAIL
    mock.expectCall("MAIL|the-template");
    mock.provideReturnValue(0);
    testee.startMessage("the-template", afl::base::Nothing);

    mock.expectCall("MAIL|the-second-template|unique1234");
    mock.provideReturnValue(0);
    testee.startMessage("the-second-template", String_t("unique1234"));

    // addParameter/PARAM
    mock.expectCall("PARAM|p1|v1");
    mock.provideReturnValue(0);
    testee.addParameter("p1", "v1");

    // addAttachment/ATTACH
    mock.expectCall("ATTACH|c2file://foo/bar");
    mock.provideReturnValue(0);
    testee.addAttachment("c2file://foo/bar");

    // send/SEND
    String_t rxs[] = { "joe", "jack", "jill" };
    mock.expectCall("SEND|joe|jack|jill");
    mock.provideReturnValue(0);
    testee.send(rxs);

    // cancelMessage/CANCEL
    mock.expectCall("CANCEL|unique6789");
    mock.provideReturnValue(0);
    testee.cancelMessage("unique6789");

    // confirmAddress/CONFIRM
    mock.expectCall("CONFIRM|user@host|g3h31m");
    mock.provideReturnValue(0);
    testee.confirmAddress("user@host", "g3h31m", afl::base::Nothing);

    mock.expectCall("CONFIRM|user2@other.host|s3cr3t|ip=127.0.0.1");
    mock.provideReturnValue(0);
    testee.confirmAddress("user2@other.host", "s3cr3t", String_t("ip=127.0.0.1"));

    // requestAddress/REQUEST
    mock.expectCall("REQUEST|joe");
    mock.provideReturnValue(0);
    testee.requestAddress("joe");

    // runQueue
    mock.expectCall("RUNQUEUE");
    mock.provideReturnValue(0);
    testee.runQueue();

    mock.checkFinish();
}
