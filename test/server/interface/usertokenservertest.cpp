/**
  *  \file test/server/interface/usertokenservertest.cpp
  *  \brief Test for server::interface::UserTokenServer
  */

#include "server/interface/usertokenserver.hpp"

#include "afl/data/access.hpp"
#include "afl/data/segment.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"
#include "server/interface/usertoken.hpp"
#include "server/interface/usertokenclient.hpp"
#include "server/types.hpp"
#include <stdexcept>

using afl::data::Segment;
using afl::string::Format;
using server::interface::UserToken;

namespace {
    class UserTokenMock : public afl::test::CallReceiver, public UserToken {
     public:
        UserTokenMock(afl::test::Assert a)
            : CallReceiver(a)
            { }
        virtual String_t getToken(String_t userId, String_t tokenType)
            {
                checkCall(Format("getToken(%s,%s)", userId, tokenType));
                return consumeReturnValue<String_t>();
            }

        virtual Info checkToken(String_t token, afl::base::Optional<String_t> requiredType, bool autoRenew)
            {
                checkCall(Format("checkToken(%s,%s,%d)", token, requiredType.orElse("<none>"), int(autoRenew)));
                return consumeReturnValue<Info>();
            }

        virtual void clearToken(String_t userId, afl::base::Memory<const String_t> tokenTypes)
            {
                String_t msg = Format("clearToken(%s", userId);
                while (const String_t* p = tokenTypes.eat()) {
                    msg += ",";
                    msg += *p;
                }
                checkCall(msg + ")");
            }
   };
}

/** Test regular server calls. */
AFL_TEST("server.interface.UserTokenServer:commands", a)
{
    UserTokenMock mock(a);
    server::interface::UserTokenServer testee(mock);

    // getToken
    mock.expectCall("getToken(uu,tt)");
    mock.provideReturnValue(String_t("kkkk"));
    a.checkEqual("01. maketoken", testee.callString(Segment().pushBackString("MAKETOKEN").pushBackString("uu").pushBackString("tt")), "kkkk");

    // checkToken
    // - out
    {
        UserToken::Info i;
        i.userId = "u1";
        i.tokenType = "t2";
        i.newToken = "nt";
        mock.expectCall("checkToken(ot,<none>,1)");
        mock.provideReturnValue(i);

        std::auto_ptr<server::Value_t> p(testee.call(Segment().pushBackString("CHECKTOKEN").pushBackString("ot").pushBackString("RENEW")));
        afl::data::Access ap(p);
        a.checkEqual("11", ap("user").toString(), "u1");
        a.checkEqual("12", ap("type").toString(), "t2");
        a.checkEqual("13", ap("new").toString(), "nt");
    }

    // - in
    mock.expectCall("checkToken(ot2,rt,0)");
    mock.provideReturnValue(UserToken::Info());
    testee.callVoid(Segment().pushBackString("CHECKTOKEN").pushBackString("ot2").pushBackString("TYPE").pushBackString("rt"));

    mock.expectCall("checkToken(ot2,rt,1)");
    mock.provideReturnValue(UserToken::Info());
    testee.callVoid(Segment().pushBackString("CHECKTOKEN").pushBackString("ot2").pushBackString("RENEW").pushBackString("TYPE").pushBackString("rt"));

    // clearToken
    mock.expectCall("clearToken(uc,c1,c2)");
    testee.callVoid(Segment().pushBackString("RESETTOKEN").pushBackString("uc").pushBackString("c1").pushBackString("c2"));
    mock.expectCall("clearToken(uc)");
    testee.callVoid(Segment().pushBackString("RESETTOKEN").pushBackString("uc"));

    // Variants
    mock.expectCall("clearToken(uc)");
    testee.callVoid(Segment().pushBackString("resettoken").pushBackString("uc"));
    mock.expectCall("checkToken(ot2,rt,1)");
    mock.provideReturnValue(UserToken::Info());
    testee.callVoid(Segment().pushBackString("CheckToken").pushBackString("ot2").pushBackString("type").pushBackString("rt").pushBackString("reNew"));

    mock.checkFinish();
}

/** Test errors. */
AFL_TEST("server.interface.UserTokenServer:errors", a)
{
    UserTokenMock mock(a);
    server::interface::UserTokenServer testee(mock);

    // No command
    Segment empty;
    AFL_CHECK_THROWS(a("01. no verb"), testee.callVoid(empty), std::exception);

    // Bad command
    AFL_CHECK_THROWS(a("11. bad verb"), testee.callVoid(Segment().pushBackString("")), std::exception);
    AFL_CHECK_THROWS(a("12. bad verb"), testee.callVoid(Segment().pushBackString("CHECK")), std::exception);

    // Wrong number of parameters
    AFL_CHECK_THROWS(a("21. missing arg"),   testee.callVoid(Segment().pushBackString("CHECKTOKEN")), std::exception);
    AFL_CHECK_THROWS(a("22. missing arg"),   testee.callVoid(Segment().pushBackString("MAKETOKEN").pushBackString("a")), std::exception);
    AFL_CHECK_THROWS(a("23. too many args"), testee.callVoid(Segment().pushBackString("MAKETOKEN").pushBackString("a").pushBackString("b").pushBackString("c")), std::exception);

    // Wrong option
    AFL_CHECK_THROWS(a("31. bad option"), testee.callVoid(Segment().pushBackString("CHECKTOKEN").pushBackString("t").pushBackString("a")), std::exception);
    AFL_CHECK_THROWS(a("32. bad option"), testee.callVoid(Segment().pushBackString("CHECKTOKEN").pushBackString("t").pushBackString("type")), std::exception);
}

/** Test round-trip compatibility. */
AFL_TEST("server.interface.UserTokenServer:roundtrip", a)
{
    UserTokenMock mock(a);
    server::interface::UserTokenServer level1(mock);
    server::interface::UserTokenClient level2(level1);
    server::interface::UserTokenServer level3(level2);
    server::interface::UserTokenClient level4(level3);

    // getToken
    mock.expectCall("getToken(uu,tt)");
    mock.provideReturnValue(String_t("kkkk"));
    a.checkEqual("01. getToken", level4.getToken("uu", "tt"), "kkkk");

    // checkToken
    UserToken::Info i;
    i.userId = "u1";
    i.tokenType = "t2";
    i.newToken = "nt";
    mock.expectCall("checkToken(ot,<none>,1)");
    mock.provideReturnValue(i);

    UserToken::Info i2 = level4.checkToken("ot", afl::base::Nothing, true);
    a.checkEqual("11. userId",    i2.userId, "u1");
    a.checkEqual("12. tokenType", i2.tokenType, "t2");
    a.checkEqual("13. newToken",  i2.newToken.orElse("x"), "nt");

    // clearToken
    String_t cs[] = { "c1", "c2" };
    mock.expectCall("clearToken(uc,c1,c2)");
    level4.clearToken("uc", cs);

    mock.checkFinish();
}
