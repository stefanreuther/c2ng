/**
  *  \file test/server/mailout/messagetest.cpp
  *  \brief Test for server::mailout::Message
  */

#include "server/mailout/message.hpp"

#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/integersetkey.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/stringlistkey.hpp"
#include "afl/net/redis/stringsetkey.hpp"
#include "afl/net/redis/subtree.hpp"
#include "afl/test/testrunner.hpp"
#include "server/mailout/root.hpp"

using afl::net::redis::HashKey;
using afl::net::redis::IntegerSetKey;
using afl::net::redis::StringListKey;
using afl::net::redis::StringSetKey;
using afl::net::redis::Subtree;

/** Test database access. */
AFL_TEST("server.mailout.Message:db", a)
{
    // Environment
    afl::net::redis::InternalDatabase db;
    server::mailout::Root root(db, server::mailout::Configuration());

    // Set up
    server::mailout::Message testee(root, 54, server::mailout::Message::Preparing);
    a.checkEqual("01", testee.getId(), 54);

    // Set properties of message
    testee.templateName().set("tp");
    testee.uniqueId().set("uid");
    testee.arguments().stringField("a1").set("v1");
    testee.attachments().pushBack("att");
    testee.receivers().add("r");
    testee.expireTime().set(1984);

    // Verify properties
    a.checkEqual("11", HashKey(db, "mqueue:msg:54:data").stringField("template").get(), "tp");
    a.checkEqual("12", HashKey(db, "mqueue:msg:54:data").stringField("uniqid").get(), "uid");
    a.checkEqual("13", HashKey(db, "mqueue:msg:54:args").stringField("a1").get(), "v1");
    a.checkEqual("14", StringListKey(db, "mqueue:msg:54:attach")[0], "att");
    a.check     ("15", StringSetKey(db, "mqueue:msg:54:to").contains("r"));
    a.checkEqual("16", HashKey(db, "mqueue:msg:54:data").intField("expire").get(), 1984);
}

/** Test remove(). */
AFL_TEST("server.mailout.Message:remove", a)
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
        a.check("01. keys", keys.size() > 0);
    }

    // Delete the message
    testee.remove();

    // Database must now be empty
    {
        afl::data::StringList_t keys;
        Subtree(db, "mqueue:").getKeyNames(keys);
        a.checkEqual("11. keys", keys.size(), 0U);
    }
}

/** Test send(). */
AFL_TEST("server.mailout.Message:send", a)
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
    a.checkEqual("01", HashKey(db, "mqueue:uniqid").intField("zx").get(), 92);
    a.check("02", IntegerSetKey(db, "mqueue:sending").contains(92));
    a.check("03", !IntegerSetKey(db, "mqueue:preparing").contains(92));
}
