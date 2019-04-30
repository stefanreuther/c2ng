/**
  *  \file u/t_server_mailout_mailqueue.cpp
  *  \brief Test for server::mailout::MailQueue
  */

#include "server/mailout/mailqueue.hpp"

#include <stdexcept>
#include "t_server_mailout.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integersetkey.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "afl/net/redis/stringlistkey.hpp"
#include "afl/net/redis/stringsetkey.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "server/mailout/configuration.hpp"
#include "server/mailout/root.hpp"
#include "server/mailout/session.hpp"
#include "server/mailout/transmitter.hpp"

using afl::net::redis::HashKey;
using afl::net::redis::IntegerSetKey;
using afl::net::redis::StringListKey;
using afl::net::redis::StringSetKey;
using afl::net::redis::StringKey;
using afl::string::Format;

namespace {
    class TransmitterMock : public server::mailout::Transmitter, public afl::test::CallReceiver {
     public:
        TransmitterMock(afl::test::Assert a)
            : CallReceiver(a)
            { }
        virtual void send(int32_t messageId)
            { checkCall(Format("send(%d)", messageId)); }

        virtual void notifyAddress(String_t address)
            { checkCall(Format("notifyAddress(%s)", address)); }

        virtual void runQueue()
            { checkCall("runQueue()"); }
    };
}


/** Simple test. */
void
TestServerMailoutMailQueue::testIt()
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root root(db, server::mailout::Configuration());
    server::mailout::Session session;
    server::mailout::MailQueue testee(root, session);

    // Send message
    TS_ASSERT_THROWS_NOTHING(testee.startMessage("tpl", String_t("uid")));
    TS_ASSERT_THROWS_NOTHING(testee.addParameter("p", "v"));
    TS_ASSERT_THROWS_NOTHING(testee.addAttachment("http://"));
    String_t rx[] = {"r"};
    TS_ASSERT_THROWS_NOTHING(testee.send(rx));

    // Verify db content
    // - message
    TS_ASSERT_EQUALS(HashKey(db, "mqueue:msg:1:data").stringField("template").get(), "tpl");
    TS_ASSERT_EQUALS(HashKey(db, "mqueue:msg:1:data").stringField("uniqid").get(), "uid");
    TS_ASSERT_EQUALS(HashKey(db, "mqueue:msg:1:args").stringField("p").get(), "v");
    TS_ASSERT_EQUALS(StringListKey(db, "mqueue:msg:1:attach")[0], "http://");
    TS_ASSERT(StringSetKey(db, "mqueue:msg:1:to").contains("r"));
    // - set
    TS_ASSERT(IntegerSetKey(db, "mqueue:sending").contains(1));
    // - uniqid
    TS_ASSERT_EQUALS(HashKey(db, "mqueue:uniqid").intField("uid").get(), 1);
}

/** Test sequence error: message configuration command without starting a message */
void
TestServerMailoutMailQueue::testSequenceError()
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root root(db, server::mailout::Configuration());
    server::mailout::Session session;
    server::mailout::MailQueue testee(root, session);

    // These commands all fail, we have no message
    TS_ASSERT_THROWS(testee.addParameter("a", "b"), std::exception);
    TS_ASSERT_THROWS(testee.addAttachment("q"), std::exception);
    TS_ASSERT_THROWS(testee.send(afl::base::Nothing), std::exception);
}

/** Test sequence error: startMessage with active message. */
void
TestServerMailoutMailQueue::testSequenceError2()
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root root(db, server::mailout::Configuration());
    server::mailout::Session session;
    server::mailout::MailQueue testee(root, session);

    // Start message
    TS_ASSERT_THROWS_NOTHING(testee.startMessage("tpl", String_t("uid")));

    // Try to start another; must fail
    TS_ASSERT_THROWS(testee.startMessage("other", String_t("x")), std::exception);

    // The original message is still being prepared
    // - check db
    TS_ASSERT(IntegerSetKey(db, "mqueue:preparing").contains(1));
    TS_ASSERT_EQUALS(HashKey(db, "mqueue:msg:1:data").stringField("template").get(), "tpl");
    // - check state
    TS_ASSERT(session.currentMessage.get() != 0);
    TS_ASSERT_EQUALS(session.currentMessage->getId(), 1);
}

/** Test requesting email, success case. */
void
TestServerMailoutMailQueue::testRequest()
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Configuration config;
    config.baseUrl = "url/";
    server::mailout::Root root(db, config);
    server::mailout::Session session;
    server::mailout::MailQueue testee(root, session);

    // Expectation on transmitter
    TransmitterMock tx("testRequest");
    root.setTransmitter(&tx);
    tx.expectCall("send(1)");

    // Define a user
    StringSetKey(db, "user:all").add("1002");
    StringKey(db, "uid:tt").set("1002");
    StringKey(db, "user:1002:name").set("tt");
    HashKey(db, "user:1002:profile").stringField("email").set("u@h");

    // Request email confirmation
    TS_ASSERT_THROWS_NOTHING(testee.requestAddress("1002"));

    // This must have generated a confirmation request. Verify db.
    // - message
    TS_ASSERT_EQUALS(HashKey(db, "mqueue:msg:1:data").stringField("template").get(), "confirm");
    TS_ASSERT_EQUALS(HashKey(db, "mqueue:msg:1:data").stringField("uniqid").get(), "confirmation-u@h"); // FIXME: should this be confirmation-1002?
    TS_ASSERT_EQUALS(HashKey(db, "mqueue:msg:1:args").stringField("email").get(), "u@h");
    TS_ASSERT_EQUALS(HashKey(db, "mqueue:msg:1:args").stringField("user").get(), "tt");
    TS_ASSERT_EQUALS(HashKey(db, "mqueue:msg:1:args").stringField("confirmlink").get(), "url/confirm.cgi?key=MTAwMiyOCD5qhk5r83gESdGzGW9K&mail=u@h");
    TS_ASSERT_EQUALS(StringListKey(db, "mqueue:msg:1:attach").size(), 0);
    TS_ASSERT(StringSetKey(db, "mqueue:msg:1:to").contains("mail:u@h"));
    // - set
    TS_ASSERT(IntegerSetKey(db, "mqueue:sending").contains(1));
    // - uniqid
    TS_ASSERT_EQUALS(HashKey(db, "mqueue:uniqid").intField("confirmation-u@h").get(), 1);

    tx.checkFinish();
}

/** Test confirmAddress(), success case. */
void
TestServerMailoutMailQueue::testConfirmSuccess()
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root root(db, server::mailout::Configuration());
    server::mailout::Session session;
    server::mailout::MailQueue testee(root, session);

    // Confirm
    TS_ASSERT_THROWS_NOTHING(testee.confirmAddress("u@h", "MTAwMiyOCD5qhk5r83gESdGzGW9K", String_t("info")));

    // Verify
    TS_ASSERT_EQUALS(HashKey(db, "email:u@h:status").stringField("status/1002").get(), "c");
    TS_ASSERT_EQUALS(HashKey(db, "email:u@h:status").stringField("confirm/1002").get(), "info");

}

/** Test confirmAddress(), failure case. */
void
TestServerMailoutMailQueue::testConfirmFailure()
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root root(db, server::mailout::Configuration());
    server::mailout::Session session;
    server::mailout::MailQueue testee(root, session);

    // Confirm
    TS_ASSERT_THROWS(testee.confirmAddress("u@h", "MTAwMiyOCD5qhk5r83gESdGWRONG", String_t("info")), std::exception);
}

/** Test confirmAddress(), success case, with transmitter. */
void
TestServerMailoutMailQueue::testConfirmSuccessTransmit()
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root root(db, server::mailout::Configuration());
    server::mailout::Session session;
    server::mailout::MailQueue testee(root, session);
    TransmitterMock tx("testConfirmSuccessTransmit");
    root.setTransmitter(&tx);

    HashKey(db, "user:1002:profile").stringField("email").set("u@h");

    // Expect
    tx.expectCall("notifyAddress(u@h)");

    // Confirm
    TS_ASSERT_THROWS_NOTHING(testee.confirmAddress("u@h", "MTAwMiyOCD5qhk5r83gESdGzGW9K", String_t("info")));

    // Verify
    TS_ASSERT_EQUALS(HashKey(db, "email:u@h:status").stringField("status/1002").get(), "c");
    TS_ASSERT_EQUALS(HashKey(db, "email:u@h:status").stringField("confirm/1002").get(), "info");
    tx.checkFinish();

    // Also query status
    server::interface::MailQueue::UserStatus st = testee.getUserStatus("1002");
    TS_ASSERT_EQUALS(st.address, "u@h");
    TS_ASSERT_EQUALS(st.status, server::interface::MailQueue::Confirmed);
}

/** Test runQueue(), without transmitter. */
void
TestServerMailoutMailQueue::testRunQueue()
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root root(db, server::mailout::Configuration());
    server::mailout::Session session;
    server::mailout::MailQueue testee(root, session);

    TS_ASSERT_THROWS_NOTHING(testee.runQueue());
}

/** Test runQueue(), with transmitter. */
void
TestServerMailoutMailQueue::testRunQueueTransmitter()
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root root(db, server::mailout::Configuration());
    server::mailout::Session session;
    server::mailout::MailQueue testee(root, session);
    TransmitterMock tx("testRunQueueTransmitter");
    root.setTransmitter(&tx);

    tx.expectCall("runQueue()");
    TS_ASSERT_THROWS_NOTHING(testee.runQueue());
    tx.checkFinish();
}
