/**
  *  \file test/server/interface/hostcronservertest.cpp
  *  \brief Test for server::interface::HostCronServer
  */

#include "server/interface/hostcronserver.hpp"

#include "afl/data/access.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"
#include "server/interface/hostcronclient.hpp"
#include <stdexcept>

using afl::string::Format;
using afl::data::Segment;
using server::interface::HostCron;

namespace {
    class HostCronMock : public HostCron, public afl::test::CallReceiver {
     public:
        HostCronMock(afl::test::Assert a)
            : CallReceiver(a)
            { }
        virtual Event getGameEvent(int32_t gameId)
            {
                checkCall(Format("get(%d)", gameId));
                return consumeReturnValue<Event>();
            }

        virtual void listGameEvents(afl::base::Optional<int32_t> limit, std::vector<Event>& result)
            {
                checkCall(Format("list(%d)", limit.orElse(-1)));
                int n = consumeReturnValue<int>();
                while (n-- > 0) {
                    result.push_back(consumeReturnValue<Event>());
                }
            }

        virtual bool kickstartGame(int32_t gameId)
            {
                checkCall(Format("kick(%d)", gameId));
                return consumeReturnValue<bool>();
            }
        virtual void suspendScheduler(int32_t relativeTime)
            {
                checkCall(Format("suspend(%d)", relativeTime));
            }
        virtual void getBrokenGames(BrokenMap_t& result)
            {
                checkCall("getBrokenGames()");
                int n = consumeReturnValue<int>();
                while (n-- > 0) {
                    int gid = consumeReturnValue<int>();
                    result[gid] = consumeReturnValue<String_t>();
                }
            }
    };
}


/** Test HostCronServer against a mock. */
AFL_TEST("server.interface.HostCronServer", a)
{
    HostCronMock mock(a);
    server::interface::HostCronServer testee(mock);

    // CRONGET
    {
        mock.expectCall("get(3)");
        mock.provideReturnValue(HostCron::Event(3, HostCron::MasterAction, 99));

        std::auto_ptr<server::Value_t> p(testee.call(Segment().pushBackString("CRONGET").pushBackInteger(3)));
        afl::data::Access ap(p);
        a.checkEqual("01. action", ap("action").toString(), "master");
        a.checkEqual("02. game",   ap("game").toInteger(), 3);
        a.checkEqual("03. time",   ap("time").toInteger(), 99);
    }

    // CRONLIST
    // - no limit
    {
        mock.expectCall("list(-1)");
        mock.provideReturnValue(2);
        mock.provideReturnValue(HostCron::Event(1, HostCron::HostAction, 22));
        mock.provideReturnValue(HostCron::Event(2, HostCron::ScheduleChangeAction, 33));

        std::auto_ptr<server::Value_t> p(testee.call(Segment().pushBackString("CRONLIST")));
        afl::data::Access ap(p);
        a.checkEqual("11. getArraySize", ap.getArraySize(), 2U);
        a.checkEqual("12. action", ap[0]("action").toString(), "host");
        a.checkEqual("13. game",   ap[0]("game").toInteger(), 1);
        a.checkEqual("14. time",   ap[0]("time").toInteger(), 22);
        a.checkEqual("15. action", ap[1]("action").toString(), "schedulechange");
        a.checkEqual("16. game",   ap[1]("game").toInteger(), 2);
        a.checkEqual("17. time",   ap[1]("time").toInteger(), 33);
    }

    // - with limit
    {
        mock.expectCall("list(7)");
        mock.provideReturnValue(0);

        std::auto_ptr<server::Value_t> p(testee.call(Segment().pushBackString("CRONLIST").pushBackString("LIMIT").pushBackInteger(7)));
        afl::data::Access ap(p);
        a.checkEqual("21. getArraySize", ap.getArraySize(), 0U);
    }

    // CRONKICK
    {
        mock.expectCall("kick(12)");
        mock.provideReturnValue(true);
        a.checkEqual("31. cronkick", testee.callInt(Segment().pushBackString("CRONKICK").pushBackInteger(12)), 1);

        mock.expectCall("kick(13)");
        mock.provideReturnValue(false);
        a.checkEqual("41. cronkick", testee.callInt(Segment().pushBackString("CRONKICK").pushBackInteger(13)), 0);
    }

    // CRONSUSPEND
    {
        mock.expectCall("suspend(0)");
        AFL_CHECK_SUCCEEDS(a("51. cronsuspend"), testee.callVoid(Segment().pushBackString("CRONSUSPEND").pushBackInteger(0)));

        mock.expectCall("suspend(9999)");
        AFL_CHECK_SUCCEEDS(a("61. cronsuspend"), testee.callVoid(Segment().pushBackString("CRONSUSPEND").pushBackInteger(9999)));
    }

    // CRONLSBROKEN
    {
        mock.expectCall("getBrokenGames()");
        mock.provideReturnValue(2);
        mock.provideReturnValue(42);
        mock.provideReturnValue(String_t("first excuse"));
        mock.provideReturnValue(77);
        mock.provideReturnValue(String_t("second excuse"));

        std::auto_ptr<server::Value_t> p;
        AFL_CHECK_SUCCEEDS(a("71. cronlsbroken"), p.reset(testee.call(Segment().pushBackString("CRONLSBROKEN"))));
        afl::data::Access ap(p);

        a.checkEqual("81. getArraySize", ap.getArraySize(), 4U);
        a.checkEqual("82. id",   ap[0].toInteger(), 42);
        a.checkEqual("83. text", ap[1].toString(), "first excuse");
        a.checkEqual("84. id",   ap[2].toInteger(), 77);
        a.checkEqual("85. text", ap[3].toString(), "second excuse");
    }

    // Variations
    mock.expectCall("kick(77)");
    mock.provideReturnValue(false);
    a.checkEqual("91. cronkick", testee.callInt(Segment().pushBackString("cronkick").pushBackInteger(77)), 0);

    mock.expectCall("list(5)");
    mock.provideReturnValue(0);
    AFL_CHECK_SUCCEEDS(a("101. cronlist"), testee.callVoid(Segment().pushBackString("cronlist").pushBackString("limit").pushBackInteger(5)));

    mock.checkFinish();
}

/** Test erroneous invocations. */
AFL_TEST("server.interface.HostCronServer:errors", a)
{
    HostCronMock mock(a);
    server::interface::HostCronServer testee(mock);

    // Bad arg count
    Segment empty;    // g++-3.4 sees an invocation of a copy constructor if I construct this object in-place.
    AFL_CHECK_THROWS(a("01. empty"), testee.callVoid(empty), std::exception);
    AFL_CHECK_THROWS(a("02. missing arg"), testee.callVoid(Segment().pushBackString("CRONKICK")), std::exception);
    AFL_CHECK_THROWS(a("03. missing option"), testee.callVoid(Segment().pushBackString("CRONLIST").pushBackString("LIMIT")), std::exception);
    AFL_CHECK_THROWS(a("04. missing arg"), testee.callVoid(Segment().pushBackString("CRONSUSPEND")), std::exception);

    // Bad keywords
    AFL_CHECK_THROWS(a("11. bad keyword"), testee.callVoid(Segment().pushBackString("CRONLIST").pushBackString("")), std::exception);
    AFL_CHECK_THROWS(a("12. bad keyword"), testee.callVoid(Segment().pushBackString("CRONLIST").pushBackString("X")), std::exception);
    AFL_CHECK_THROWS(a("13. bad keyword"), testee.callVoid(Segment().pushBackString("X")), std::exception);
}

AFL_TEST("server.interface.HostCronServer:roundtrip", a)
{
    HostCronMock mock(a);
    server::interface::HostCronServer level1(mock);
    server::interface::HostCronClient level2(level1);
    server::interface::HostCronServer level3(level2);
    server::interface::HostCronClient level4(level3);

    // get
    {
        mock.expectCall("get(42)");
        mock.provideReturnValue(HostCron::Event(42, HostCron::NoAction, 3));

        HostCron::Event e = level4.getGameEvent(42);
        a.checkEqual("01. gameId", e.gameId, 42);
        a.checkEqual("02. action", e.action, HostCron::NoAction);
        a.checkEqual("03. time",   e.time, 3);
    }

    // list
    {
        mock.expectCall("list(-1)");
        mock.provideReturnValue(2);
        mock.provideReturnValue(HostCron::Event(1, HostCron::UnknownAction, 1010));
        mock.provideReturnValue(HostCron::Event(2, HostCron::MasterAction, 2020));

        std::vector<HostCron::Event> result;
        level4.listGameEvents(afl::base::Nothing, result);

        a.checkEqual("11. size", result.size(), 2U);
        a.checkEqual("12. gameId", result[0].gameId, 1);
        a.checkEqual("13. action", result[0].action, HostCron::UnknownAction);
        a.checkEqual("14. time",   result[0].time, 1010);
        a.checkEqual("15. gameId", result[1].gameId, 2);
        a.checkEqual("16. action", result[1].action, HostCron::MasterAction);
        a.checkEqual("17. time",   result[1].time, 2020);
    }

    // list
    {
        mock.expectCall("list(8)");
        mock.provideReturnValue(1);
        mock.provideReturnValue(HostCron::Event(7, HostCron::HostAction, 777));

        std::vector<HostCron::Event> result;
        level4.listGameEvents(8, result);

        a.checkEqual("21. size", result.size(), 1U);
        a.checkEqual("22. gameId", result[0].gameId, 7);
        a.checkEqual("23. action", result[0].action, HostCron::HostAction);
        a.checkEqual("24. time",   result[0].time, 777);
    }

    // kick
    mock.expectCall("kick(12)");
    mock.provideReturnValue(true);
    a.check("31. kickstartGame", level4.kickstartGame(12));

    mock.expectCall("kick(17)");
    mock.provideReturnValue(false);
    a.check("41. kickstartGame", !level4.kickstartGame(17));

    // suspend
    mock.expectCall("suspend(3)");
    AFL_CHECK_SUCCEEDS(a("51. suspendScheduler"), level4.suspendScheduler(3));

    // getBrokenGames
    {
        mock.expectCall("getBrokenGames()");
        mock.provideReturnValue(2);
        mock.provideReturnValue(42);
        mock.provideReturnValue(String_t("first excuse"));
        mock.provideReturnValue(77);
        mock.provideReturnValue(String_t("second excuse"));

        HostCron::BrokenMap_t result;
        AFL_CHECK_SUCCEEDS(a("61. getBrokenGames"), level4.getBrokenGames(result));

        a.checkEqual("71. size", result.size(), 2U);
        a.checkEqual("72. first text",  result[42], "first excuse");
        a.checkEqual("73. second text", result[77], "second excuse");
    }
    mock.checkFinish();
}
