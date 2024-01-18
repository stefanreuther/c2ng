/**
  *  \file test/server/host/keystoretest.cpp
  *  \brief Test for server::host::KeyStore
  */

#include "server/host/keystore.hpp"

#include "afl/charset/charset.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/test/testrunner.hpp"
#include "game/v3/registrationkey.hpp"
#include "server/host/configuration.hpp"

using afl::net::redis::HashKey;
using afl::net::redis::InternalDatabase;
using afl::net::redis::StringSetKey;
using afl::net::redis::Subtree;
using game::v3::RegistrationKey;
using server::host::KeyStore;

namespace {

    std::auto_ptr<afl::charset::Charset> makeCharset()
    {
        return std::auto_ptr<afl::charset::Charset>(new afl::charset::CodepageCharset(afl::charset::g_codepage437));
    }

    server::host::Configuration makeConfig(int maxStoredKeys)
    {
        server::host::Configuration config;
        config.maxStoredKeys = maxStoredKeys;
        return config;
    }
}

/** Simple round-trip functionality test.
    A: create a dummy key. Store and retrieve it.
    E: retrieved key is identical to original */
AFL_TEST("server.host.KeyStore:basics", a)
{
    InternalDatabase db;
    KeyStore testee(Subtree(db, "t:"), makeConfig(10));

    String_t dummyKeyBlob(RegistrationKey::KEY_SIZE_BYTES, 'x');
    RegistrationKey key(makeCharset());
    key.unpackFromBytes(afl::string::toBytes(dummyKeyBlob));

    const server::Time_t TIME = 99999;
    const int32_t GAME_ID = 12;

    testee.addKey(key, TIME, GAME_ID);

    // Key must be listable
    KeyStore::Infos_t keys;
    testee.listKeys(keys, *makeCharset());
    a.checkEqual("01. size", keys.size(), 1U);
    a.checkEqual("02. lastGame", keys[0].lastGame, GAME_ID);
    a.checkEqual("03. lastUsed", keys[0].lastUsed, TIME);
    a.checkEqual("04. useCount", keys[0].useCount, 1);

    // Key must be readable
    RegistrationKey key2(makeCharset());
    bool ok = testee.getKey(keys[0].keyId, key2);
    a.check("11. getKey", ok);

    a.checkEqual("21. getKeyId", key2.getKeyId(), key.getKeyId());
    a.checkEqual("22. Line1", key2.getLine(RegistrationKey::Line1), key.getLine(RegistrationKey::Line1));
    a.checkEqual("23. Line2", key2.getLine(RegistrationKey::Line2), key.getLine(RegistrationKey::Line2));

    // DB content
    a.checkEqual("31. db", StringSetKey(db, "t:all").size(), 1);
}

/** Test listing a bogus (empty) key.
    A: create database containing a key without payload
    E: empty key is not included in listings and cannot be retrieved */
AFL_TEST("server.host.KeyStore:ilst-empty", a)
{
    // Environment
    InternalDatabase db;

    // - bad key (no payload data)
    StringSetKey(db, "t:all").add("badkey");

    // - good key
    StringSetKey(db, "t:all").add("goodkey");
    HashKey(db, "t:id:badkey").intField("lastGame").set(3);
    HashKey(db, "t:id:goodkey").intField("lastGame").set(3);
    HashKey(db, "t:id:goodkey").intField("lastUsed").set(500);
    HashKey(db, "t:id:goodkey").intField("useCount").set(2);
    HashKey(db, "t:id:goodkey").stringField("blob").set("xxxxx");

    // Test it
    KeyStore testee(Subtree(db, "t:"), makeConfig(10));

    RegistrationKey k(makeCharset());
    a.check("01. getKey", !testee.getKey("nokey", k));
    a.check("02. getKey", !testee.getKey("badkey", k));
    a.check("03. getKey",  testee.getKey("goodkey", k));

    KeyStore::Infos_t keys;
    testee.listKeys(keys, *makeCharset());
    a.checkEqual("11. size",     keys.size(), 1U);
    a.checkEqual("12. keyId",    keys[0].keyId, "goodkey");
    a.checkEqual("13. lastGame", keys[0].lastGame, 3);
    a.checkEqual("14. lastUsed", keys[0].lastUsed, 500);
    a.checkEqual("15. useCount", keys[0].useCount, 2);
}

/** Test key expiry.
    A: register 15 keys.
    E: only 10 keys remain (config option) */
AFL_TEST("server.host.KeyStore:expire", a)
{
    InternalDatabase db;
    KeyStore testee(Subtree(db, "t:"), makeConfig(10));

    // Register 15 keys
    for (int i = 1; i <= 15; ++i) {
        String_t dummyKeyBlob(RegistrationKey::KEY_SIZE_BYTES, char(i));
        RegistrationKey key(makeCharset());
        key.unpackFromBytes(afl::string::toBytes(dummyKeyBlob));
        testee.addKey(key, 1000+i, i);
    }

    // Read back
    KeyStore::Infos_t keys;
    testee.listKeys(keys, *makeCharset());
    a.checkEqual("01. size", keys.size(), 10U);
    for (size_t i = 0; i < keys.size(); ++i) {
        a.checkLessThan("02. lastGame", 5, keys[i].lastGame);
        a.checkLessThan("03. lastUsed", 1005, keys[i].lastUsed);
    }
    a.checkEqual("04. db", StringSetKey(db, "t:all").size(), 10);
}

/** Test configuration: key store disabled.
    A: configure maxStoredKeys=0. Register 15 keys.
    E: nothing stored */
AFL_TEST("server.host.KeyStore:store-disabled", a)
{
    InternalDatabase db;
    KeyStore testee(Subtree(db, "t:"), makeConfig(0));

    // Register 15 keys
    for (int i = 1; i <= 15; ++i) {
        String_t dummyKeyBlob(RegistrationKey::KEY_SIZE_BYTES, char(i));
        RegistrationKey key(makeCharset());
        key.unpackFromBytes(afl::string::toBytes(dummyKeyBlob));
        testee.addKey(key, 1000+i, i);
    }

    // Read back: all keys stored, none stored
    KeyStore::Infos_t keys;
    testee.listKeys(keys, *makeCharset());
    a.checkEqual("01. size", keys.size(), 0U);
    a.checkEqual("02. db", StringSetKey(db, "t:all").size(), 0);
}

/** Test configuration: key store limit disabled.
    A: configure maxStoredKeys=-1 (no limit). Register 200 keys.
    E: all keys stored */
AFL_TEST("server.host.KeyStore:unlimited", a)
{
    InternalDatabase db;
    KeyStore testee(Subtree(db, "t:"), makeConfig(-1));

    // Register 15 keys
    for (int i = 1; i <= 200; ++i) {
        String_t dummyKeyBlob(RegistrationKey::KEY_SIZE_BYTES, char(i));
        RegistrationKey key(makeCharset());
        key.unpackFromBytes(afl::string::toBytes(dummyKeyBlob));
        testee.addKey(key, 1000+i, i);
    }

    // Read back: all keys stored, none expired
    KeyStore::Infos_t keys;
    testee.listKeys(keys, *makeCharset());
    a.checkEqual("01. size", keys.size(), 200U);
    a.checkEqual("02. db", StringSetKey(db, "t:all").size(), 200);
}
