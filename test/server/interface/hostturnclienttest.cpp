/**
  *  \file test/server/interface/hostturnclienttest.cpp
  *  \brief Test for server::interface::HostTurnClient
  */

#include "server/interface/hostturnclient.hpp"

#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/test/commandhandler.hpp"
#include "afl/test/testrunner.hpp"
#include "server/types.hpp"

using afl::data::Hash;
using afl::data::HashValue;

/** Command tests. */
AFL_TEST("server.interface.HostTurnClient", a)
{
    afl::test::CommandHandler mock(a);
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
        a.checkEqual("01. state",         r.state, 2);
        a.checkEqual("02. output",        r.output, "turn check result here");
        a.checkEqual("03. gameId",        r.gameId, 42);
        a.checkEqual("04. slot",          r.slot, 8);
        a.checkEqual("05. previousState", r.previousState, 1);
        a.checkEqual("06. userId",        r.userId, "uu");
        a.checkEqual("07. gameName",      r.gameName, "The Game");
        a.checkEqual("08. turnNumber",    r.turnNumber, 27);
        a.checkEqual("09. allowTemp",     r.allowTemp, true);
    }

    // - partial [no result]
    {
        mock.expectCall("TRN, content2, GAME, 7");
        mock.provideNewResult(0);

        server::interface::HostTurn::Result r = testee.submit("content2", 7, afl::base::Nothing, afl::base::Nothing, afl::base::Nothing);
        a.checkEqual("11. state",         r.state, 0);
        a.checkEqual("12. output",        r.output, "");
        a.checkEqual("13. gameId",        r.gameId, 0);
        a.checkEqual("14. slot",          r.slot, 0);
        a.checkEqual("15. previousState", r.previousState, 0);
        a.checkEqual("16. userId",        r.userId, "");
        a.checkEqual("17. gameName",      r.gameName, "");
        a.checkEqual("18. turnNumber",    r.turnNumber, 0);
        a.checkEqual("19. allowTemp",     r.allowTemp, false);
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
