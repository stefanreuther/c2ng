/**
  *  \file test/server/interface/mailqueueclienttest.cpp
  *  \brief Test for server::interface::MailQueueClient
  */

#include "server/interface/mailqueueclient.hpp"

#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/test/commandhandler.hpp"
#include "afl/test/testrunner.hpp"
#include "server/types.hpp"

/** Mail queue client tests. */
AFL_TEST("server.interface.MailQueueClient", a)
{
    afl::test::CommandHandler mock(a);
    server::interface::MailQueueClient testee(mock);

    // startMessage/MAIL
    mock.expectCall("MAIL, the-template");
    mock.provideNewResult(0);
    testee.startMessage("the-template", afl::base::Nothing);

    mock.expectCall("MAIL, the-second-template, unique1234");
    mock.provideNewResult(0);
    testee.startMessage("the-second-template", String_t("unique1234"));

    // addParameter/PARAM
    mock.expectCall("PARAM, p1, v1");
    mock.provideNewResult(0);
    testee.addParameter("p1", "v1");

    // addAttachment/ATTACH
    mock.expectCall("ATTACH, c2file://foo/bar");
    mock.provideNewResult(0);
    testee.addAttachment("c2file://foo/bar");

    // send/SEND
    String_t rxs[] = { "joe", "jack", "jill" };
    mock.expectCall("SEND, joe, jack, jill");
    mock.provideNewResult(0);
    testee.send(rxs);

    // cancelMessage/CANCEL
    mock.expectCall("CANCEL, unique6789");
    mock.provideNewResult(0);
    testee.cancelMessage("unique6789");

    // confirmAddress/CONFIRM
    mock.expectCall("CONFIRM, user@host, g3h31m");
    mock.provideNewResult(0);
    testee.confirmAddress("user@host", "g3h31m", afl::base::Nothing);

    mock.expectCall("CONFIRM, user2@other.host, s3cr3t, ip=127.0.0.1");
    mock.provideNewResult(0);
    testee.confirmAddress("user2@other.host", "s3cr3t", String_t("ip=127.0.0.1"));

    // requestAddress/REQUEST
    mock.expectCall("REQUEST, joe");
    mock.provideNewResult(0);
    testee.requestAddress("joe");

    // runQueue
    mock.expectCall("RUNQUEUE");
    mock.provideNewResult(0);
    testee.runQueue();

    // getUserStatus/STATUS
    {
        afl::data::Hash::Ref_t h = afl::data::Hash::create();
        h->setNew("address", server::makeStringValue("foo@bar"));
        h->setNew("status", server::makeStringValue("r"));
        mock.expectCall("STATUS, jack");
        mock.provideNewResult(new afl::data::HashValue(h));

        server::interface::MailQueueClient::UserStatus st = testee.getUserStatus("jack");
        a.checkEqual("01. address", st.address, "foo@bar");
        a.checkEqual("02. status", st.status, server::interface::MailQueueClient::Requested);
    }

    mock.checkFinish();
}
