/**
  *  \file u/t_server_interface_mailqueueserver.cpp
  *  \brief Test for server::interface::MailQueueServer
  */

#include "server/interface/mailqueueserver.hpp"

#include <stdexcept>
#include "t_server_interface.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "server/interface/mailqueue.hpp"
#include "server/interface/mailqueueclient.hpp"

using afl::string::Format;

namespace {
    class MailQueueMock : public server::interface::MailQueue, public afl::test::CallReceiver {
     public:
        MailQueueMock(afl::test::Assert a)
            : CallReceiver(a)
            { }
        virtual void startMessage(String_t templateName, afl::base::Optional<String_t> uniqueId)
            { checkCall(Format("startMessage(%s,%s)", templateName, uniqueId.orElse("no-id"))); }

        virtual void addParameter(String_t parameterName, String_t value)
            { checkCall(Format("addParameter(%s,%s)", parameterName, value)); }

        virtual void addAttachment(String_t url)
            { checkCall(Format("addAttachment(%s)", url)); }

        virtual void send(afl::base::Memory<const String_t> receivers)
            {
                String_t cmd = "send(";
                const char* sep = "";
                while (const String_t* p = receivers.eat()) {
                    cmd += sep;
                    cmd += *p;
                    sep = ",";
                }
                cmd += ")";
                checkCall(cmd);
            }

        virtual void cancelMessage(String_t uniqueId)
            { checkCall(Format("cancelMessage(%s)", uniqueId)); }

        virtual void confirmAddress(String_t address, String_t key, afl::base::Optional<String_t> info)
            { checkCall(Format("confirmAddress(%s,%s,%s)", address, key, info.orElse("no-info"))); }

        virtual void requestAddress(String_t user)
            { checkCall(Format("requestAddress(%s)", user)); }

        virtual void runQueue()
            { checkCall("runQueue()"); }
    };
}

/** Mail queue server tests. */
void
TestServerInterfaceMailQueueServer::testIt()
{
    using afl::data::Segment;

    MailQueueMock mock("testIt");
    server::interface::MailQueueServer testee(mock);

    // Commands
    mock.expectCall("startMessage(the-template,no-id)");
    testee.callVoid(Segment().pushBackString("MAIL").pushBackString("the-template"));
    mock.expectCall("startMessage(the-template,the-uniqueId)");
    testee.callVoid(Segment().pushBackString("MAIL").pushBackString("the-template").pushBackString("the-uniqueId"));

    mock.expectCall("addParameter(key,value)");
    testee.callVoid(Segment().pushBackString("PARAM").pushBackString("key").pushBackString("value"));

    mock.expectCall("addAttachment(http://foo)");
    testee.callVoid(Segment().pushBackString("ATTACH").pushBackString("http://foo"));

    mock.expectCall("send(fred,wilma,barney,betty)");
    testee.callVoid(Segment().pushBackString("SEND").pushBackString("fred").pushBackString("wilma").pushBackString("barney").pushBackString("betty"));
    mock.expectCall("send()");
    testee.callVoid(Segment().pushBackString("SEND"));

    mock.expectCall("cancelMessage(oops)");
    testee.callVoid(Segment().pushBackString("CANCEL").pushBackString("oops"));

    mock.expectCall("confirmAddress(trump@whitehouse.gov,whatever,no-info)");
    testee.callVoid(Segment().pushBackString("CONFIRM").pushBackString("trump@whitehouse.gov").pushBackString("whatever"));
    mock.expectCall("confirmAddress(billg@microsoft.com,whatever,info here)");
    testee.callVoid(Segment().pushBackString("CONFIRM").pushBackString("billg@microsoft.com").pushBackString("whatever").pushBackString("info here"));

    mock.expectCall("requestAddress(batman)");
    testee.callVoid(Segment().pushBackString("REQUEST").pushBackString("batman"));

    mock.expectCall("runQueue()");
    testee.callVoid(Segment().pushBackString("RUNQUEUE"));

    // Variations
    mock.expectCall("startMessage(The-Template,no-id)");
    testee.callVoid(Segment().pushBackString("mail").pushBackString("The-Template"));

    // Additional commands
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("PING")), "PONG");
    TS_ASSERT(testee.callString(Segment().pushBackString("HELP")).size() > 0);

    // Errors
    Segment empty;    // g++-3.4 sees an invocation of a copy constructor if I construct this object in-place.
    TS_ASSERT_THROWS(testee.callVoid(empty), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("MAIL")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("MAIL").pushBackString("a").pushBackString("b").pushBackString("c")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("HUH")), std::exception);

    mock.checkFinish();
}

/** Test roundtrip. */
void
TestServerInterfaceMailQueueServer::testRoundtrip()
{
    MailQueueMock mock("testRoundtrip");
    server::interface::MailQueueServer level1(mock);
    server::interface::MailQueueClient level2(level1);
    server::interface::MailQueueServer level3(level2);
    server::interface::MailQueueClient level4(level3);

    mock.expectCall("startMessage(t,no-id)");
    level4.startMessage("t", afl::base::Nothing);
    mock.expectCall("startMessage(tt,u)");
    level4.startMessage("tt", String_t("u"));

    mock.expectCall("addParameter(p,v)");
    level4.addParameter("p", "v");

    mock.expectCall("addAttachment(a)");
    level4.addAttachment("a");

    mock.expectCall("send(1,2,3,4,5)");
    String_t rxs[] = { "1", "2", "3", "4", "5" };
    level4.send(rxs);
    mock.expectCall("send()");
    level4.send(afl::base::Memory<const String_t>());

    mock.expectCall("cancelMessage(q)");
    level4.cancelMessage("q");

    mock.expectCall("confirmAddress(a,k,no-info)");
    level4.confirmAddress("a", "k", afl::base::Nothing);
    mock.expectCall("confirmAddress(a,k,47)");
    level4.confirmAddress("a", "k", String_t("47"));

    mock.expectCall("requestAddress(u)");
    level4.requestAddress("u");

    mock.expectCall("runQueue()");
    level4.runQueue();

    mock.checkFinish();
}

