/**
  *  \file u/t_server_mailout_root.cpp
  *  \brief Test for server::mailout::Root
  */

#include <map>
#include "server/mailout/root.hpp"

#include "t_server_mailout.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/integerkey.hpp"
#include "afl/net/redis/integersetkey.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "afl/net/redis/stringsetkey.hpp"
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
void
TestServerMailoutRoot::testAllocateMessage()
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root testee(db, makeConfig());
    IntegerKey(db, "mqueue:msg:id").set(42);

    // Allocate a message
    std::auto_ptr<server::mailout::Message> p(testee.allocateMessage());

    // Verify
    TS_ASSERT(p.get() != 0);
    TS_ASSERT_EQUALS(p->getId(), 43);
    TS_ASSERT_EQUALS(IntegerKey(db, "mqueue:msg:id").get(), 43);
    TS_ASSERT(IntegerSetKey(db, "mqueue:preparing").contains(43));
}

/** Test resolving a SMTP address, normal case.
    Must produce correct result. */
void
TestServerMailoutRoot::testResolveMail()
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root testee(db, makeConfig());

    String_t smtpAddress;
    String_t authUser;
    TS_ASSERT(testee.resolveAddress("mail:a@b", smtpAddress, authUser));
    TS_ASSERT_EQUALS(smtpAddress, "a@b");
    TS_ASSERT_EQUALS(authUser, "anon");
}

/** Test resolving a SMTP address, error case (blocked).
    Must throw (hard failure). */
void
TestServerMailoutRoot::testResolveMailBlocked()
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root testee(db, makeConfig());
    HashKey(db, "email:x@y:status").stringField("status/anon").set("b");
    
    String_t smtpAddress;
    String_t authUser;
    TS_ASSERT_THROWS(testee.resolveAddress("mail:x@y", smtpAddress, authUser), std::exception);
}

/** Test resolving a user address, error case (no email).
    Must throw (hard failure). */
void
TestServerMailoutRoot::testResolveUserNoMail()
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root testee(db, makeConfig());

    String_t smtpAddress;
    String_t authUser;
    TS_ASSERT_THROWS(testee.resolveAddress("user:1009", smtpAddress, authUser), std::exception);
}

/** Test resolving a user address, unconfirmed email.
    Must return false (postpone), and queue a confirmation request. */
void
TestServerMailoutRoot::testResolveUserUnconfirmed()
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root testee(db, makeConfig());
    HashKey(db, "user:1009:profile").stringField("email").set("ad@re.ss");

    String_t smtpAddress;
    String_t authUser;
    TS_ASSERT_EQUALS(testee.resolveAddress("user:1009", smtpAddress, authUser), false);

    // Verify that status is now requested
    TS_ASSERT_EQUALS(HashKey(db, "email:ad@re.ss:status").stringField("status/1009").get(), "r");

    // Verify that it queues a confirmation mail
    TS_ASSERT_EQUALS(IntegerKey(db, "mqueue:msg:id").get(), 1);
    TS_ASSERT_EQUALS(HashKey(db, "mqueue:msg:1:data").stringField("template").get(), "confirm");
    TS_ASSERT_EQUALS(HashKey(db, "mqueue:msg:1:args").stringField("email").get(), "ad@re.ss");
    TS_ASSERT_EQUALS(HashKey(db, "mqueue:msg:1:args").stringField("confirmlink").get(), "web/confirm.cgi?key=MTAwOSwFD4jm%2BqJtd7hL3HdHW%2BlO&mail=ad@re.ss");
    TS_ASSERT(StringSetKey(db, "mqueue:msg:1:to").contains("mail:ad@re.ss"));
}

/** Test resolving a user address, requested confirmation.
    Must return false (postpone) but not queue a confirmation request. */
void
TestServerMailoutRoot::testResolveUserRequested()
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root testee(db, makeConfig());
    HashKey(db, "user:1009:profile").stringField("email").set("ad@re.ss");
    HashKey(db, "email:ad@re.ss:status").stringField("status/1009").set("r");
    HashKey(db, "email:ad@re.ss:status").intField("expire/1009").set(testee.getCurrentTime() + 10);

    String_t smtpAddress;
    String_t authUser;
    TS_ASSERT_EQUALS(testee.resolveAddress("user:1009", smtpAddress, authUser), false);

    // Verify that status is still requested
    TS_ASSERT_EQUALS(HashKey(db, "email:ad@re.ss:status").stringField("status/1009").get(), "r");

    // Verify that it does not queue a confirmation mail
    TS_ASSERT_EQUALS(IntegerKey(db, "mqueue:msg:id").get(), 0);
}

/** Test resolving a user address, expired confirmation.
    Must return false (postpone) and queue a new confirmation request. */
void
TestServerMailoutRoot::testResolveUserExpired()
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root testee(db, makeConfig());
    HashKey(db, "user:1009:profile").stringField("email").set("ad@re.ss");
    HashKey(db, "email:ad@re.ss:status").stringField("status/1009").set("r");
    HashKey(db, "email:ad@re.ss:status").intField("expire/1009").set(testee.getCurrentTime() - 10);

    String_t smtpAddress;
    String_t authUser;
    TS_ASSERT_EQUALS(testee.resolveAddress("user:1009", smtpAddress, authUser), false);

    // Verify that status is still requested with updated expiration time
    TS_ASSERT_EQUALS(HashKey(db, "email:ad@re.ss:status").stringField("status/1009").get(), "r");
    TS_ASSERT(HashKey(db, "email:ad@re.ss:status").intField("expire/1009").get() > testee.getCurrentTime());

    // Verify that it queues a confirmation mail
    TS_ASSERT_EQUALS(IntegerKey(db, "mqueue:msg:id").get(), 1);
    TS_ASSERT_EQUALS(HashKey(db, "mqueue:msg:1:data").stringField("template").get(), "confirm");
    TS_ASSERT_EQUALS(HashKey(db, "mqueue:msg:1:args").stringField("email").get(), "ad@re.ss");
}

/** Test resolving a user address, confirmed.
    Must return true (proceed) and not queue anything. */
void
TestServerMailoutRoot::testResolveUserConfirmed()
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root testee(db, makeConfig());
    HashKey(db, "user:1009:profile").stringField("email").set("ad@re.ss");
    HashKey(db, "email:ad@re.ss:status").stringField("status/1009").set("c");
    HashKey(db, "email:ad@re.ss:status").intField("expire/1009").set(testee.getCurrentTime() - 10);

    String_t smtpAddress;
    String_t authUser;
    TS_ASSERT_EQUALS(testee.resolveAddress("user:1009", smtpAddress, authUser), true);

    // Verify that status is still confirmed
    TS_ASSERT_EQUALS(HashKey(db, "email:ad@re.ss:status").stringField("status/1009").get(), "c");

    // Verify that it does not queue a confirmation mail
    TS_ASSERT_EQUALS(IntegerKey(db, "mqueue:msg:id").get(), 0);
}

/** Test resolving a user address, blocked.
    Must throw (hard failure). */
void
TestServerMailoutRoot::testResolveUserBlocked()
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root testee(db, makeConfig());
    HashKey(db, "user:1009:profile").stringField("email").set("ad@re.ss");
    HashKey(db, "email:ad@re.ss:status").stringField("status/1009").set("b");

    String_t smtpAddress;
    String_t authUser;
    TS_ASSERT_THROWS(testee.resolveAddress("user:1009", smtpAddress, authUser), std::exception);
}

/** Test confirmMail(), success case. */
void
TestServerMailoutRoot::testConfirmMail()
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root testee(db, makeConfig());

    TS_ASSERT(testee.confirmMail("ad@re.ss", "MTAwOSwFD4jm+qJtd7hL3HdHW+lO", "i"));
    TS_ASSERT_EQUALS(HashKey(db, "email:ad@re.ss:status").stringField("status/1009").get(), "c");
    TS_ASSERT_EQUALS(HashKey(db, "email:ad@re.ss:status").stringField("confirm/1009").get(), "i");
}

/** Test confirmMail(), failure cases. */
void
TestServerMailoutRoot::testConfirmMailFail()
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root testee(db, makeConfig());

    HashKey(db, "email:ad@re.ss:status").stringField("status/1009").set("r");
    HashKey(db, "email:ad@re.ss:status").stringField("status/1024").set("r");

    // Forgot to urldecode
    TS_ASSERT(!testee.confirmMail("ad@re.ss", "MTAwOSwFD4jm%2bqJtd7hL3HdHW%2blO", "i"));

    // Case problem
    TS_ASSERT(!testee.confirmMail("ad@re.ss", "MTAWOSWFD4JM+QJTD7HL3HDHW+LO", "i"));

    // Padding
    TS_ASSERT(!testee.confirmMail("ad@re.ss", "MTAwOSwFD4jm+qJtd7hL3HdHW+lO==", "i"));

    // Syntax
    TS_ASSERT(!testee.confirmMail("ad@re.ss", "", "i"));
    TS_ASSERT(!testee.confirmMail("ad@re.ss", "99999", "i"));
    TS_ASSERT(!testee.confirmMail("ad@re.ss", "MTAWOSWFD4JM+QJTD7HL3HDHW+LOMTAWOS", "i"));

    // User mismatch (specified user 1009, but signed user 1024, i.e. simple spoofing)
    TS_ASSERT(!testee.confirmMail("ad@re.ss", "MTAwOSy///IZYhztobfFurWpCjTZ", "i"));

    // Address mismatch
    TS_ASSERT(!testee.confirmMail("ad1@re.ss", "MTAwOSwFD4jm+qJtd7hL3HdHW+lO", "i"));

    // No change
    TS_ASSERT_EQUALS(HashKey(db, "email:ad@re.ss:status").stringField("status/1009").get(), "r");
    TS_ASSERT_EQUALS(HashKey(db, "email:ad@re.ss:status").stringField("status/1024").get(), "r");
}

/** Test prepareQueues(). */
void
TestServerMailoutRoot::testPrepareQueue()
{
    class TransmitterMock : public server::mailout::Transmitter {
     public:
        virtual void send(int32_t messageId)
            { ++mids[messageId]; }
        virtual void notifyAddress(String_t /*address*/)
            { TS_ASSERT(0); }
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
    TS_ASSERT_EQUALS(tx.mids.size(), 2U);
    TS_ASSERT_EQUALS(tx.mids[9], 1);
    TS_ASSERT_EQUALS(tx.mids[84], 1);
}

