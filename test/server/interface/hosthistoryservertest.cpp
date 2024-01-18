/**
  *  \file test/server/interface/hosthistoryservertest.cpp
  *  \brief Test for server::interface::HostHistoryServer
  */

#include "server/interface/hosthistoryserver.hpp"

#include "afl/data/access.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"
#include "server/interface/hosthistory.hpp"
#include "server/interface/hosthistoryclient.hpp"
#include <stdexcept>

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
AFL_TEST("server.interface.HostHistoryServer:commands", a)
{
    HostHistoryMock mock(a);
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
        Access ap(p);
        a.checkEqual("01. getArraySize", ap.getArraySize(), 1U);
        a.checkEqual("02. time",         ap[0]("time").toInteger(), 99);
        a.checkEqual("03. event",        ap[0]("event").toString(), "game-state");
        a.checkEqual("04. game",         ap[0]("game").toInteger(), 42);
        a.checkEqual("05. gameName",     ap[0]("gameName").toString(), "Three");
        a.checkEqual("06. user",         ap[0]("user").toString(), "jill");
        a.checkEqual("07. slot",         ap[0]("slot").toInteger(), 12);
        a.checkEqual("08. state",        ap[0]("state").toString(), "running");
    }

    // HISTEVENTS - empty
    // Return value must not be null, but an empty array.
    {
        mock.expectCall("getEvents");
        mock.provideReturnValue(0);

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("HISTEVENTS")));

        a.checkNonNull("11. histevents", p.get());
        a.checkEqual("12. getArraySize", Access(p).getArraySize(), 0U);
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
        Access ap(p);
        a.checkEqual("21. getArraySize", ap.getArraySize(), 1U);
        a.checkEqual("22. turn",      ap[0]("turn").toInteger(), 12);
        a.checkEqual("23. players",   ap[0]("players").getArraySize(), 2U);
        a.checkEqual("24. players",   ap[0]("players")[0].toString(), "u");
        a.checkEqual("25. players",   ap[0]("players")[1].toString(), "v");
        a.checkEqual("26. turns",     ap[0]("turns").getArraySize(), 3U);
        a.checkEqual("27. turns",     ap[0]("turns")[0].toInteger(), 2);
        a.checkEqual("28. turns",     ap[0]("turns")[1].toInteger(), 7);
        a.checkEqual("29. turns",     ap[0]("turns")[2].toInteger(), 9);
        a.checkEqual("30. scores",    ap[0]("scores").getArraySize(), 2U);
        a.checkEqual("31. scores",    ap[0]("scores")[0].toInteger(), 66666);
        a.checkEqual("32. scores",    ap[0]("scores")[1].toInteger(), -1);
        a.checkEqual("33. time",      ap[0]("time").toInteger(), 88);
        a.checkEqual("34. timestamp", ap[0]("timestamp").toString(), "88-77-6655:44:33");
    }

    // HISTTURN - empty
    {
        mock.expectCall("getTurns 84");
        mock.provideReturnValue(0);

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("HISTTURN").pushBackInteger(84)));

        a.checkNonNull("41. histturn", p.get());
        a.checkEqual("42. getArraySize", Access(p).getArraySize(), 0U);
    }

    // Variant
    {
        mock.expectCall("getTurns 12 endTurn=99 reportPlayers");
        mock.provideReturnValue(0);
        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("histturn")
                                             .pushBackInteger(12)
                                             .pushBackString("player")
                                             .pushBackString("Until").pushBackInteger(99)));

        a.checkNonNull("51. histturn", p.get());
        a.checkEqual("52. getArraySize", Access(p).getArraySize(), 0U);
    }

    mock.checkFinish();
}

/** Test error cases. */
AFL_TEST("server.interface.HostHistoryServer:errors", a)
{
    HostHistoryMock mock(a);
    server::interface::HostHistoryServer testee(mock);

    Segment empty;
    AFL_CHECK_THROWS(a("01. empty"),          testee.callVoid(empty), std::exception);
    AFL_CHECK_THROWS(a("02. bas verb"),       testee.callVoid(Segment().pushBackString("")), std::exception);
    AFL_CHECK_THROWS(a("03. missing arg"),    testee.callVoid(Segment().pushBackString("HISTTURN")), std::exception);
    AFL_CHECK_THROWS(a("04. bad arg"),        testee.callVoid(Segment().pushBackString("HISTTURN").pushBackString("NaN")), std::exception);
    AFL_CHECK_THROWS(a("05. missing option"), testee.callVoid(Segment().pushBackString("HISTTURN").pushBackInteger(12).pushBackString("PLAYER")), std::exception);
    AFL_CHECK_THROWS(a("06. bad option"),     testee.callVoid(Segment().pushBackString("HISTTURN").pushBackInteger(12).pushBackString("FOO")), std::exception);
    AFL_CHECK_THROWS(a("07. missing option"), testee.callVoid(Segment().pushBackString("HISTEVENTS").pushBackString("USER")), std::exception);
    AFL_CHECK_THROWS(a("08. bad option"),     testee.callVoid(Segment().pushBackString("HISTEVENTS").pushBackString("FOO")), std::exception);
}

/** Test round-trip compatibility with HostHistoryClient. */
AFL_TEST("server.interface.HostHistoryServer:roundtrip", a)
{
    HostHistoryMock mock(a);
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
        AFL_CHECK_SUCCEEDS(a("01. getEvents"), level4.getEvents(filter, result));

        a.checkEqual("11. size", result.size(), 1U);
        a.checkNonNull("12. result",   result[0]);
        a.checkEqual("13. time",       result[0]->time, 99);
        a.checkEqual("14. eventType",  result[0]->eventType, "game-state");
        a.checkEqual("15. gameId",     result[0]->gameId.orElse(-1), 42);
        a.checkEqual("16. gameName",   result[0]->gameName.orElse(""), "Three");
        a.checkEqual("17. userId",     result[0]->userId.orElse(""), "jill");
        a.checkEqual("18. slotNumber", result[0]->slotNumber.orElse(-1), 12);
        a.checkEqual("19. gameState",  result[0]->gameState.isValid(), true);
        a.checkEqual("20. gameState", *result[0]->gameState.get(), server::interface::HostGame::Running);
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
        AFL_CHECK_SUCCEEDS(a("21. getTurns"), level4.getTurns(84, filter, result));

        a.checkEqual("31. size", result.size(), 1U);
        a.checkNonNull("32. result", result[0]);

        a.checkEqual("41. turnNumber",  result[0]->turnNumber, 12);
        a.checkEqual("42. slotPlayers", result[0]->slotPlayers.size(), 2U);
        a.checkEqual("43. slotPlayers", result[0]->slotPlayers[0], "u");
        a.checkEqual("44. slotPlayers", result[0]->slotPlayers[1], "v");
        a.checkEqual("45. slotStates",  result[0]->slotStates.size(), 3U);
        a.checkEqual("46. slotStates",  result[0]->slotStates[0], 2);
        a.checkEqual("47. slotStates",  result[0]->slotStates[1], 7);
        a.checkEqual("48. slotStates",  result[0]->slotStates[2], 9);
        a.checkEqual("49. slotScores",  result[0]->slotScores.size(), 2U);
        a.checkEqual("50. slotScores",  result[0]->slotScores[0], 66666);
        a.checkEqual("51. slotScores",  result[0]->slotScores[1], -1);
        a.checkEqual("52. time",        result[0]->time, 88);
        a.checkEqual("53. timestamp",   result[0]->timestamp, "88-77-6655:44:33");
    }
}
