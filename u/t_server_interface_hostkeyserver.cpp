/**
  *  \file u/t_server_interface_hostkeyserver.cpp
  *  \brief Test for server::interface::HostKeyServer
  */

#include <stdexcept>
#include "server/interface/hostkeyserver.hpp"

#include "t_server_interface.hpp"
#include "afl/data/access.hpp"
#include "afl/data/segment.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "server/interface/hostkey.hpp"
#include "server/interface/hostkeyclient.hpp"

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
void
TestServerInterfaceHostKeyServer::testIt()
{
    HostKeyMock mock("testIt");
    server::interface::HostKeyServer testee(mock);

    // getKey
    mock.expectCall("getKey(aaa)");
    mock.provideReturnValue(String_t("bbb"));
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("KEYGET").pushBackString("aaa")), "bbb");

    mock.expectCall("getKey(ccc)");
    mock.provideReturnValue(String_t("ddd"));
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("keyget").pushBackString("ccc")), "ddd");

    // listKeys
    mock.expectCall("listKeys");
    mock.provideReturnValue(size_t(2));
    mock.provideReturnValue(makeFullInfo());
    mock.provideReturnValue(makePartialInfo());

    {
        std::auto_ptr<server::Value_t> p(testee.call(Segment().pushBackString("KEYLS")));
        afl::data::Access a(p.get());
        TS_ASSERT_EQUALS(a.getArraySize(), 2U);
        TS_ASSERT_EQUALS(a[0]("id").toString(),            "key1");
        TS_ASSERT_EQUALS(a[0]("reg").toInteger(),          1);
        TS_ASSERT_EQUALS(a[0]("key1").toString(),          "key1 line1");
        TS_ASSERT_EQUALS(a[0]("key2").toString(),          "key1 line2");
        TS_ASSERT_EQUALS(a[0]("filePathName").toString(),  "u/x/ke1");
        TS_ASSERT_EQUALS(a[0]("fileUseCount").toInteger(), 5);
        TS_ASSERT_EQUALS(a[0]("game").toInteger(),         19);
        TS_ASSERT_EQUALS(a[0]("gameName").toString(),     "the Game");
        TS_ASSERT_EQUALS(a[0]("gameUseCount").toInteger(), 30);
        TS_ASSERT_EQUALS(a[0]("gameLastUsed").toInteger(), 99999);

        TS_ASSERT_EQUALS(a[1]("id").toString(),            "key2");
        TS_ASSERT_EQUALS(a[1]("reg").toInteger(),          0);
        TS_ASSERT_EQUALS(a[1]("key1").toString(),          "key2 line1");
        TS_ASSERT_EQUALS(a[1]("key2").toString(),          "key2 line2");
        TS_ASSERT_EQUALS(a[1]("gameUseCount").getValue(),  (server::Value_t*) 0);
    }
}

/** Test error cases.
    A: send invalid commands to a server.
    E: errors correctly reported. */
void
TestServerInterfaceHostKeyServer::testErrors()
{
    HostKeyMock mock("testErrors");
    server::interface::HostKeyServer testee(mock);

    Segment empty;    // g++-3.4 sees an invocation of a copy constructor if I construct this object in-place.
    TS_ASSERT_THROWS(testee.callVoid(empty), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("huh")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("KEYGET")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("KEYLS").pushBackString("X")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("KEYGET").pushBackString("A").pushBackString("B")), std::exception);
}

/** Test roundtrip behaviour.
    A: connect multiple servers and clients; give some commands.
    E: commands and replies correctly given through the stack. */
void
TestServerInterfaceHostKeyServer::testRoundtrip()
{
    HostKeyMock mock("testRoundtrip");
    server::interface::HostKeyServer level1(mock);
    server::interface::HostKeyClient level2(level1);
    server::interface::HostKeyServer level3(level2);
    server::interface::HostKeyClient level4(level3);

    // getKey
    mock.expectCall("getKey(aaa)");
    mock.provideReturnValue(String_t("bbb"));
    TS_ASSERT_EQUALS(level4.getKey("aaa"), "bbb");

    // list
    mock.expectCall("listKeys");
    mock.provideReturnValue(size_t(2));
    mock.provideReturnValue(makeFullInfo());
    mock.provideReturnValue(makePartialInfo());

    HostKey::Infos_t result;
    TS_ASSERT_THROWS_NOTHING(level4.listKeys(result));

    TS_ASSERT_EQUALS(result.size(),                    2U);
    TS_ASSERT_EQUALS(result[0].keyId,                  "key1");
    TS_ASSERT_EQUALS(result[0].isRegistered,           true);
    TS_ASSERT_EQUALS(result[0].label1,                 "key1 line1");
    TS_ASSERT_EQUALS(result[0].label2,                 "key1 line2");
    TS_ASSERT_EQUALS(result[0].filePathName.orElse(""), "u/x/ke1");
    TS_ASSERT_EQUALS(result[0].fileUseCount.orElse(0), 5);
    TS_ASSERT_EQUALS(result[0].lastGame.orElse(0),     19);
    TS_ASSERT_EQUALS(result[0].lastGameName.orElse(""), "the Game");
    TS_ASSERT_EQUALS(result[0].gameUseCount.orElse(0), 30);
    TS_ASSERT_EQUALS(result[0].gameLastUsed.orElse(0), 99999);

    TS_ASSERT_EQUALS(result[1].keyId,                  "key2");
    TS_ASSERT_EQUALS(result[1].isRegistered,           false);
    TS_ASSERT_EQUALS(result[1].label1,                 "key2 line1");
    TS_ASSERT_EQUALS(result[1].label2,                 "key2 line2");
    TS_ASSERT_EQUALS(result[1].filePathName.isValid(), false);
    TS_ASSERT_EQUALS(result[1].fileUseCount.isValid(), false);
    TS_ASSERT_EQUALS(result[1].lastGame.isValid(),     false);
    TS_ASSERT_EQUALS(result[1].lastGameName.isValid(), false);
    TS_ASSERT_EQUALS(result[1].gameUseCount.isValid(), false);
    TS_ASSERT_EQUALS(result[1].gameLastUsed.isValid(), false);
}

