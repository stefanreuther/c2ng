/**
  *  \file u/t_server_interface_hostturnclient.cpp
  *  \brief Test for server::interface::HostTurnClient
  */

#include "server/interface/hostturnclient.hpp"

#include "t_server_interface.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/test/commandhandler.hpp"
#include "server/types.hpp"

using afl::data::Hash;
using afl::data::HashValue;

/** Command tests. */
void
TestServerInterfaceHostTurnClient::testIt()
{
    afl::test::CommandHandler mock("testIt");
    server::interface::HostTurnClient testee(mock);

    // TRN
    // - full
    {
        Hash::Ref_t h = Hash::create();
        h->setNew("status",   server::makeIntegerValue(2));
        h->setNew("output",   server::makeStringValue("turn check result here"));
        h->setNew("game",     server::makeIntegerValue(42));
        h->setNew("slot",     server::makeIntegerValue(8));
        h->setNew("previous", server::makeIntegerValue(1));
        h->setNew("user",     server::makeStringValue("uu"));
        h->setNew("name",     server::makeStringValue("The Game"));
        h->setNew("turn",     server::makeIntegerValue(27));
        h->setNew("allowtemp", server::makeIntegerValue(1));

        mock.expectCall("TRN, content, GAME, 99, SLOT, 7, MAIL, u@h.d, INFO, detail");
        mock.provideNewResult(new HashValue(h));

        server::interface::HostTurn::Result r = testee.submit("content", 99, 7, String_t("u@h.d"), String_t("detail"));
        TS_ASSERT_EQUALS(r.state, 2);
        TS_ASSERT_EQUALS(r.output, "turn check result here");
        TS_ASSERT_EQUALS(r.gameId, 42);
        TS_ASSERT_EQUALS(r.slot, 8);
        TS_ASSERT_EQUALS(r.previousState, 1);
        TS_ASSERT_EQUALS(r.userId, "uu");
        TS_ASSERT_EQUALS(r.gameName, "The Game");
        TS_ASSERT_EQUALS(r.turnNumber, 27);
        TS_ASSERT_EQUALS(r.allowTemp, true);
    }

    // - partial [no result]
    {
        mock.expectCall("TRN, content2, GAME, 7");
        mock.provideNewResult(0);

        server::interface::HostTurn::Result r = testee.submit("content2", 7, afl::base::Nothing, afl::base::Nothing, afl::base::Nothing);
        TS_ASSERT_EQUALS(r.state, 0);
        TS_ASSERT_EQUALS(r.output, "");
        TS_ASSERT_EQUALS(r.gameId, 0);
        TS_ASSERT_EQUALS(r.slot, 0);
        TS_ASSERT_EQUALS(r.previousState, 0);
        TS_ASSERT_EQUALS(r.userId, "");
        TS_ASSERT_EQUALS(r.gameName, "");
        TS_ASSERT_EQUALS(r.turnNumber, 0);
        TS_ASSERT_EQUALS(r.allowTemp, false);
    }

    // - partial
    {
        mock.expectCall("TRN, content3, INFO, zz");
        mock.provideNewResult(0);
        testee.submit("content3", afl::base::Nothing, afl::base::Nothing, afl::base::Nothing, String_t("zz"));
    }

    // - minimum args
    {
        mock.expectCall("TRN, content4");
        mock.provideNewResult(0);
        testee.submit("content4", afl::base::Nothing, afl::base::Nothing, afl::base::Nothing, afl::base::Nothing);
    }

    // TRNMARKTEMP
    mock.expectCall("TRNMARKTEMP, 9, 11, 0");
    mock.provideNewResult(0);
    testee.setTemporary(9, 11, false);

    mock.expectCall("TRNMARKTEMP, 154, 2, 1");
    mock.provideNewResult(0);
    testee.setTemporary(154, 2, true);

    mock.checkFinish();
}

