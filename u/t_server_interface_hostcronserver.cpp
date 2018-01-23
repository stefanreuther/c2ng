/**
  *  \file u/t_server_interface_hostcronserver.cpp
  *  \brief Test for server::interface::HostCronServer
  */

#include "server/interface/hostcronserver.hpp"

#include <stdexcept>
#include "t_server_interface.hpp"
#include "afl/data/access.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "server/interface/hostcronclient.hpp"

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
    };
}


/** Test HostCronServer against a mock. */
void
TestServerInterfaceHostCronServer::testIt()
{
    HostCronMock mock("testIt");
    server::interface::HostCronServer testee(mock);

    // CRONGET
    {
        mock.expectCall("get(3)");
        mock.provideReturnValue(HostCron::Event(3, HostCron::MasterAction, 99));

        std::auto_ptr<server::Value_t> p(testee.call(Segment().pushBackString("CRONGET").pushBackInteger(3)));
        afl::data::Access a(p);
        TS_ASSERT_EQUALS(a("action").toString(), "master");
        TS_ASSERT_EQUALS(a("game").toInteger(), 3);
        TS_ASSERT_EQUALS(a("time").toInteger(), 99);
    }

    // CRONLIST
    // - no limit
    {
        mock.expectCall("list(-1)");
        mock.provideReturnValue(2);
        mock.provideReturnValue(HostCron::Event(1, HostCron::HostAction, 22));
        mock.provideReturnValue(HostCron::Event(2, HostCron::ScheduleChangeAction, 33));

        std::auto_ptr<server::Value_t> p(testee.call(Segment().pushBackString("CRONLIST")));
        afl::data::Access a(p);
        TS_ASSERT_EQUALS(a.getArraySize(), 2U);
        TS_ASSERT_EQUALS(a[0]("action").toString(), "host");
        TS_ASSERT_EQUALS(a[0]("game").toInteger(), 1);
        TS_ASSERT_EQUALS(a[0]("time").toInteger(), 22);
        TS_ASSERT_EQUALS(a[1]("action").toString(), "schedulechange");
        TS_ASSERT_EQUALS(a[1]("game").toInteger(), 2);
        TS_ASSERT_EQUALS(a[1]("time").toInteger(), 33);
    }

    // - with limit
    {
        mock.expectCall("list(7)");
        mock.provideReturnValue(0);

        std::auto_ptr<server::Value_t> p(testee.call(Segment().pushBackString("CRONLIST").pushBackString("LIMIT").pushBackInteger(7)));
        afl::data::Access a(p);
        TS_ASSERT_EQUALS(a.getArraySize(), 0U);
    }

    // CRONKICK
    {
        mock.expectCall("kick(12)");
        mock.provideReturnValue(true);
        TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("CRONKICK").pushBackInteger(12)), 1);

        mock.expectCall("kick(13)");
        mock.provideReturnValue(false);
        TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("CRONKICK").pushBackInteger(13)), 0);
    }

    // Variations
    mock.expectCall("kick(77)");
    mock.provideReturnValue(false);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("cronkick").pushBackInteger(77)), 0);

    mock.expectCall("list(5)");
    mock.provideReturnValue(0);
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("cronlist").pushBackString("limit").pushBackInteger(5)));

    mock.checkFinish();
}

/** Test erroneous invocations. */
void
TestServerInterfaceHostCronServer::testErrors()
{
    HostCronMock mock("testErrors");
    server::interface::HostCronServer testee(mock);

    // Bad arg count
    Segment empty;    // g++-3.4 sees an invocation of a copy constructor if I construct this object in-place.
    TS_ASSERT_THROWS(testee.callVoid(empty), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("CRONKICK")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("CRONLIST").pushBackString("LIMIT")), std::exception);

    // Bad keywords
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("CRONLIST").pushBackString("")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("CRONLIST").pushBackString("X")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("X")), std::exception);
}

void
TestServerInterfaceHostCronServer::testRoundtrip()
{
    HostCronMock mock("testRoundtrip");
    server::interface::HostCronServer level1(mock);
    server::interface::HostCronClient level2(level1);
    server::interface::HostCronServer level3(level2);
    server::interface::HostCronClient level4(level3);

    // get
    {
        mock.expectCall("get(42)");
        mock.provideReturnValue(HostCron::Event(42, HostCron::NoAction, 3));

        HostCron::Event e = level4.getGameEvent(42);
        TS_ASSERT_EQUALS(e.gameId, 42);
        TS_ASSERT_EQUALS(e.action, HostCron::NoAction);
        TS_ASSERT_EQUALS(e.time, 3);
    }

    // list
    {
        mock.expectCall("list(-1)");
        mock.provideReturnValue(2);
        mock.provideReturnValue(HostCron::Event(1, HostCron::UnknownAction, 1010));
        mock.provideReturnValue(HostCron::Event(2, HostCron::MasterAction, 2020));

        std::vector<HostCron::Event> result;
        level4.listGameEvents(afl::base::Nothing, result);

        TS_ASSERT_EQUALS(result.size(), 2U);
        TS_ASSERT_EQUALS(result[0].gameId, 1);
        TS_ASSERT_EQUALS(result[0].action, HostCron::UnknownAction);
        TS_ASSERT_EQUALS(result[0].time, 1010);
        TS_ASSERT_EQUALS(result[1].gameId, 2);
        TS_ASSERT_EQUALS(result[1].action, HostCron::MasterAction);
        TS_ASSERT_EQUALS(result[1].time, 2020);
    }

    // list
    {
        mock.expectCall("list(8)");
        mock.provideReturnValue(1);
        mock.provideReturnValue(HostCron::Event(7, HostCron::HostAction, 777));

        std::vector<HostCron::Event> result;
        level4.listGameEvents(8, result);

        TS_ASSERT_EQUALS(result.size(), 1U);
        TS_ASSERT_EQUALS(result[0].gameId, 7);
        TS_ASSERT_EQUALS(result[0].action, HostCron::HostAction);
        TS_ASSERT_EQUALS(result[0].time, 777);
    }

    // kick
    mock.expectCall("kick(12)");
    mock.provideReturnValue(true);
    TS_ASSERT(level4.kickstartGame(12));

    mock.expectCall("kick(17)");
    mock.provideReturnValue(false);
    TS_ASSERT(!level4.kickstartGame(17));
}
