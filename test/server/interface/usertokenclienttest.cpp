/**
  *  \file test/server/interface/usertokenclienttest.cpp
  *  \brief Test for server::interface::UserTokenClient
  */

#include "server/interface/usertokenclient.hpp"

#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/test/commandhandler.hpp"
#include "afl/test/testrunner.hpp"
#include "server/types.hpp"

AFL_TEST("server.interface.UserTokenClient", a)
{
    afl::test::CommandHandler mock(a);
    server::interface::UserTokenClient testee(mock);

    // getToken
    mock.expectCall("MAKETOKEN, u10, key");
    mock.provideNewResult(server::makeStringValue("019283132"));
    a.checkEqual("01. getToken", testee.getToken("u10", "key"), "019283132");

    // checkToken
    // - input
    mock.expectCall("CHECKTOKEN, xyzzy");
    mock.provideNewResult(0);
    AFL_CHECK_SUCCEEDS(a("11. checkToken"), testee.checkToken("xyzzy", afl::base::Nothing, false));

    mock.expectCall("CHECKTOKEN, xyzzy, RENEW");
    mock.provideNewResult(0);
    AFL_CHECK_SUCCEEDS(a("21. checkToken"), testee.checkToken("xyzzy", afl::base::Nothing, true));

    mock.expectCall("CHECKTOKEN, hurz, TYPE, api");
    mock.provideNewResult(0);
    AFL_CHECK_SUCCEEDS(a("31. checkToken"), testee.checkToken("hurz", String_t("api"), false));

    // - output
    {
        afl::data::Hash::Ref_t h = afl::data::Hash::create();
        h->setNew("user", server::makeStringValue("x"));
        h->setNew("type", server::makeStringValue("reset"));
        h->setNew("new",  server::makeStringValue("hehe"));
        mock.expectCall("CHECKTOKEN, foo");
        mock.provideNewResult(new afl::data::HashValue(h));
        server::interface::UserToken::Info i = testee.checkToken("foo", afl::base::Nothing, false);
        a.checkEqual("41. userId",    i.userId, "x");
        a.checkEqual("42. tokenType", i.tokenType, "reset");
        a.checkEqual("43. newToken",  i.newToken.orElse(""), "hehe");
    }
    {
        afl::data::Hash::Ref_t h = afl::data::Hash::create();
        h->setNew("user", server::makeStringValue("y"));
        h->setNew("type", server::makeStringValue("api"));
        mock.expectCall("CHECKTOKEN, foo");
        mock.provideNewResult(new afl::data::HashValue(h));
        server::interface::UserToken::Info i = testee.checkToken("foo", afl::base::Nothing, false);
        a.checkEqual("44. userId",    i.userId, "y");
        a.checkEqual("45. tokenType", i.tokenType, "api");
        a.checkEqual("46. newToken",  i.newToken.isValid(), false);
    }

    // clearToken
    const String_t types[] = { "a", "b" };
    mock.expectCall("RESETTOKEN, u99, a, b");
    mock.provideNewResult(0);
    AFL_CHECK_SUCCEEDS(a("51. clearToken"), testee.clearToken("u99", types));

    mock.expectCall("RESETTOKEN, u99");
    mock.provideNewResult(0);
    AFL_CHECK_SUCCEEDS(a("61. clearToken"), testee.clearToken("u99", afl::base::Memory<const String_t>()));

    mock.checkFinish();
}
