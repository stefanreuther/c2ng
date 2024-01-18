/**
  *  \file test/server/interface/hostkeyclienttest.cpp
  *  \brief Test for server::interface::HostKeyClient
  */

#include "server/interface/hostkeyclient.hpp"

#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/test/commandhandler.hpp"
#include "afl/test/testrunner.hpp"
#include "server/types.hpp"

using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Vector;
using afl::data::VectorValue;

/** Test client functionality.
    A: call client functions.
    E: observe that correct commands are generated, return data is correctly unpacked. */
AFL_TEST("server.interface.HostKeyClient", a)
{
    afl::test::CommandHandler mock(a);
    server::interface::HostKeyClient testee(mock);

    // getKey
    mock.expectCall("KEYGET, ggg");
    mock.provideNewResult(server::makeStringValue("bbb"));
    a.checkEqual("01. getKey", testee.getKey("ggg"), "bbb");

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
        AFL_CHECK_SUCCEEDS(a("11. listKeys"), testee.listKeys(result));

        // Verify
        a.checkEqual("21. size", result.size(), 2U);

        a.checkEqual("31. keyId",        result[0].keyId, "kkk1");
        a.checkEqual("32. isRegistered", result[0].isRegistered, true);
        a.checkEqual("33. label1",       result[0].label1, "first1");
        a.checkEqual("34. label2",       result[0].label2, "second1");
        a.checkEqual("35. filePathName", result[0].filePathName.orElse(""), "u/v/w");
        a.checkEqual("36. fileUseCount", result[0].fileUseCount.orElse(0), 7);
        a.checkEqual("37. lastGame",     result[0].lastGame.orElse(0), 12);
        a.checkEqual("38. lastGameName", result[0].lastGameName.orElse(""), "twelve");
        a.checkEqual("39. gameUseCount", result[0].gameUseCount.orElse(0), 150);
        a.checkEqual("40. gameLastUsed", result[0].gameLastUsed.orElse(0), 55555);

        a.checkEqual("41. keyId",        result[1].keyId, "kkk2");
        a.checkEqual("42. isRegistered", result[1].isRegistered, false);
        a.checkEqual("43. label1",       result[1].label1, "first2");
        a.checkEqual("44. label2",       result[1].label2, "second2");
        a.checkEqual("45. filePathName", result[1].filePathName.isValid(), false);
        a.checkEqual("46. fileUseCount", result[1].fileUseCount.isValid(), false);
        a.checkEqual("47. lastGame",     result[1].lastGame.isValid(), false);
        a.checkEqual("48. lastGameName", result[1].lastGameName.isValid(), false);
        a.checkEqual("49. gameUseCount", result[1].gameUseCount.isValid(), false);
        a.checkEqual("50. gameLastUsed", result[1].gameLastUsed.isValid(), false);
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
        AFL_CHECK_SUCCEEDS(a("51. listKeys"), testee.listKeys(result));

        // Verify
        a.checkEqual("61. size", result.size(), 1U);
        a.checkEqual("62. keyId",        result[0].keyId, "");
        a.checkEqual("63. isRegistered", result[0].isRegistered, false);
        a.checkEqual("64. label1",       result[0].label1, "");
        a.checkEqual("65. label2",       result[0].label2, "");
        a.checkEqual("66. filePathName", result[0].filePathName.isValid(), false);
        a.checkEqual("67. fileUseCount", result[0].fileUseCount.isValid(), false);
        a.checkEqual("68. lastGame",     result[0].lastGame.isValid(), false);
        a.checkEqual("69. lastGameName", result[0].lastGameName.isValid(), false);
        a.checkEqual("70. gameUseCount", result[0].gameUseCount.isValid(), false);
        a.checkEqual("71. gameLastUsed", result[0].gameLastUsed.isValid(), false);
    }

    mock.checkFinish();
}
