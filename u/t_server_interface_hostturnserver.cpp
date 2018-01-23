/**
  *  \file u/t_server_interface_hostturnserver.cpp
  *  \brief Test for server::interface::HostTurnServer
  */

#include "server/interface/hostturnserver.hpp"

#include <stdexcept>
#include "t_server_interface.hpp"
#include "afl/data/access.hpp"
#include "afl/data/segment.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "server/interface/hostturn.hpp"
#include "server/interface/hostturnclient.hpp"
#include "server/types.hpp"

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

void
TestServerInterfaceHostTurnServer::testIt()
{
    HostTurnMock mock("testIt");
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
        mock.expectCall("submit(foo,-1,-1,-,-)");
        mock.provideReturnValue(r);

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("TRN").pushBackString("foo")));
        Access a(p);
        TS_ASSERT_EQUALS(a("status").toInteger(), 9);
        TS_ASSERT_EQUALS(a("output").toString(), "text...");
        TS_ASSERT_EQUALS(a("game").toInteger(), 39);
        TS_ASSERT_EQUALS(a("slot").toInteger(), 7);
        TS_ASSERT_EQUALS(a("previous").toInteger(), 2);
        TS_ASSERT_EQUALS(a("user").toString(), "u");
    }
    {
        mock.expectCall("submit(bar,231,-1,x@y.z,-)");
        mock.provideReturnValue(HostTurn::Result());
        TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("TRN").pushBackString("bar").
                                                 pushBackString("GAME").pushBackInteger(231).pushBackString("MAIL").pushBackString("x@y.z")));
    }
    {
        mock.expectCall("submit(baz,32768,5,a@b,log)");
        mock.provideReturnValue(HostTurn::Result());
        TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("TRN").pushBackString("baz").
                                                 pushBackString("INFO").pushBackString("log").
                                                 pushBackString("GAME").pushBackInteger(32768).
                                                 pushBackString("MAIL").pushBackString("a@b").
                                                 pushBackString("SLOT").pushBackInteger(5)));
    }

    // TRNMARKTEMP
    mock.expectCall("setTemporary(9,3,1)");
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("TRNMARKTEMP").pushBackInteger(9).pushBackInteger(3).pushBackInteger(1)));

    // Variations
    mock.expectCall("setTemporary(9,12,1)");
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("trnmarktemp").pushBackInteger(9).pushBackString("12").pushBackInteger(1)));

    mock.expectCall("submit(baz,32768,5,a@b,log)");
    mock.provideReturnValue(HostTurn::Result());
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("trn").pushBackString("baz").
                                             pushBackString("game").pushBackInteger(32768).
                                             pushBackString("info").pushBackString("log").
                                             pushBackString("mail").pushBackString("a@b").
                                             pushBackString("slot").pushBackInteger(5)));

    mock.checkFinish();
}

void
TestServerInterfaceHostTurnServer::testErrors()
{
    HostTurnMock mock("testErrors");
    server::interface::HostTurnServer testee(mock);

    Segment empty;
    TS_ASSERT_THROWS(testee.callVoid(empty), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("HI")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("TRN")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("TRN").pushBackString("data").pushBackString("MAIL")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("TRNMARKTEMP").pushBackInteger(1)), std::exception);

    mock.checkFinish();
}

void
TestServerInterfaceHostTurnServer::testRoundtrip()
{
    HostTurnMock mock("testRoundtrip");
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
        mock.expectCall("submit(foo,-1,-1,-,-)");
        mock.provideReturnValue(in);

        HostTurn::Result out = level4.submit("foo", afl::base::Nothing, afl::base::Nothing, afl::base::Nothing, afl::base::Nothing);
        TS_ASSERT_EQUALS(out.state, 9);
        TS_ASSERT_EQUALS(out.output, "text...");
        TS_ASSERT_EQUALS(out.gameId, 39);
        TS_ASSERT_EQUALS(out.slot, 7);
        TS_ASSERT_EQUALS(out.previousState, 2);
        TS_ASSERT_EQUALS(out.userId, "u");
    }
    {
        mock.expectCall("submit(bar,231,-1,x@y.z,-)");
        mock.provideReturnValue(HostTurn::Result());
        TS_ASSERT_THROWS_NOTHING(level4.submit("bar", 231, afl::base::Nothing, String_t("x@y.z"), afl::base::Nothing));
    }
    {
        mock.expectCall("submit(baz,32768,5,a@b,log)");
        mock.provideReturnValue(HostTurn::Result());
        TS_ASSERT_THROWS_NOTHING(level4.submit("baz", 32768, 5, String_t("a@b"), String_t("log")));
    }

    // TRNMARKTEMP
    mock.expectCall("setTemporary(9,3,1)");
    TS_ASSERT_THROWS_NOTHING(level4.setTemporary(9, 3, true));

    mock.checkFinish();
}

