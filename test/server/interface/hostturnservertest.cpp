/**
  *  \file test/server/interface/hostturnservertest.cpp
  *  \brief Test for server::interface::HostTurnServer
  */

#include "server/interface/hostturnserver.hpp"

#include "afl/data/access.hpp"
#include "afl/data/segment.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"
#include "server/interface/hostturn.hpp"
#include "server/interface/hostturnclient.hpp"
#include "server/types.hpp"
#include <stdexcept>

using afl::string::Format;
using afl::data::Segment;
using afl::data::Access;
using server::interface::HostTurn;
using server::Value_t;

namespace {
    class HostTurnMock : public HostTurn, public afl::test::CallReceiver {
     public:
        HostTurnMock(afl::test::Assert a)
            : CallReceiver(a)
            { }
        virtual Result submit(const String_t& blob,
                              afl::base::Optional<int32_t> game,
                              afl::base::Optional<int32_t> slot,
                              afl::base::Optional<String_t> mail,
                              afl::base::Optional<String_t> info)
            {
                checkCall(Format("submit(%s,%d,%d,%s,%s)") << blob << game.orElse(-1) << slot.orElse(-1) << mail.orElse("-") << info.orElse("-"));
                return consumeReturnValue<Result>();
            }
        virtual void setTemporary(int32_t gameId, int32_t slot, bool flag)
            {
                checkCall(Format("setTemporary(%d,%d,%d)", gameId, slot, int(flag)));
            }
    };
}

AFL_TEST("server.interface.HostTurnServer:commands", a)
{
    HostTurnMock mock(a);
    server::interface::HostTurnServer testee(mock);

    // TRN
    {
        HostTurn::Result r;
        r.state = 9;
        r.output = "text...";
        r.gameId = 39;
        r.slot = 7;
        r.previousState = 2;
        r.userId = "u";
        r.turnNumber = 82;
        r.gameName = "Battle";
        r.allowTemp = true;
        mock.expectCall("submit(foo,-1,-1,-,-)");
        mock.provideReturnValue(r);

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("TRN").pushBackString("foo")));
        Access ap(p);
        a.checkEqual("01. status",    ap("status").toInteger(), 9);
        a.checkEqual("02. output",    ap("output").toString(), "text...");
        a.checkEqual("03. game",      ap("game").toInteger(), 39);
        a.checkEqual("04. slot",      ap("slot").toInteger(), 7);
        a.checkEqual("05. previous",  ap("previous").toInteger(), 2);
        a.checkEqual("06. user",      ap("user").toString(), "u");
        a.checkEqual("07. name",      ap("name").toString(), "Battle");
        a.checkEqual("08. turn",      ap("turn").toInteger(), 82);
        a.checkEqual("09. allowtemp", ap("allowtemp").toInteger(), 1);
    }
    {
        mock.expectCall("submit(bar,231,-1,x@y.z,-)");
        mock.provideReturnValue(HostTurn::Result());
        AFL_CHECK_SUCCEEDS(a("10. trn"), testee.callVoid(Segment().pushBackString("TRN").pushBackString("bar").
                                                         pushBackString("GAME").pushBackInteger(231).pushBackString("MAIL").pushBackString("x@y.z")));
    }
    {
        mock.expectCall("submit(baz,32768,5,a@b,log)");
        mock.provideReturnValue(HostTurn::Result());
        AFL_CHECK_SUCCEEDS(a("11. trn"), testee.callVoid(Segment().pushBackString("TRN").pushBackString("baz").
                                                         pushBackString("INFO").pushBackString("log").
                                                         pushBackString("GAME").pushBackInteger(32768).
                                                         pushBackString("MAIL").pushBackString("a@b").
                                                         pushBackString("SLOT").pushBackInteger(5)));
    }

    // TRNMARKTEMP
    mock.expectCall("setTemporary(9,3,1)");
    AFL_CHECK_SUCCEEDS(a("21. trnmarktemp"), testee.callVoid(Segment().pushBackString("TRNMARKTEMP").pushBackInteger(9).pushBackInteger(3).pushBackInteger(1)));

    // Variations
    mock.expectCall("setTemporary(9,12,1)");
    AFL_CHECK_SUCCEEDS(a("31. trnmarktemp"), testee.callVoid(Segment().pushBackString("trnmarktemp").pushBackInteger(9).pushBackString("12").pushBackInteger(1)));

    mock.expectCall("submit(baz,32768,5,a@b,log)");
    mock.provideReturnValue(HostTurn::Result());
    AFL_CHECK_SUCCEEDS(a("41. trn"), testee.callVoid(Segment().pushBackString("trn").pushBackString("baz").
                                                     pushBackString("game").pushBackInteger(32768).
                                                     pushBackString("info").pushBackString("log").
                                                     pushBackString("mail").pushBackString("a@b").
                                                     pushBackString("slot").pushBackInteger(5)));

    mock.checkFinish();
}

AFL_TEST("server.interface.HostTurnServer:errors", a)
{
    HostTurnMock mock(a);
    server::interface::HostTurnServer testee(mock);

    Segment empty;
    AFL_CHECK_THROWS(a("01. empty"),          testee.callVoid(empty), std::exception);
    AFL_CHECK_THROWS(a("02. bad verb"),       testee.callVoid(Segment().pushBackString("")), std::exception);
    AFL_CHECK_THROWS(a("03. bad verb"),       testee.callVoid(Segment().pushBackString("HI")), std::exception);
    AFL_CHECK_THROWS(a("04. missing arg"),    testee.callVoid(Segment().pushBackString("TRN")), std::exception);
    AFL_CHECK_THROWS(a("05. missing option"), testee.callVoid(Segment().pushBackString("TRN").pushBackString("data").pushBackString("MAIL")), std::exception);
    AFL_CHECK_THROWS(a("06. missing arg"),    testee.callVoid(Segment().pushBackString("TRNMARKTEMP").pushBackInteger(1)), std::exception);

    mock.checkFinish();
}

AFL_TEST("server.interface.HostTurnServer:roundtrip", a)
{
    HostTurnMock mock(a);
    server::interface::HostTurnServer level1(mock);
    server::interface::HostTurnClient level2(level1);
    server::interface::HostTurnServer level3(level2);
    server::interface::HostTurnClient level4(level3);

    // TRN
    {
        HostTurn::Result in;
        in.state = 9;
        in.output = "text...";
        in.gameId = 39;
        in.slot = 7;
        in.previousState = 2;
        in.userId = "u";
        in.turnNumber = 72;
        in.gameName = "Party";
        in.allowTemp = true;
        mock.expectCall("submit(foo,-1,-1,-,-)");
        mock.provideReturnValue(in);

        HostTurn::Result out = level4.submit("foo", afl::base::Nothing, afl::base::Nothing, afl::base::Nothing, afl::base::Nothing);
        a.checkEqual("01. state",         out.state, 9);
        a.checkEqual("02. output",        out.output, "text...");
        a.checkEqual("03. gameId",        out.gameId, 39);
        a.checkEqual("04. slot",          out.slot, 7);
        a.checkEqual("05. previousState", out.previousState, 2);
        a.checkEqual("06. userId",        out.userId, "u");
        a.checkEqual("07. turnNumber",    out.turnNumber, 72);
        a.checkEqual("08. gameName",      out.gameName, "Party");
        a.checkEqual("09. allowTemp",     out.allowTemp, true);
    }
    {
        mock.expectCall("submit(bar,231,-1,x@y.z,-)");
        mock.provideReturnValue(HostTurn::Result());
        AFL_CHECK_SUCCEEDS(a("10. submit"), level4.submit("bar", 231, afl::base::Nothing, String_t("x@y.z"), afl::base::Nothing));
    }
    {
        mock.expectCall("submit(baz,32768,5,a@b,log)");
        mock.provideReturnValue(HostTurn::Result());
        AFL_CHECK_SUCCEEDS(a("11. submit"), level4.submit("baz", 32768, 5, String_t("a@b"), String_t("log")));
    }

    // TRNMARKTEMP
    mock.expectCall("setTemporary(9,3,1)");
    AFL_CHECK_SUCCEEDS(a("21. setTemporary"), level4.setTemporary(9, 3, true));

    mock.checkFinish();
}
