/**
  *  \file test/server/interface/userdataservertest.cpp
  *  \brief Test for server::interface::UserDataServer
  */

#include "server/interface/userdataserver.hpp"

#include "afl/data/segment.hpp"
#include "afl/test/assert.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"
#include "server/interface/userdata.hpp"
#include "server/interface/userdataclient.hpp"
#include <stdexcept>

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
AFL_TEST("server.interface.UserDataServer:commands", a)
{
    UserDataMock mock(a);
    server::interface::UserDataServer testee(mock);

    // Commands
    mock.expectCall("set(aa,bb,cc)");
    AFL_CHECK_SUCCEEDS(a("01. uset"), testee.callVoid(Segment().pushBackString("USET").pushBackString("aa").pushBackString("bb").pushBackString("cc")));

    mock.expectCall("get(Aa,Bb)");
    mock.provideReturnValue(String_t("Rr"));
    a.checkEqual("11. uget", testee.callString(Segment().pushBackString("UGET").pushBackString("Aa").pushBackString("Bb")), "Rr");

    // Variation
    mock.expectCall("get(AA,BB)");
    mock.provideReturnValue(String_t("RR"));
    a.checkEqual("21. uget", testee.callString(Segment().pushBackString("uget").pushBackString("AA").pushBackString("BB")), "RR");

    mock.checkFinish();
}

/** Test errors. */
AFL_TEST("server.interface.UserDataServer:errors", a)
{
    UserDataMock mock(a);
    server::interface::UserDataServer testee(mock);

    // Parameter count
    Segment empty;
    AFL_CHECK_THROWS(a("01. no verb"),       testee.callVoid(empty), std::exception);
    AFL_CHECK_THROWS(a("02. missing arg"),   testee.callVoid(Segment().pushBackString("USET")), std::exception);
    AFL_CHECK_THROWS(a("03. missing arg"),   testee.callVoid(Segment().pushBackString("USET").pushBackString("XX")), std::exception);
    AFL_CHECK_THROWS(a("04. missing arg"),   testee.callVoid(Segment().pushBackString("USET").pushBackString("XX").pushBackString("XX")), std::exception);
    AFL_CHECK_THROWS(a("05. too many args"), testee.callVoid(Segment().pushBackString("UGET").pushBackString("XX").pushBackString("XX").pushBackString("XX")), std::exception);

    // Verb
    AFL_CHECK_THROWS(a("11. bad verb"), testee.callVoid(Segment().pushBackString("")), std::exception);
    AFL_CHECK_THROWS(a("12. bad verb"), testee.callVoid(Segment().pushBackString("GET")), std::exception);
    AFL_CHECK_THROWS(a("13. bad verb"), testee.callVoid(Segment().pushBackString("foo")), std::exception);

    mock.checkFinish();
}

/** Test round-trip compatibility with UserDataClient. */
AFL_TEST("server.interface.UserDataServer:roundtrip", a)
{
    UserDataMock mock(a);
    server::interface::UserDataServer level1(mock);
    server::interface::UserDataClient level2(level1);
    server::interface::UserDataServer level3(level2);
    server::interface::UserDataClient level4(level3);

    mock.expectCall("set(one,two,three)");
    AFL_CHECK_SUCCEEDS(a("01. set"), level4.set("one", "two", "three"));

    mock.expectCall("get(user,key)");
    mock.provideReturnValue(String_t("result"));
    a.checkEqual("11. get", level4.get("user", "key"), "result");

    mock.checkFinish();
}
