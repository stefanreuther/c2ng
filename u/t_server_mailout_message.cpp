/**
  *  \file u/t_server_mailout_message.cpp
  *  \brief Test for server::mailout::Message
  */

#include "server/mailout/message.hpp"

#include "t_server_mailout.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/integersetkey.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/stringlistkey.hpp"
#include "afl/net/redis/stringsetkey.hpp"
#include "afl/net/redis/subtree.hpp"
#include "server/mailout/root.hpp"

using afl::net::redis::HashKey;
using afl::net::redis::IntegerSetKey;
using afl::net::redis::StringListKey;
using afl::net::redis::StringSetKey;
using afl::net::redis::Subtree;

/** Test database access. */
void
TestServerMailoutMessage::testDatabase()
{
    // Environment
    afl::net::redis::InternalDatabase db;
    server::mailout::Root root(db, server::mailout::Configuration());

    // Set up
    server::mailout::Message testee(root, 54, server::mailout::Message::Preparing);
    TS_ASSERT_EQUALS(testee.getId(), 54);

    // Set properties of message
    testee.templateName().set("tp");
    testee.uniqueId().set("uid");
    testee.arguments().stringField("a1").set("v1");
    testee.attachments().pushBack("att");
    testee.receivers().add("r");
    testee.expireTime().set(1984);

    // Verify properties
    TS_ASSERT_EQUALS(HashKey(db, "mqueue:msg:54:data").stringField("template").get(), "tp");
    TS_ASSERT_EQUALS(HashKey(db, "mqueue:msg:54:data").stringField("uniqid").get(), "uid");
    TS_ASSERT_EQUALS(HashKey(db, "mqueue:msg:54:args").stringField("a1").get(), "v1");
    TS_ASSERT_EQUALS(StringListKey(db, "mqueue:msg:54:attach")[0], "att");
    TS_ASSERT(StringSetKey(db, "mqueue:msg:54:to").contains("r"));
    TS_ASSERT_EQUALS(HashKey(db, "mqueue:msg:54:data").intField("expire").get(), 1984);
}

/** Test remove(). */
void
TestServerMailoutMessage::testRemove()
{
    // Environment
    afl::net::redis::InternalDatabase db;
    server::mailout::Root root(db, server::mailout::Configuration());

    // Set up
    server::mailout::Message testee(root, 27, server::mailout::Message::Preparing);
    testee.templateName().set("tp");
    testee.attachments().pushBack("att");

    // Verify that this hit the database
    {
        afl::data::StringList_t keys;
        Subtree(db, "mqueue:").getKeyNames(keys);
        TS_ASSERT(keys.size() > 0);
    }

    // Delete the message
    testee.remove();

    // Database must now be empty
    {
        afl::data::StringList_t keys;
        Subtree(db, "mqueue:").getKeyNames(keys);
        TS_ASSERT_EQUALS(keys.size(), 0U);
    }
}

/** Test send(). */
void
TestServerMailoutMessage::testSend()
{
    // Environment
    afl::net::redis::InternalDatabase db;
    server::mailout::Root root(db, server::mailout::Configuration());

    // Set up
    server::mailout::Message testee(root, 92, server::mailout::Message::Preparing);
    testee.templateName().set("tp");
    testee.uniqueId().set("zx");
    testee.receivers().add("r");
    IntegerSetKey(db, "mqueue:preparing").add(92);

    // Send
    testee.send();

    // Verify
    TS_ASSERT_EQUALS(HashKey(db, "mqueue:uniqid").intField("zx").get(), 92);
    TS_ASSERT(IntegerSetKey(db, "mqueue:sending").contains(92));
    TS_ASSERT(!IntegerSetKey(db, "mqueue:preparing").contains(92));
}

