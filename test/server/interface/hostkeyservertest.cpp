/**
  *  \file test/server/interface/hostkeyservertest.cpp
  *  \brief Test for server::interface::HostKeyServer
  */

#include "server/interface/hostkeyserver.hpp"

#include "afl/data/access.hpp"
#include "afl/data/segment.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"
#include "server/interface/hostkey.hpp"
#include "server/interface/hostkeyclient.hpp"
#include <stdexcept>

using afl::string::Format;
using afl::data::Segment;
using server::interface::HostKey;

namespace {
    class HostKeyMock : public HostKey, public afl::test::CallReceiver {
     public:
        HostKeyMock(afl::test::Assert a)
            : HostKey(), CallReceiver(a)
            { }

        void listKeys(Infos_t& out)
            {
                checkCall("listKeys");
                size_t n = consumeReturnValue<size_t>();
                while (n > 0) {
                    out.push_back(consumeReturnValue<Info>());
                    --n;
                }
            }

        String_t getKey(String_t keyId)
            {
                checkCall(Format("getKey(%s)", keyId));
                return consumeReturnValue<String_t>();
            }
    };

    HostKey::Info makeFullInfo()
    {
        HostKey::Info info;
        info.keyId = "key1";
        info.isRegistered = true;
        info.label1 = "key1 line1";
        info.label2 = "key1 line2";
        info.filePathName = String_t("u/x/ke1");
        info.fileUseCount = 5;
        info.lastGame = 19;
        info.lastGameName = String_t("the Game");
        info.gameUseCount = 30;
        info.gameLastUsed = 99999;
        return info;
    }

    HostKey::Info makePartialInfo()
    {
        HostKey::Info info;
        info.keyId = "key2";
        info.isRegistered = false;
        info.label1 = "key2 line1";
        info.label2 = "key2 line2";
        return info;
    }
}

/** Test server.
    A: give commands to a server.
    E: commands are correctly decoded, correct results created. */
AFL_TEST("server.interface.HostKeyServer:commands", a)
{
    HostKeyMock mock(a);
    server::interface::HostKeyServer testee(mock);

    // getKey
    mock.expectCall("getKey(aaa)");
    mock.provideReturnValue(String_t("bbb"));
    a.checkEqual("01. keyget", testee.callString(Segment().pushBackString("KEYGET").pushBackString("aaa")), "bbb");

    mock.expectCall("getKey(ccc)");
    mock.provideReturnValue(String_t("ddd"));
    a.checkEqual("11. keyget", testee.callString(Segment().pushBackString("keyget").pushBackString("ccc")), "ddd");

    // listKeys
    mock.expectCall("listKeys");
    mock.provideReturnValue(size_t(2));
    mock.provideReturnValue(makeFullInfo());
    mock.provideReturnValue(makePartialInfo());

    {
        std::auto_ptr<server::Value_t> p(testee.call(Segment().pushBackString("KEYLS")));
        afl::data::Access ap(p.get());
        a.checkEqual("21. getArraySize", ap.getArraySize(), 2U);
        a.checkEqual("22. id",           ap[0]("id").toString(),            "key1");
        a.checkEqual("23. reg",          ap[0]("reg").toInteger(),          1);
        a.checkEqual("24. key1",         ap[0]("key1").toString(),          "key1 line1");
        a.checkEqual("25. key2",         ap[0]("key2").toString(),          "key1 line2");
        a.checkEqual("26. filePathName", ap[0]("filePathName").toString(),  "u/x/ke1");
        a.checkEqual("27. fileUseCount", ap[0]("fileUseCount").toInteger(), 5);
        a.checkEqual("28. game",         ap[0]("game").toInteger(),         19);
        a.checkEqual("29. gameName",     ap[0]("gameName").toString(),     "the Game");
        a.checkEqual("30. gameUseCount", ap[0]("gameUseCount").toInteger(), 30);
        a.checkEqual("31. gameLastUsed", ap[0]("gameLastUsed").toInteger(), 99999);

        a.checkEqual("41. id",           ap[1]("id").toString(),            "key2");
        a.checkEqual("42. reg",          ap[1]("reg").toInteger(),          0);
        a.checkEqual("43. key1",         ap[1]("key1").toString(),          "key2 line1");
        a.checkEqual("44. key2",         ap[1]("key2").toString(),          "key2 line2");
        a.checkEqual("45. gameUseCount", ap[1]("gameUseCount").getValue(),  (server::Value_t*) 0);
    }
}

/** Test error cases.
    A: send invalid commands to a server.
    E: errors correctly reported. */
AFL_TEST("server.interface.HostKeyServer:errors", a)
{
    HostKeyMock mock(a);
    server::interface::HostKeyServer testee(mock);

    Segment empty;    // g++-3.4 sees an invocation of a copy constructor if I construct this object in-place.
    AFL_CHECK_THROWS(a("01. empty"),         testee.callVoid(empty), std::exception);
    AFL_CHECK_THROWS(a("02. bad verb"),      testee.callVoid(Segment().pushBackString("huh")), std::exception);
    AFL_CHECK_THROWS(a("03. missing arg"),   testee.callVoid(Segment().pushBackString("KEYGET")), std::exception);
    AFL_CHECK_THROWS(a("04. too many args"), testee.callVoid(Segment().pushBackString("KEYLS").pushBackString("X")), std::exception);
    AFL_CHECK_THROWS(a("05. too many args"), testee.callVoid(Segment().pushBackString("KEYGET").pushBackString("A").pushBackString("B")), std::exception);
}

/** Test roundtrip behaviour.
    A: connect multiple servers and clients; give some commands.
    E: commands and replies correctly given through the stack. */
AFL_TEST("server.interface.HostKeyServer:roundtrip", a)
{
    HostKeyMock mock(a);
    server::interface::HostKeyServer level1(mock);
    server::interface::HostKeyClient level2(level1);
    server::interface::HostKeyServer level3(level2);
    server::interface::HostKeyClient level4(level3);

    // getKey
    mock.expectCall("getKey(aaa)");
    mock.provideReturnValue(String_t("bbb"));
    a.checkEqual("01. getKey", level4.getKey("aaa"), "bbb");

    // list
    mock.expectCall("listKeys");
    mock.provideReturnValue(size_t(2));
    mock.provideReturnValue(makeFullInfo());
    mock.provideReturnValue(makePartialInfo());

    HostKey::Infos_t result;
    AFL_CHECK_SUCCEEDS(a("11. listKeys"), level4.listKeys(result));

    a.checkEqual("21. size", result.size(),                    2U);
    a.checkEqual("22. keyId",        result[0].keyId,                  "key1");
    a.checkEqual("23. isRegistered", result[0].isRegistered,           true);
    a.checkEqual("24. label1",       result[0].label1,                 "key1 line1");
    a.checkEqual("25. label2",       result[0].label2,                 "key1 line2");
    a.checkEqual("26. filePathName", result[0].filePathName.orElse(""), "u/x/ke1");
    a.checkEqual("27. fileUseCount", result[0].fileUseCount.orElse(0), 5);
    a.checkEqual("28. lastGame",     result[0].lastGame.orElse(0),     19);
    a.checkEqual("29. lastGameName", result[0].lastGameName.orElse(""), "the Game");
    a.checkEqual("30. gameUseCount", result[0].gameUseCount.orElse(0), 30);
    a.checkEqual("31. gameLastUsed", result[0].gameLastUsed.orElse(0), 99999);

    a.checkEqual("41. keyId",        result[1].keyId,                  "key2");
    a.checkEqual("42. isRegistered", result[1].isRegistered,           false);
    a.checkEqual("43. label1",       result[1].label1,                 "key2 line1");
    a.checkEqual("44. label2",       result[1].label2,                 "key2 line2");
    a.checkEqual("45. filePathName", result[1].filePathName.isValid(), false);
    a.checkEqual("46. fileUseCount", result[1].fileUseCount.isValid(), false);
    a.checkEqual("47. lastGame",     result[1].lastGame.isValid(),     false);
    a.checkEqual("48. lastGameName", result[1].lastGameName.isValid(), false);
    a.checkEqual("49. gameUseCount", result[1].gameUseCount.isValid(), false);
    a.checkEqual("50. gameLastUsed", result[1].gameLastUsed.isValid(), false);
}
