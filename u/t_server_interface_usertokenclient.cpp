/**
  *  \file u/t_server_interface_usertokenclient.cpp
  *  \brief Test for server::interface::UserTokenClient
  */

#include "server/interface/usertokenclient.hpp"

#include "t_server_interface.hpp"
#include "afl/test/commandhandler.hpp"
#include "server/types.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"

void
TestServerInterfaceUserTokenClient::testIt()
{
    afl::test::CommandHandler mock("testIt");
    server::interface::UserTokenClient testee(mock);

    // getToken
    mock.expectCall("MAKETOKEN, u10, key");
    mock.provideNewResult(server::makeStringValue("019283132"));
    TS_ASSERT_EQUALS(testee.getToken("u10", "key"), "019283132");

    // checkToken
    // - input
    mock.expectCall("CHECKTOKEN, xyzzy");
    mock.provideNewResult(0);
    TS_ASSERT_THROWS_NOTHING(testee.checkToken("xyzzy", afl::base::Nothing, false));

    mock.expectCall("CHECKTOKEN, xyzzy, RENEW");
    mock.provideNewResult(0);
    TS_ASSERT_THROWS_NOTHING(testee.checkToken("xyzzy", afl::base::Nothing, true));

    mock.expectCall("CHECKTOKEN, hurz, TYPE, api");
    mock.provideNewResult(0);
    TS_ASSERT_THROWS_NOTHING(testee.checkToken("hurz", String_t("api"), false));

    // - output
    {
        afl::data::Hash::Ref_t h = afl::data::Hash::create();
        h->setNew("user", server::makeStringValue("x"));
        h->setNew("type", server::makeStringValue("reset"));
        h->setNew("new",  server::makeStringValue("hehe"));
        mock.expectCall("CHECKTOKEN, foo");
        mock.provideNewResult(new afl::data::HashValue(h));
        server::interface::UserToken::Info i = testee.checkToken("foo", afl::base::Nothing, false);
        TS_ASSERT_EQUALS(i.userId, "x");
        TS_ASSERT_EQUALS(i.tokenType, "reset");
        TS_ASSERT_EQUALS(i.newToken.orElse(""), "hehe");
    }
    {
        afl::data::Hash::Ref_t h = afl::data::Hash::create();
        h->setNew("user", server::makeStringValue("y"));
        h->setNew("type", server::makeStringValue("api"));
        mock.expectCall("CHECKTOKEN, foo");
        mock.provideNewResult(new afl::data::HashValue(h));
        server::interface::UserToken::Info i = testee.checkToken("foo", afl::base::Nothing, false);
        TS_ASSERT_EQUALS(i.userId, "y");
        TS_ASSERT_EQUALS(i.tokenType, "api");
        TS_ASSERT_EQUALS(i.newToken.isValid(), false);
    }

    // clearToken
    const String_t types[] = { "a", "b" };
    mock.expectCall("RESETTOKEN, u99, a, b");
    mock.provideNewResult(0);
    TS_ASSERT_THROWS_NOTHING(testee.clearToken("u99", types));

    mock.expectCall("RESETTOKEN, u99");
    mock.provideNewResult(0);
    TS_ASSERT_THROWS_NOTHING(testee.clearToken("u99", afl::base::Memory<const String_t>()));

    mock.checkFinish();
}

