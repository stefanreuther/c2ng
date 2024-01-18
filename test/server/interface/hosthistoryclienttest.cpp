/**
  *  \file test/server/interface/hosthistoryclienttest.cpp
  *  \brief Test for server::interface::HostHistoryClient
  */

#include "server/interface/hosthistoryclient.hpp"

#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/test/commandhandler.hpp"
#include "afl/test/testrunner.hpp"

using afl::container::PtrVector;
using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Vector;
using afl::data::VectorValue;
using server::interface::HostGame;
using server::interface::HostHistory;
using server::makeIntegerValue;
using server::makeStringValue;

/** Simple test. */
AFL_TEST("server.interface.HostHistoryClient", a)
{
    afl::test::CommandHandler mock(a);
    server::interface::HostHistoryClient testee(mock);

    // getEvents
    // - null in, null out
    {
        mock.expectCall("HISTEVENTS");
        mock.provideNewResult(0);

        PtrVector<HostHistory::Event> result;
        testee.getEvents(HostHistory::EventFilter(), result);
        a.checkEqual("01. size", result.size(), 0U);
    }

    // - full in, full out
    {
        // Prepare
        mock.expectCall("HISTEVENTS, GAME, 3, USER, bill, LIMIT, 7");

        Vector::Ref_t v = Vector::create();
        Hash::Ref_t h1 = Hash::create();
        h1->setNew("time",     makeIntegerValue(1492));
        h1->setNew("event",    makeStringValue("game-join"));
        h1->setNew("game",     makeIntegerValue(42));
        h1->setNew("gameName", makeStringValue("Santa Maria"));
        h1->setNew("user",     makeStringValue("joe"));
        h1->setNew("slot",     makeIntegerValue(12));
        h1->setNew("state",    makeStringValue("joining"));
        v->pushBackNew(new HashValue(h1));

        Hash::Ref_t h2 = Hash::create();
        h2->setNew("time",     makeIntegerValue(1871));
        h2->setNew("event",    makeStringValue("game-kick"));
        h2->setNew("game",     makeIntegerValue(17));
        h2->setNew("gameName", makeStringValue("Santa Claus"));
        v->pushBackNew(new HashValue(h2));
        mock.provideNewResult(new VectorValue(v));

        // Call
        HostHistory::EventFilter filter;
        filter.gameId = 3;
        filter.userId = "bill";
        filter.limit = 7;
        PtrVector<HostHistory::Event> result;
        testee.getEvents(filter, result);

        // Verify
        a.checkEqual("11. size", result.size(), 2U);
        a.checkNonNull("12. result",   result[0]);
        a.checkEqual("13. time",       result[0]->time, 1492);
        a.checkEqual("14. eventType",  result[0]->eventType, "game-join");
        a.checkEqual("15. gameId",     result[0]->gameId.orElse(-1), 42);
        a.checkEqual("16. gameName",   result[0]->gameName.orElse(""), "Santa Maria");
        a.checkEqual("17. userId",     result[0]->userId.orElse(""), "joe");
        a.checkEqual("18. slotNumber", result[0]->slotNumber.orElse(-1), 12);
        a.checkEqual("19. gameState",  result[0]->gameState.isValid(), true);
        a.checkEqual("20. gameState", *result[0]->gameState.get(), HostGame::Joining);

        a.checkNonNull("21. result",   result[1]);
        a.checkEqual("22. time",       result[1]->time, 1871);
        a.checkEqual("23. eventType",  result[1]->eventType, "game-kick");
        a.checkEqual("24. gameId",     result[1]->gameId.orElse(-1), 17);
        a.checkEqual("25. gameName",   result[1]->gameName.orElse(""), "Santa Claus");
        a.checkEqual("26. userId",     result[1]->userId.isValid(), false);
        a.checkEqual("27. slotNumber", result[1]->slotNumber.isValid(), false);
        a.checkEqual("28. gameState",  result[1]->gameState.isValid(), false);
    }

    // getTurns
    // - null in, null out
    {
        mock.expectCall("HISTTURN, 17");
        mock.provideNewResult(0);

        PtrVector<HostHistory::Turn> result;
        testee.getTurns(17, HostHistory::TurnFilter(), result);
        a.checkEqual("31. size", result.size(), 0U);
    }

    // - full in, full out
    {
        // Prepare
        mock.expectCall("HISTTURN, 37, UNTIL, 50, LIMIT, 30, SINCETIME, 9999, SCORE, total, PLAYER, STATUS");

        Vector::Ref_t v11 = Vector::create();
        v11->pushBackString("fred");
        v11->pushBackString("wilma");
        v11->pushBackString("");

        Vector::Ref_t v12 = Vector::create();
        v12->pushBackInteger(3);
        v12->pushBackInteger(-1);
        v12->pushBackInteger(17);

        Vector::Ref_t v13 = Vector::create();
        v13->pushBackInteger(-1);
        v13->pushBackInteger(9999);
        v13->pushBackInteger(7777);
        v13->pushBackInteger(5555);

        Vector::Ref_t v = Vector::create();
        Hash::Ref_t h1 = Hash::create();
        h1->setNew("turn",      makeIntegerValue(42));
        h1->setNew("players",   new VectorValue(v11));
        h1->setNew("turns",     new VectorValue(v12));
        h1->setNew("scores",    new VectorValue(v13));
        h1->setNew("time",      makeIntegerValue(1918));
        h1->setNew("timestamp", makeStringValue("11-22-3333:44:55:66"));
        v->pushBackNew(new HashValue(h1));

        Hash::Ref_t h2 = Hash::create();
        h2->setNew("turn",      makeIntegerValue(43));
        h2->setNew("time",      makeIntegerValue(1919));
        h2->setNew("timestamp", makeStringValue("77-66-5555:44:33:22"));
        v->pushBackNew(new HashValue(h2));

        mock.provideNewResult(new VectorValue(v));

        // Call
        HostHistory::TurnFilter filter;
        filter.endTurn = 50;
        filter.limit = 30;
        filter.startTime = 9999;
        filter.scoreName = "total";
        filter.reportPlayers = true;
        filter.reportStatus = true;
        PtrVector<HostHistory::Turn> result;
        testee.getTurns(37, filter, result);

        // Verify
        a.checkEqual("41. size", result.size(), 2U);
        a.checkNonNull("42. result",    result[0]);
        a.checkEqual("43. turnNumber",  result[0]->turnNumber, 42);
        a.checkEqual("44. slotPlayers", result[0]->slotPlayers.size(), 3U);
        a.checkEqual("45. slotPlayers", result[0]->slotPlayers[0], "fred");
        a.checkEqual("46. slotPlayers", result[0]->slotPlayers[1], "wilma");
        a.checkEqual("47. slotPlayers", result[0]->slotPlayers[2], "");
        a.checkEqual("48. slotStates",  result[0]->slotStates.size(), 3U);
        a.checkEqual("49. slotStates",  result[0]->slotStates[0], 3);
        a.checkEqual("50. slotStates",  result[0]->slotStates[1], -1);
        a.checkEqual("51. slotStates",  result[0]->slotStates[2], 17);
        a.checkEqual("52. slotScores",  result[0]->slotScores.size(), 4U);
        a.checkEqual("53. slotScores",  result[0]->slotScores[0], -1);
        a.checkEqual("54. slotScores",  result[0]->slotScores[1], 9999);
        a.checkEqual("55. slotScores",  result[0]->slotScores[2], 7777);
        a.checkEqual("56. slotScores",  result[0]->slotScores[3], 5555);
        a.checkEqual("57. time",        result[0]->time, 1918);
        a.checkEqual("58. timestamp",   result[0]->timestamp, "11-22-3333:44:55:66");

        a.checkNonNull("61. result",    result[1]);
        a.checkEqual("62. turnNumber",  result[1]->turnNumber, 43);
        a.checkEqual("63. slotPlayers", result[1]->slotPlayers.size(), 0U);
        a.checkEqual("64. slotStates",  result[1]->slotStates.size(), 0U);
        a.checkEqual("65. slotScores",  result[1]->slotScores.size(), 0U);
        a.checkEqual("66. time",        result[1]->time, 1919);
        a.checkEqual("67. timestamp",   result[1]->timestamp, "77-66-5555:44:33:22");
    }
}
