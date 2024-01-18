/**
  *  \file test/server/mailout/roottest.cpp
  *  \brief Test for server::mailout::Root
  */

#include "server/mailout/root.hpp"

#include <stdexcept>
#include <map>
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/integerkey.hpp"
#include "afl/net/redis/integersetkey.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "afl/net/redis/stringsetkey.hpp"
#include "afl/test/testrunner.hpp"
#include "server/mailout/configuration.hpp"
#include "server/mailout/message.hpp"
#include "server/mailout/transmitter.hpp"

using afl::net::redis::HashKey;
using afl::net::redis::IntegerField;
using afl::net::redis::IntegerKey;
using afl::net::redis::IntegerSetKey;
using afl::net::redis::StringKey;
using afl::net::redis::StringSetKey;

namespace {
    server::mailout::Configuration makeConfig()
    {
        server::mailout::Configuration config;
        config.confirmationKey = "1234";
        config.baseUrl = "web/";
        return config;
    }
}

/** Test allocateMessage(). */
AFL_TEST("server.mailout.Root:allocateMessage", a)
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root testee(db, makeConfig());
    IntegerKey(db, "mqueue:msg:id").set(42);

    // Allocate a message
    std::auto_ptr<server::mailout::Message> p(testee.allocateMessage());

    // Verify
    a.checkNonNull("01", p.get());
    a.checkEqual("02. getId", p->getId(), 43);
    a.checkEqual("03. db", IntegerKey(db, "mqueue:msg:id").get(), 43);
    a.check("04. db", IntegerSetKey(db, "mqueue:preparing").contains(43));
}

/** Test resolving a SMTP address, normal case.
    Must produce correct result. */
AFL_TEST("server.mailout.Root:resolveAddress:mail", a)
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root testee(db, makeConfig());

    String_t smtpAddress;
    String_t authUser;
    a.check("01. resolveAddress", testee.resolveAddress("mail:a@b", smtpAddress, authUser));
    a.checkEqual("02. smtpAddress", smtpAddress, "a@b");
    a.checkEqual("03. authUser", authUser, "anon");
}

/** Test resolving a SMTP address, error case (blocked).
    Must throw (hard failure). */
AFL_TEST("server.mailout.Root:resolveAddress:mail:blocked", a)
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root testee(db, makeConfig());
    HashKey(db, "email:x@y:status").stringField("status/anon").set("b");

    String_t smtpAddress;
    String_t authUser;
    AFL_CHECK_THROWS(a, testee.resolveAddress("mail:x@y", smtpAddress, authUser), std::exception);
}

/** Test resolving a user address, error case (no email).
    Must throw (hard failure). */
AFL_TEST("server.mailout.Root:resolveAddress:user:no-mail", a)
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root testee(db, makeConfig());

    String_t smtpAddress;
    String_t authUser;
    AFL_CHECK_THROWS(a, testee.resolveAddress("user:1009", smtpAddress, authUser), std::exception);
}

/** Test resolving a user address, unconfirmed email.
    Must return false (postpone), and queue a confirmation request. */
AFL_TEST("server.mailout.Root:resolveAddress:user:unconfirmed", a)
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root testee(db, makeConfig());
    HashKey(db, "user:1009:profile").stringField("email").set("ad@re.ss");

    String_t smtpAddress;
    String_t authUser;
    a.checkEqual("01. resolveAddress", testee.resolveAddress("user:1009", smtpAddress, authUser), false);

    // Verify that status is now requested
    a.checkEqual("11. status r", HashKey(db, "email:ad@re.ss:status").stringField("status/1009").get(), "r");

    // Verify that it queues a confirmation mail
    a.checkEqual("21. db mail", IntegerKey(db, "mqueue:msg:id").get(), 1);
    a.checkEqual("22. db mail", HashKey(db, "mqueue:msg:1:data").stringField("template").get(), "confirm");
    a.checkEqual("23. db mail", HashKey(db, "mqueue:msg:1:args").stringField("email").get(), "ad@re.ss");
    a.checkEqual("24. db mail", HashKey(db, "mqueue:msg:1:args").stringField("confirmlink").get(), "web/confirm.cgi?key=MTAwOSwFD4jm%2BqJtd7hL3HdHW%2BlO&mail=ad@re.ss");
    a.check("25. db mail", StringSetKey(db, "mqueue:msg:1:to").contains("mail:ad@re.ss"));
}

/** Test resolving a user address, requested confirmation.
    Must return false (postpone) but not queue a confirmation request. */
AFL_TEST("server.mailout.Root:resolveAddress:user:requested", a)
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root testee(db, makeConfig());
    HashKey(db, "user:1009:profile").stringField("email").set("ad@re.ss");
    HashKey(db, "email:ad@re.ss:status").stringField("status/1009").set("r");
    HashKey(db, "email:ad@re.ss:status").intField("expire/1009").set(testee.getCurrentTime() + 10);

    String_t smtpAddress;
    String_t authUser;
    a.checkEqual("01. resolveAddress", testee.resolveAddress("user:1009", smtpAddress, authUser), false);

    // Verify that status is still requested
    a.checkEqual("11. status r", HashKey(db, "email:ad@re.ss:status").stringField("status/1009").get(), "r");

    // Verify that it does not queue a confirmation mail
    a.checkEqual("21. db mail", IntegerKey(db, "mqueue:msg:id").get(), 0);
}

/** Test resolving a user address, expired confirmation.
    Must return false (postpone) and queue a new confirmation request. */
AFL_TEST("server.mailout.Root:resolveAddress:user:expired", a)
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root testee(db, makeConfig());
    HashKey(db, "user:1009:profile").stringField("email").set("ad@re.ss");
    HashKey(db, "email:ad@re.ss:status").stringField("status/1009").set("r");
    HashKey(db, "email:ad@re.ss:status").intField("expire/1009").set(testee.getCurrentTime() - 10);

    String_t smtpAddress;
    String_t authUser;
    a.checkEqual("01. resolveAddress", testee.resolveAddress("user:1009", smtpAddress, authUser), false);

    // Verify that status is still requested with updated expiration time
    a.checkEqual("11. status r", HashKey(db, "email:ad@re.ss:status").stringField("status/1009").get(), "r");
    a.check("12. time", HashKey(db, "email:ad@re.ss:status").intField("expire/1009").get() > testee.getCurrentTime());

    // Verify that it queues a confirmation mail
    a.checkEqual("21. db mail", IntegerKey(db, "mqueue:msg:id").get(), 1);
    a.checkEqual("22. db mail", HashKey(db, "mqueue:msg:1:data").stringField("template").get(), "confirm");
    a.checkEqual("23. db mail", HashKey(db, "mqueue:msg:1:args").stringField("email").get(), "ad@re.ss");
}

/** Test resolving a user address, confirmed.
    Must return true (proceed) and not queue anything. */
AFL_TEST("server.mailout.Root:resolveAddress:user:confirmed", a)
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root testee(db, makeConfig());
    HashKey(db, "user:1009:profile").stringField("email").set("ad@re.ss");
    HashKey(db, "email:ad@re.ss:status").stringField("status/1009").set("c");
    HashKey(db, "email:ad@re.ss:status").intField("expire/1009").set(testee.getCurrentTime() - 10);

    String_t smtpAddress;
    String_t authUser;
    a.checkEqual("01. resolveAddress", testee.resolveAddress("user:1009", smtpAddress, authUser), true);

    // Verify that status is still confirmed
    a.checkEqual("11. status c", HashKey(db, "email:ad@re.ss:status").stringField("status/1009").get(), "c");

    // Verify that it does not queue a confirmation mail
    a.checkEqual("21. db mail", IntegerKey(db, "mqueue:msg:id").get(), 0);
}

/** Test resolving a user address, blocked.
    Must throw (hard failure). */
AFL_TEST("server.mailout.Root:resolveAddress:user:blocked", a)
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root testee(db, makeConfig());
    HashKey(db, "user:1009:profile").stringField("email").set("ad@re.ss");
    HashKey(db, "email:ad@re.ss:status").stringField("status/1009").set("b");

    String_t smtpAddress;
    String_t authUser;
    AFL_CHECK_THROWS(a, testee.resolveAddress("user:1009", smtpAddress, authUser), std::exception);
}

/** Test confirmMail(), success case. */
AFL_TEST("server.mailout.Root:confirmMail:success", a)
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root testee(db, makeConfig());

    a.check("01. confirmMail", testee.confirmMail("ad@re.ss", "MTAwOSwFD4jm+qJtd7hL3HdHW+lO", "i"));
    a.checkEqual("02. db", HashKey(db, "email:ad@re.ss:status").stringField("status/1009").get(), "c");
    a.checkEqual("03. db", HashKey(db, "email:ad@re.ss:status").stringField("confirm/1009").get(), "i");
}

/** Test confirmMail(), failure cases. */
AFL_TEST("server.mailout.Root:confirmMail:fail", a)
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root testee(db, makeConfig());

    HashKey(db, "email:ad@re.ss:status").stringField("status/1009").set("r");
    HashKey(db, "email:ad@re.ss:status").stringField("status/1024").set("r");

    // Forgot to urldecode
    a.check("01. not decoded", !testee.confirmMail("ad@re.ss", "MTAwOSwFD4jm%2bqJtd7hL3HdHW%2blO", "i"));

    // Case problem
    a.check("11. case", !testee.confirmMail("ad@re.ss", "MTAWOSWFD4JM+QJTD7HL3HDHW+LO", "i"));

    // Padding
    a.check("21. padding", !testee.confirmMail("ad@re.ss", "MTAwOSwFD4jm+qJtd7hL3HdHW+lO==", "i"));

    // Syntax
    a.check("31. syntax", !testee.confirmMail("ad@re.ss", "", "i"));
    a.check("32. syntax", !testee.confirmMail("ad@re.ss", "99999", "i"));
    a.check("33. syntax", !testee.confirmMail("ad@re.ss", "MTAWOSWFD4JM+QJTD7HL3HDHW+LOMTAWOS", "i"));

    // User mismatch (specified user 1009, but signed user 1024, i.e. simple spoofing)
    a.check("41. user mismatch", !testee.confirmMail("ad@re.ss", "MTAwOSy///IZYhztobfFurWpCjTZ", "i"));

    // Address mismatch
    a.check("51. address mismatch", !testee.confirmMail("ad1@re.ss", "MTAwOSwFD4jm+qJtd7hL3HdHW+lO", "i"));

    // No change
    a.checkEqual("61. db", HashKey(db, "email:ad@re.ss:status").stringField("status/1009").get(), "r");
    a.checkEqual("62. db", HashKey(db, "email:ad@re.ss:status").stringField("status/1024").get(), "r");
}

/** Test prepareQueues(). */
AFL_TEST("server.mailout.Root:prepareQueues", a)
{
    class TransmitterMock : public server::mailout::Transmitter {
     public:
        virtual void send(int32_t messageId)
            { ++mids[messageId]; }
        virtual void notifyAddress(String_t /*address*/)
            { throw std::runtime_error("notifyAddress not expected"); }
        virtual void runQueue()
            { }
        std::map<int32_t, int32_t> mids;
    };

    afl::net::redis::InternalDatabase db;
    server::mailout::Root testee(db, makeConfig());
    TransmitterMock tx;
    testee.setTransmitter(&tx);

    // Create a few messages
    HashKey(db, "mqueue:msg:9:data").stringField("template").set("t9");
    IntegerSetKey(db, "mqueue:sending").add(9);

    HashKey(db, "mqueue:msg:12:data").stringField("template").set("t12");
    IntegerSetKey(db, "mqueue:preparing").add(12);

    HashKey(db, "mqueue:msg:54:data").stringField("template").set("t54");
    IntegerSetKey(db, "mqueue:preparing").add(54);

    HashKey(db, "mqueue:msg:84:data").stringField("template").set("t84");
    IntegerSetKey(db, "mqueue:sending").add(84);

    // Call
    testee.prepareQueues();

    // Verify
    a.checkEqual("11", tx.mids.size(), 2U);
    a.checkEqual("12", tx.mids[9], 1);
    a.checkEqual("13", tx.mids[84], 1);
}

/** Test getUserStatus(), regular case. */
AFL_TEST("server.mailout.Root:getUserStatus:normal", a)
{
    using server::interface::MailQueue;

    afl::net::redis::InternalDatabase db;
    server::mailout::Root testee(db, makeConfig());

    HashKey(db, "user:1009:profile").stringField("email").set("ad@re.ss");
    HashKey(db, "email:ad@re.ss:status").stringField("status/1009").set("c");
    HashKey(db, "email:ad@re.ss:status").intField("expire/1009").set(testee.getCurrentTime() - 10);

    MailQueue::UserStatus st = testee.getUserStatus("1009");
    a.checkEqual("01. address", st.address, "ad@re.ss");
    a.checkEqual("02. status", st.status, MailQueue::Confirmed);
}

/** Test getUserStatus(), empty database (aka user has no email). */
AFL_TEST("server.mailout.Root:getUserStatus:empty", a)
{
    using server::interface::MailQueue;

    afl::net::redis::InternalDatabase db;
    server::mailout::Root testee(db, makeConfig());

    MailQueue::UserStatus st = testee.getUserStatus("1009");
    a.checkEqual("01. address", st.address, "");
    a.checkEqual("02. status", st.status, MailQueue::NotSet);
}

/** Test getUserStatus(), half-empty database (aka user created but not yet requested). */
AFL_TEST("server.mailout.Root:getUserStatus:unconfirmed", a)
{
    using server::interface::MailQueue;

    afl::net::redis::InternalDatabase db;
    server::mailout::Root testee(db, makeConfig());

    HashKey(db, "user:1009:profile").stringField("email").set("ad@re.ss");

    MailQueue::UserStatus st = testee.getUserStatus("1009");
    a.checkEqual("01. address", st.address, "ad@re.ss");
    a.checkEqual("02. status", st.status, MailQueue::Unconfirmed);
}

/** Test cleanupUniqueIdMap(). */
AFL_TEST("server.mailout.Root:cleanupUniqueIdMap", a)
{
    // Database content (derived from an actual planetscentral.com state)
    afl::net::redis::InternalDatabase db;
    IntegerKey(db, "mqueue:msg:id").set(44848);
    IntegerSetKey(db, "mqueue:sending").add(43218);
    HashKey(db, "mqueue:uniqid").intField("confirmation-2588mike").set(12646);
    HashKey(db, "mqueue:uniqid").intField("confirmation-2878828247").set(31072);
    HashKey(db, "mqueue:uniqid").intField("confirmation-4e7dfdg").set(41310);
    HashKey(db, "mqueue:uniqid").intField("confirmation-Alexander").set(2367);
    HashKey(db, "mqueue:uniqid").intField("confirmation-Bernd").set(261);
    HashKey(db, "mqueue:uniqid").intField("confirmation-Bjoern").set(24792);
    HashKey(db, "mqueue:uniqid").intField("confirmation-Carsten").set(24);
    HashKey(db, "mqueue:uniqid").intField("post-3003").set(43219);

    // Testee
    server::mailout::Root testee(db, makeConfig());
    testee.cleanupUniqueIdMap();

    a.checkEqual("01", HashKey(db, "mqueue:uniqid").size(), 0);
}
