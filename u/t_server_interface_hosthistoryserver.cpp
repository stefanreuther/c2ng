/**
  *  \file u/t_server_interface_hosthistoryserver.cpp
  *  \brief Test for server::interface::HostHistoryServer
  */

#include <stdexcept>
#include "server/interface/hosthistoryserver.hpp"

#include "t_server_interface.hpp"
#include "afl/test/callreceiver.hpp"
#include "server/interface/hosthistory.hpp"
#include "afl/string/format.hpp"
#include "afl/data/access.hpp"
#include "server/interface/hosthistoryclient.hpp"

using afl::container::PtrVector;
using afl::data::Access;
using afl::data::Segment;
using afl::string::Format;
using server::Value_t;
using server::interface::HostHistory;

namespace {
    class HostHistoryMock : public server::interface::HostHistory, public afl::test::CallReceiver {
     public:
        HostHistoryMock(afl::test::Assert a)
            : HostHistory(),
              CallReceiver(a)
            { }

        virtual void getEvents(const EventFilter& filter, afl::container::PtrVector<Event>& result)
            {
                // Verify call
                String_t call = "getEvents";
                if (const int32_t* p = filter.gameId.get()) {
                    call += Format(" gameId=%d", *p);
                }
                if (const String_t* p = filter.userId.get()) {
                    call += Format(" userId=%s", *p);
                }
                if (const int32_t* p = filter.limit.get()) {
                    call += Format(" limit=%d", *p);
                }
                checkCall(call);

                // Produce result
                int n = consumeReturnValue<int>();
                while (n-- > 0) {
                    result.pushBackNew(new Event(consumeReturnValue<Event>()));
                }
            }

        virtual void getTurns(int32_t gameId, const TurnFilter& filter, afl::container::PtrVector<Turn>& result)
            {
                // Verify call
                String_t call = Format("getTurns %d", gameId);
                if (const int32_t* p = filter.endTurn.get()) {
                    call += Format(" endTurn=%d", *p);
                }
                if (const int32_t* p = filter.limit.get()) {
                    call += Format(" limit=%d", *p);
                }
                if (const int32_t* p = filter.startTime.get()) {
                    call += Format(" startTime=%d", *p);
                }
                if (const String_t* p = filter.scoreName.get()) {
                    call += Format(" scoreName=%s", *p);
                }
                if (filter.reportPlayers) {
                    call += " reportPlayers";
                }
                if (filter.reportStatus) {
                    call += " reportStatus";
                }
                checkCall(call);

                // Produce result
                int n = consumeReturnValue<int>();
                while (n-- > 0) {
                    result.pushBackNew(new Turn(consumeReturnValue<Turn>()));
                }
            }
    };

}

/** Simple functionality test. */
void
TestServerInterfaceHostHistoryServer::testIt()
{
    HostHistoryMock mock("TestServerInterfaceHostHistoryServer::testIt");
    server::interface::HostHistoryServer testee(mock);

    // HISTEVENTS - full
    {
        HostHistory::Event e1;
        e1.time = 99;
        e1.eventType = "game-state";
        e1.gameId = 42;
        e1.gameName = "Three";
        e1.userId = "jill";
        e1.slotNumber = 12;
        e1.gameState = server::interface::HostGame::Running;

        mock.expectCall("getEvents gameId=3 userId=jane limit=7");
        mock.provideReturnValue(1);
        mock.provideReturnValue(e1);

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("HISTEVENTS")
                                             .pushBackString("LIMIT").pushBackInteger(7)
                                             .pushBackString("GAME").pushBackInteger(3)
                                             .pushBackString("USER").pushBackString("jane")));
        Access a(p);
        TS_ASSERT_EQUALS(a.getArraySize(), 1U);
        TS_ASSERT_EQUALS(a[0]("time").toInteger(), 99);
        TS_ASSERT_EQUALS(a[0]("event").toString(), "game-state");
        TS_ASSERT_EQUALS(a[0]("game").toInteger(), 42);
        TS_ASSERT_EQUALS(a[0]("gameName").toString(), "Three");
        TS_ASSERT_EQUALS(a[0]("user").toString(), "jill");
        TS_ASSERT_EQUALS(a[0]("slot").toInteger(), 12);
        TS_ASSERT_EQUALS(a[0]("state").toString(), "running");
    }

    // HISTEVENTS - empty
    // Return value must not be null, but an empty array.
    {
        mock.expectCall("getEvents");
        mock.provideReturnValue(0);

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("HISTEVENTS")));

        TS_ASSERT(p.get() != 0);
        TS_ASSERT_EQUALS(Access(p).getArraySize(), 0U);
    }

    // HISTTURN - full
    {
        HostHistory::Turn t1;
        t1.turnNumber = 12;
        t1.slotPlayers.push_back("u");
        t1.slotPlayers.push_back("v");
        t1.slotStates.push_back(2);
        t1.slotStates.push_back(7);
        t1.slotStates.push_back(9);
        t1.slotScores.push_back(66666);
        t1.slotScores.push_back(-1);
        t1.time = 88;
        t1.timestamp = "88-77-6655:44:33";

        mock.expectCall("getTurns 84 endTurn=17 limit=9 startTime=99999 scoreName=tim reportPlayers reportStatus");
        mock.provideReturnValue(1);
        mock.provideReturnValue(t1);

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("HISTTURN")
                                             .pushBackInteger(84)
                                             .pushBackString("PLAYER")
                                             .pushBackString("SCORE").pushBackString("tim")
                                             .pushBackString("STATUS")
                                             .pushBackString("SINCETIME").pushBackInteger(99999)
                                             .pushBackString("LIMIT").pushBackInteger(9)
                                             .pushBackString("UNTIL").pushBackInteger(17)));
        Access a(p);
        TS_ASSERT_EQUALS(a.getArraySize(), 1U);
        TS_ASSERT_EQUALS(a[0]("turn").toInteger(), 12);
        TS_ASSERT_EQUALS(a[0]("players").getArraySize(), 2U);
        TS_ASSERT_EQUALS(a[0]("players")[0].toString(), "u");
        TS_ASSERT_EQUALS(a[0]("players")[1].toString(), "v");
        TS_ASSERT_EQUALS(a[0]("turns").getArraySize(), 3U);
        TS_ASSERT_EQUALS(a[0]("turns")[0].toInteger(), 2);
        TS_ASSERT_EQUALS(a[0]("turns")[1].toInteger(), 7);
        TS_ASSERT_EQUALS(a[0]("turns")[2].toInteger(), 9);
        TS_ASSERT_EQUALS(a[0]("scores").getArraySize(), 2U);
        TS_ASSERT_EQUALS(a[0]("scores")[0].toInteger(), 66666);
        TS_ASSERT_EQUALS(a[0]("scores")[1].toInteger(), -1);
        TS_ASSERT_EQUALS(a[0]("time").toInteger(), 88);
        TS_ASSERT_EQUALS(a[0]("timestamp").toString(), "88-77-6655:44:33");
    }

    // HISTTURN - empty
    {
        mock.expectCall("getTurns 84");
        mock.provideReturnValue(0);

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("HISTTURN").pushBackInteger(84)));

        TS_ASSERT(p.get() != 0);
        TS_ASSERT_EQUALS(Access(p).getArraySize(), 0U);
    }

    // Variant
    {
        mock.expectCall("getTurns 12 endTurn=99 reportPlayers");
        mock.provideReturnValue(0);
        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("histturn")
                                             .pushBackInteger(12)
                                             .pushBackString("player")
                                             .pushBackString("Until").pushBackInteger(99)));

        TS_ASSERT(p.get() != 0);
        TS_ASSERT_EQUALS(Access(p).getArraySize(), 0U);
    }

    mock.checkFinish();
}

/** Test error cases. */
void
TestServerInterfaceHostHistoryServer::testErrors()
{
    HostHistoryMock mock("TestServerInterfaceHostHistoryServer::testErrors");
    server::interface::HostHistoryServer testee(mock);

    Segment empty;
    TS_ASSERT_THROWS(testee.callVoid(empty), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("HISTTURN")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("HISTTURN").pushBackString("NaN")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("HISTTURN").pushBackInteger(12).pushBackString("PLAYER")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("HISTTURN").pushBackInteger(12).pushBackString("FOO")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("HISTEVENTS").pushBackString("USER")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("HISTEVENTS").pushBackString("FOO")), std::exception);
}

/** Test round-trip compatibility with HostHistoryClient. */
void
TestServerInterfaceHostHistoryServer::testRoundtrip()
{
    HostHistoryMock mock("TestServerInterfaceHostHistoryServer::testRoundtrip");
    server::interface::HostHistoryServer level1(mock);
    server::interface::HostHistoryClient level2(level1);
    server::interface::HostHistoryServer level3(level2);
    server::interface::HostHistoryClient level4(level3);

    // HISTEVENTS - full
    {
        HostHistory::Event e1;
        e1.time = 99;
        e1.eventType = "game-state";
        e1.gameId = 42;
        e1.gameName = "Three";
        e1.userId = "jill";
        e1.slotNumber = 12;
        e1.gameState = server::interface::HostGame::Running;

        mock.expectCall("getEvents gameId=3 userId=jane limit=7");
        mock.provideReturnValue(1);
        mock.provideReturnValue(e1);

        HostHistory::EventFilter filter;
        filter.limit = 7;
        filter.gameId = 3;
        filter.userId = "jane";

        PtrVector<HostHistory::Event> result;
        TS_ASSERT_THROWS_NOTHING(level4.getEvents(filter, result));

        TS_ASSERT_EQUALS(result.size(), 1U);
        TS_ASSERT(result[0] != 0);
        TS_ASSERT_EQUALS(result[0]->time, 99);
        TS_ASSERT_EQUALS(result[0]->eventType, "game-state");
        TS_ASSERT_EQUALS(result[0]->gameId.orElse(-1), 42);
        TS_ASSERT_EQUALS(result[0]->gameName.orElse(""), "Three");
        TS_ASSERT_EQUALS(result[0]->userId.orElse(""), "jill");
        TS_ASSERT_EQUALS(result[0]->slotNumber.orElse(-1), 12);
        TS_ASSERT_EQUALS(result[0]->gameState.isValid(), true);
        TS_ASSERT_EQUALS(*result[0]->gameState.get(), server::interface::HostGame::Running);
    }

    // HISTTURN - full
    {
        HostHistory::Turn t1;
        t1.turnNumber = 12;
        t1.slotPlayers.push_back("u");
        t1.slotPlayers.push_back("v");
        t1.slotStates.push_back(2);
        t1.slotStates.push_back(7);
        t1.slotStates.push_back(9);
        t1.slotScores.push_back(66666);
        t1.slotScores.push_back(-1);
        t1.time = 88;
        t1.timestamp = "88-77-6655:44:33";

        mock.expectCall("getTurns 84 endTurn=17 limit=9 startTime=1952 scoreName=tim reportPlayers reportStatus");
        mock.provideReturnValue(1);
        mock.provideReturnValue(t1);

        HostHistory::TurnFilter filter;
        filter.reportPlayers = true;
        filter.reportStatus = true;
        filter.scoreName = "tim";
        filter.endTurn = 17;
        filter.limit = 9;
        filter.startTime = 1952;

        PtrVector<HostHistory::Turn> result;
        TS_ASSERT_THROWS_NOTHING(level4.getTurns(84, filter, result));

        TS_ASSERT_EQUALS(result.size(), 1U);
        TS_ASSERT(result[0] != 0);

        TS_ASSERT_EQUALS(result[0]->turnNumber, 12);
        TS_ASSERT_EQUALS(result[0]->slotPlayers.size(), 2U);
        TS_ASSERT_EQUALS(result[0]->slotPlayers[0], "u");
        TS_ASSERT_EQUALS(result[0]->slotPlayers[1], "v");
        TS_ASSERT_EQUALS(result[0]->slotStates.size(), 3U);
        TS_ASSERT_EQUALS(result[0]->slotStates[0], 2);
        TS_ASSERT_EQUALS(result[0]->slotStates[1], 7);
        TS_ASSERT_EQUALS(result[0]->slotStates[2], 9);
        TS_ASSERT_EQUALS(result[0]->slotScores.size(), 2U);
        TS_ASSERT_EQUALS(result[0]->slotScores[0], 66666);
        TS_ASSERT_EQUALS(result[0]->slotScores[1], -1);
        TS_ASSERT_EQUALS(result[0]->time, 88);
        TS_ASSERT_EQUALS(result[0]->timestamp, "88-77-6655:44:33");
    }
}

