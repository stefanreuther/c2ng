/**
  *  \file test/server/mailout/mailqueuetest.cpp
  *  \brief Test for server::mailout::MailQueue
  */

#include "server/mailout/mailqueue.hpp"

#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integersetkey.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "afl/net/redis/stringlistkey.hpp"
#include "afl/net/redis/stringsetkey.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"
#include "server/mailout/configuration.hpp"
#include "server/mailout/root.hpp"
#include "server/mailout/session.hpp"
#include "server/mailout/transmitter.hpp"
#include <stdexcept>

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
AFL_TEST("server.mailout.MailQueue:basics", a)
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root root(db, server::mailout::Configuration());
    server::mailout::Session session;
    server::mailout::MailQueue testee(root, session);

    // Send message
    AFL_CHECK_SUCCEEDS(a("01. startMessage"),  testee.startMessage("tpl", String_t("uid")));
    AFL_CHECK_SUCCEEDS(a("02. addParameter"),  testee.addParameter("p", "v"));
    AFL_CHECK_SUCCEEDS(a("03. addAttachment"), testee.addAttachment("http://"));
    String_t rx[] = {"r"};
    AFL_CHECK_SUCCEEDS(a("04. send"), testee.send(rx));

    // Verify db content
    // - message
    a.checkEqual("11. db msg", HashKey(db, "mqueue:msg:1:data").stringField("template").get(), "tpl");
    a.checkEqual("12. db msg", HashKey(db, "mqueue:msg:1:data").stringField("uniqid").get(), "uid");
    a.checkEqual("13. db msg", HashKey(db, "mqueue:msg:1:args").stringField("p").get(), "v");
    a.checkEqual("14. db msg", StringListKey(db, "mqueue:msg:1:attach")[0], "http://");
    a.check     ("15. db msg", StringSetKey(db, "mqueue:msg:1:to").contains("r"));
    // - set
    a.check     ("16. db set", IntegerSetKey(db, "mqueue:sending").contains(1));
    // - uniqid
    a.checkEqual("17. db id",  HashKey(db, "mqueue:uniqid").intField("uid").get(), 1);
}

/** Test sequence error: message configuration command without starting a message */
AFL_TEST("server.mailout.MailQueue:error:no-start", a)
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root root(db, server::mailout::Configuration());
    server::mailout::Session session;
    server::mailout::MailQueue testee(root, session);

    // These commands all fail, we have no message
    AFL_CHECK_THROWS(a("01. addParameter"),  testee.addParameter("a", "b"), std::exception);
    AFL_CHECK_THROWS(a("02. addAttachment"), testee.addAttachment("q"), std::exception);
    AFL_CHECK_THROWS(a("03. send"),          testee.send(afl::base::Nothing), std::exception);
}

/** Test sequence error: startMessage with active message. */
AFL_TEST("server.mailout.MailQueue:error:double-start", a)
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root root(db, server::mailout::Configuration());
    server::mailout::Session session;
    server::mailout::MailQueue testee(root, session);

    // Start message
    AFL_CHECK_SUCCEEDS(a("01. startMessage"), testee.startMessage("tpl", String_t("uid")));

    // Try to start another; must fail
    AFL_CHECK_THROWS(a("11. startMessage"), testee.startMessage("other", String_t("x")), std::exception);

    // The original message is still being prepared
    // - check db
    a.check     ("21. db", IntegerSetKey(db, "mqueue:preparing").contains(1));
    a.checkEqual("22. db", HashKey(db, "mqueue:msg:1:data").stringField("template").get(), "tpl");
    // - check state
    a.checkNonNull("23. currentMessage", session.currentMessage.get());
    a.checkEqual("24. currentMessage", session.currentMessage->getId(), 1);
}

/** Test requesting email, success case. */
AFL_TEST("server.mailout.MailQueue:requestAddress", a)
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Configuration config;
    config.baseUrl = "url/";
    server::mailout::Root root(db, config);
    server::mailout::Session session;
    server::mailout::MailQueue testee(root, session);

    // Expectation on transmitter
    TransmitterMock tx(a);
    root.setTransmitter(&tx);
    tx.expectCall("send(1)");

    // Define a user
    StringSetKey(db, "user:all").add("1002");
    StringKey(db, "uid:tt").set("1002");
    StringKey(db, "user:1002:name").set("tt");
    HashKey(db, "user:1002:profile").stringField("email").set("u@h");

    // Request email confirmation
    AFL_CHECK_SUCCEEDS(a("01. requestAddress"), testee.requestAddress("1002"));

    // This must have generated a confirmation request. Verify db.
    // - message
    a.checkEqual("11. db msg", HashKey(db, "mqueue:msg:1:data").stringField("template").get(), "confirm");
    a.checkEqual("12. db msg", HashKey(db, "mqueue:msg:1:data").stringField("uniqid").get(), "confirmation-u@h"); // FIXME: should this be confirmation-1002?
    a.checkEqual("13. db msg", HashKey(db, "mqueue:msg:1:args").stringField("email").get(), "u@h");
    a.checkEqual("14. db msg", HashKey(db, "mqueue:msg:1:args").stringField("user").get(), "tt");
    a.checkEqual("15. db msg", HashKey(db, "mqueue:msg:1:args").stringField("confirmlink").get(), "url/confirm.cgi?key=MTAwMiyOCD5qhk5r83gESdGzGW9K&mail=u@h");
    a.checkEqual("16. db msg", StringListKey(db, "mqueue:msg:1:attach").size(), 0);
    a.check("17. db msg", StringSetKey(db, "mqueue:msg:1:to").contains("mail:u@h"));
    // - set
    a.check("18. db set", IntegerSetKey(db, "mqueue:sending").contains(1));
    // - uniqid
    a.checkEqual("19. db id", HashKey(db, "mqueue:uniqid").intField("confirmation-u@h").get(), 1);

    tx.checkFinish();
}

/** Test confirmAddress(), success case. */
AFL_TEST("server.mailout.MailQueue:confirmAddress:success", a)
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root root(db, server::mailout::Configuration());
    server::mailout::Session session;
    server::mailout::MailQueue testee(root, session);

    // Confirm
    AFL_CHECK_SUCCEEDS(a("01. confirmAddress"), testee.confirmAddress("u@h", "MTAwMiyOCD5qhk5r83gESdGzGW9K", String_t("info")));

    // Verify
    a.checkEqual("11. db", HashKey(db, "email:u@h:status").stringField("status/1002").get(), "c");
    a.checkEqual("12. db", HashKey(db, "email:u@h:status").stringField("confirm/1002").get(), "info");
}

/** Test confirmAddress(), failure case. */
AFL_TEST("server.mailout.MailQueue:confirmAddress:failure", a)
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root root(db, server::mailout::Configuration());
    server::mailout::Session session;
    server::mailout::MailQueue testee(root, session);

    // Confirm
    AFL_CHECK_THROWS(a("01. confirmAddress"), testee.confirmAddress("u@h", "MTAwMiyOCD5qhk5r83gESdGWRONG", String_t("info")), std::exception);
}

/** Test confirmAddress(), success case, with transmitter. */
AFL_TEST("server.mailout.MailQueue:confirmAddress:success:transmitter", a)
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root root(db, server::mailout::Configuration());
    server::mailout::Session session;
    server::mailout::MailQueue testee(root, session);
    TransmitterMock tx(a);
    root.setTransmitter(&tx);

    HashKey(db, "user:1002:profile").stringField("email").set("u@h");

    // Expect
    tx.expectCall("notifyAddress(u@h)");

    // Confirm
    AFL_CHECK_SUCCEEDS(a("01. confirmAddress"), testee.confirmAddress("u@h", "MTAwMiyOCD5qhk5r83gESdGzGW9K", String_t("info")));

    // Verify
    a.checkEqual("11. db", HashKey(db, "email:u@h:status").stringField("status/1002").get(), "c");
    a.checkEqual("12. db", HashKey(db, "email:u@h:status").stringField("confirm/1002").get(), "info");
    tx.checkFinish();

    // Also query status
    server::interface::MailQueue::UserStatus st = testee.getUserStatus("1002");
    a.checkEqual("21. address", st.address, "u@h");
    a.checkEqual("22. status", st.status, server::interface::MailQueue::Confirmed);
}

/** Test runQueue(), without transmitter. */
AFL_TEST("server.mailout.MailQueue:runQueue:no-transmitter", a)
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root root(db, server::mailout::Configuration());
    server::mailout::Session session;
    server::mailout::MailQueue testee(root, session);

    AFL_CHECK_SUCCEEDS(a, testee.runQueue());
}

/** Test runQueue(), with transmitter. */
AFL_TEST("server.mailout.MailQueue:runQueue:transmitter", a)
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root root(db, server::mailout::Configuration());
    server::mailout::Session session;
    server::mailout::MailQueue testee(root, session);
    TransmitterMock tx(a);
    root.setTransmitter(&tx);

    tx.expectCall("runQueue()");
    AFL_CHECK_SUCCEEDS(a, testee.runQueue());
    tx.checkFinish();
}
