/**
  *  \file u/t_server_interface_hostkeyclient.cpp
  *  \brief Test for server::interface::HostKeyClient
  */

#include "server/interface/hostkeyclient.hpp"

#include "t_server_interface.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/test/commandhandler.hpp"
#include "server/types.hpp"

using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Vector;
using afl::data::VectorValue;

/** Test client functionality.
    A: call client functions.
    E: observe that correct commands are generated, return data is correctly unpacked. */
void
TestServerInterfaceHostKeyClient::testIt()
{
    afl::test::CommandHandler mock("testIt");
    server::interface::HostKeyClient testee(mock);

    // getKey
    mock.expectCall("KEYGET, ggg");
    mock.provideNewResult(server::makeStringValue("bbb"));
    TS_ASSERT_EQUALS(testee.getKey("ggg"), "bbb");

    // listKeys
    {
        Vector::Ref_t vec = Vector::create();

        // one fully-populated entry
        Hash::Ref_t h1 = Hash::create();
        h1->setNew("id",           server::makeStringValue("kkk1"));
        h1->setNew("reg",          server::makeIntegerValue(1));
        h1->setNew("key1",         server::makeStringValue("first1"));
        h1->setNew("key2",         server::makeStringValue("second1"));
        h1->setNew("filePathName", server::makeStringValue("u/v/w"));
        h1->setNew("fileUseCount", server::makeIntegerValue(7));
        h1->setNew("game",         server::makeIntegerValue(12));
        h1->setNew("gameName",     server::makeStringValue("twelve"));
        h1->setNew("gameUseCount", server::makeIntegerValue(150));
        h1->setNew("gameLastUsed", server::makeIntegerValue(55555));
        vec->pushBackNew(new HashValue(h1));

        // one half-populated entry
        Hash::Ref_t h2 = Hash::create();
        h2->setNew("id",           server::makeStringValue("kkk2"));
        h2->setNew("reg",          server::makeIntegerValue(0));
        h2->setNew("key1",         server::makeStringValue("first2"));
        h2->setNew("key2",         server::makeStringValue("second2"));
        vec->pushBackNew(new HashValue(h2));

        mock.expectCall("KEYLS");
        mock.provideNewResult(new VectorValue(vec));

        // Call
        server::interface::HostKey::Infos_t result;
        TS_ASSERT_THROWS_NOTHING(testee.listKeys(result));

        // Verify
        TS_ASSERT_EQUALS(result.size(), 2U);

        TS_ASSERT_EQUALS(result[0].keyId, "kkk1");
        TS_ASSERT_EQUALS(result[0].isRegistered, true);
        TS_ASSERT_EQUALS(result[0].label1, "first1");
        TS_ASSERT_EQUALS(result[0].label2, "second1");
        TS_ASSERT_EQUALS(result[0].filePathName.orElse(""), "u/v/w");
        TS_ASSERT_EQUALS(result[0].fileUseCount.orElse(0), 7);
        TS_ASSERT_EQUALS(result[0].lastGame.orElse(0), 12);
        TS_ASSERT_EQUALS(result[0].lastGameName.orElse(""), "twelve");
        TS_ASSERT_EQUALS(result[0].gameUseCount.orElse(0), 150);
        TS_ASSERT_EQUALS(result[0].gameLastUsed.orElse(0), 55555);

        TS_ASSERT_EQUALS(result[1].keyId, "kkk2");
        TS_ASSERT_EQUALS(result[1].isRegistered, false);
        TS_ASSERT_EQUALS(result[1].label1, "first2");
        TS_ASSERT_EQUALS(result[1].label2, "second2");
        TS_ASSERT_EQUALS(result[1].filePathName.isValid(), false);
        TS_ASSERT_EQUALS(result[1].fileUseCount.isValid(), false);
        TS_ASSERT_EQUALS(result[1].lastGame.isValid(), false);
        TS_ASSERT_EQUALS(result[1].lastGameName.isValid(), false);
        TS_ASSERT_EQUALS(result[1].gameUseCount.isValid(), false);
        TS_ASSERT_EQUALS(result[1].gameLastUsed.isValid(), false);
    }

    // listKeys, abuse case: one null entry (will be ignored), one empty entry
    {
        Vector::Ref_t vec = Vector::create();
        vec->pushBackNew(0);
        vec->pushBackNew(new HashValue(Hash::create()));
        mock.expectCall("KEYLS");
        mock.provideNewResult(new VectorValue(vec));

        // Call
        server::interface::HostKey::Infos_t result;
        TS_ASSERT_THROWS_NOTHING(testee.listKeys(result));

        // Verify
        TS_ASSERT_EQUALS(result.size(), 1U);
        TS_ASSERT_EQUALS(result[0].keyId, "");
        TS_ASSERT_EQUALS(result[0].isRegistered, false);
        TS_ASSERT_EQUALS(result[0].label1, "");
        TS_ASSERT_EQUALS(result[0].label2, "");
        TS_ASSERT_EQUALS(result[0].filePathName.isValid(), false);
        TS_ASSERT_EQUALS(result[0].fileUseCount.isValid(), false);
        TS_ASSERT_EQUALS(result[0].lastGame.isValid(), false);
        TS_ASSERT_EQUALS(result[0].lastGameName.isValid(), false);
        TS_ASSERT_EQUALS(result[0].gameUseCount.isValid(), false);
        TS_ASSERT_EQUALS(result[0].gameLastUsed.isValid(), false);
    }

    mock.checkFinish();
}

