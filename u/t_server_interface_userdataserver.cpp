/**
  *  \file u/t_server_interface_userdataserver.cpp
  *  \brief Test for server::interface::UserDataServer
  */

#include <stdexcept>
#include "server/interface/userdataserver.hpp"

#include "t_server_interface.hpp"
#include "afl/data/segment.hpp"
#include "afl/test/assert.hpp"
#include "afl/test/callreceiver.hpp"
#include "server/interface/userdata.hpp"
#include "server/interface/userdataclient.hpp"

using afl::data::Segment;

namespace {
    class UserDataMock : public server::interface::UserData, public afl::test::CallReceiver {
     public:
        UserDataMock(afl::test::Assert a)
            : UserData(),
              CallReceiver(a)
            { }

        virtual void set(String_t userId, String_t key, String_t value)
            {
                checkCall(afl::string::Format("set(%s,%s,%s)", userId, key, value));
            }

        virtual String_t get(String_t userId, String_t key)
            {
                checkCall(afl::string::Format("get(%s,%s)", userId, key));
                return consumeReturnValue<String_t>();
            }
    };
}

/** Test regular usage. */
void
TestServerInterfaceUserDataServer::testIt()
{
    UserDataMock mock("TestServerInterfaceUserDataServer::testIt");
    server::interface::UserDataServer testee(mock);

    // Commands
    mock.expectCall("set(aa,bb,cc)");
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("USET").pushBackString("aa").pushBackString("bb").pushBackString("cc")));

    mock.expectCall("get(Aa,Bb)");
    mock.provideReturnValue(String_t("Rr"));
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("UGET").pushBackString("Aa").pushBackString("Bb")), "Rr");

    // Variation
    mock.expectCall("get(AA,BB)");
    mock.provideReturnValue(String_t("RR"));
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("uget").pushBackString("AA").pushBackString("BB")), "RR");

    mock.checkFinish();
}

/** Test errors. */
void
TestServerInterfaceUserDataServer::testErrors()
{
    UserDataMock mock("TestServerInterfaceUserDataServer::testErrors");
    server::interface::UserDataServer testee(mock);

    // Parameter count
    Segment empty;
    TS_ASSERT_THROWS(testee.callVoid(empty), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("USET")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("USET").pushBackString("XX")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("USET").pushBackString("XX").pushBackString("XX")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("UGET").pushBackString("XX").pushBackString("XX").pushBackString("XX")), std::exception);

    // Verb
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("GET")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("foo")), std::exception);

    mock.checkFinish();
}

/** Test round-trip compatibility with UserDataClient. */
void
TestServerInterfaceUserDataServer::testRoundtrip()
{
    UserDataMock mock("TestServerInterfaceUserDataServer::testErrors");
    server::interface::UserDataServer level1(mock);
    server::interface::UserDataClient level2(level1);
    server::interface::UserDataServer level3(level2);
    server::interface::UserDataClient level4(level3);

    mock.expectCall("set(one,two,three)");
    TS_ASSERT_THROWS_NOTHING(level4.set("one", "two", "three"));

    mock.expectCall("get(user,key)");
    mock.provideReturnValue(String_t("result"));
    TS_ASSERT_EQUALS(level4.get("user", "key"), "result");

    mock.checkFinish();
}

