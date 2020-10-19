/**
  *  \file u/t_server_host_keystore.cpp
  *  \brief Test for server::host::KeyStore
  */

#include "server/host/keystore.hpp"

#include "t_server_host.hpp"
#include "afl/charset/charset.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
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
void
TestServerHostKeyStore::testIt()
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
    TS_ASSERT_EQUALS(keys.size(), 1U);
    TS_ASSERT_EQUALS(keys[0].lastGame, GAME_ID);
    TS_ASSERT_EQUALS(keys[0].lastUsed, TIME);
    TS_ASSERT_EQUALS(keys[0].useCount, 1);
    
    // Key must be readable
    RegistrationKey key2(makeCharset());
    bool ok = testee.getKey(keys[0].keyId, key2);
    TS_ASSERT(ok);

    TS_ASSERT_EQUALS(key2.getKeyId(), key.getKeyId());
    TS_ASSERT_EQUALS(key2.getLine(RegistrationKey::Line1), key.getLine(RegistrationKey::Line1));
    TS_ASSERT_EQUALS(key2.getLine(RegistrationKey::Line2), key.getLine(RegistrationKey::Line2));

    // DB content
    TS_ASSERT_EQUALS(StringSetKey(db, "t:all").size(), 1);
}

/** Test listing a bogus (empty) key.
    A: create database containing a key without payload
    E: empty key is not included in listings and cannot be retrieved */
void
TestServerHostKeyStore::testListEmpty()
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
    TS_ASSERT(!testee.getKey("nokey", k));
    TS_ASSERT(!testee.getKey("badkey", k));
    TS_ASSERT( testee.getKey("goodkey", k));

    KeyStore::Infos_t keys;
    testee.listKeys(keys, *makeCharset());
    TS_ASSERT_EQUALS(keys.size(), 1U);
    TS_ASSERT_EQUALS(keys[0].keyId, "goodkey");
    TS_ASSERT_EQUALS(keys[0].lastGame, 3);
    TS_ASSERT_EQUALS(keys[0].lastUsed, 500);
    TS_ASSERT_EQUALS(keys[0].useCount, 2);
}

/** Test key expiry.
    A: register 15 keys.
    E: only 10 keys remain (config option) */
void
TestServerHostKeyStore::testExpire()
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
    TS_ASSERT_EQUALS(keys.size(), 10U);
    for (size_t i = 0; i < keys.size(); ++i) {
        TS_ASSERT_LESS_THAN(5, keys[i].lastGame);
        TS_ASSERT_LESS_THAN(1005, keys[i].lastUsed);
    }
    TS_ASSERT_EQUALS(StringSetKey(db, "t:all").size(), 10);
}

/** Test configuration: key store disabled.
    A: configure maxStoredKeys=0. Register 15 keys.
    E: nothing stored */
void
TestServerHostKeyStore::testNoStore()
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
    TS_ASSERT_EQUALS(keys.size(), 0U);
    TS_ASSERT_EQUALS(StringSetKey(db, "t:all").size(), 0);
}

/** Test configuration: key store limit disabled.
    A: configure maxStoredKeys=-1 (no limit). Register 200 keys.
    E: all keys stored */
void
TestServerHostKeyStore::testNoLimit()
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
    TS_ASSERT_EQUALS(keys.size(), 200U);
    TS_ASSERT_EQUALS(StringSetKey(db, "t:all").size(), 200);
}

