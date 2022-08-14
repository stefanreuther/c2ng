/**
  *  \file server/interface/hosthistoryclient.cpp
  *  \brief Class server::interface::HostHistoryClient
  */

#include <memory>
#include "server/interface/hosthistoryclient.hpp"
#include "afl/data/segment.hpp"
#include "afl/except/invaliddataexception.hpp"

using afl::data::Access;
using afl::data::Segment;

server::interface::HostHistoryClient::HostHistoryClient(afl::net::CommandHandler& commandHandler)
    : m_commandHandler(commandHandler)
{ }

void
server::interface::HostHistoryClient::getEvents(const EventFilter& filter, afl::container::PtrVector<Event>& result)
{
    // Build command
    Segment cmd;
    cmd.pushBackString("HISTEVENTS");
    if (const int32_t* p = filter.gameId.get()) {
        cmd.pushBackString("GAME");
        cmd.pushBackInteger(*p);
    }
    if (const String_t* p = filter.userId.get()) {
        cmd.pushBackString("USER");
        cmd.pushBackString(*p);
    }
    if (const int32_t* p = filter.limit.get()) {
        cmd.pushBackString("LIMIT");
        cmd.pushBackInteger(*p);
    }

    // Call
    std::auto_ptr<Value_t> p(m_commandHandler.call(cmd));
    Access a(p);

    // Build result
    for (size_t i = 0, n = a.getArraySize(); i < n; ++i) {
        std::auto_ptr<Event> e(new Event());
        unpackEvent(*e, a[i]);
        result.pushBackNew(e.release());
    }
}

void
server::interface::HostHistoryClient::getTurns(int32_t gameId, const TurnFilter& filter, afl::container::PtrVector<Turn>& result)
{
    // Build command
    Segment cmd;
    cmd.pushBackString("HISTTURN");
    cmd.pushBackInteger(gameId);
    if (const int32_t* p = filter.endTurn.get()) {
        cmd.pushBackString("UNTIL");
        cmd.pushBackInteger(*p);
    }
    if (const int32_t* p = filter.limit.get()) {
        cmd.pushBackString("LIMIT");
        cmd.pushBackInteger(*p);
    }
    if (const int32_t* p = filter.startTime.get()) {
        cmd.pushBackString("SINCETIME");
        cmd.pushBackInteger(*p);
    }
    if (const String_t* p = filter.scoreName.get()) {
        cmd.pushBackString("SCORE");
        cmd.pushBackString(*p);
    }
    if (filter.reportPlayers) {
        cmd.pushBackString("PLAYER");
    }
    if (filter.reportStatus) {
        cmd.pushBackString("STATUS");
    }

    // Call
    std::auto_ptr<Value_t> p(m_commandHandler.call(cmd));
    Access a(p);

    // Build result
    for (size_t i = 0, n = a.getArraySize(); i < n; ++i) {
        std::auto_ptr<Turn> t(new Turn());
        unpackTurn(*t, a[i]);
        result.pushBackNew(t.release());
    }
}

void
server::interface::HostHistoryClient::unpackEvent(Event& out, afl::data::Access a)
{
    out.time       = a("time").toInteger();
    out.eventType  = a("event").toString();
    out.gameId     = toOptionalInteger(a("game").getValue());
    out.gameName   = toOptionalString(a("gameName").getValue());
    out.userId     = toOptionalString(a("user").getValue());
    out.slotNumber = toOptionalInteger(a("slot").getValue());
    if (const Value_t* p = a("state").getValue()) {
        HostGame::State st;
        if (!HostGame::parseState(toString(p), st)) {
            throw afl::except::InvalidDataException("<HostHistory.unpackEvent>");
        }
        out.gameState = st;
    }
}

void
server::interface::HostHistoryClient::unpackTurn(Turn& out, afl::data::Access a)
{
    out.turnNumber = a("turn").toInteger();
    out.time = a("time").toInteger();
    out.timestamp = a("timestamp").toString();

    a("players").toStringList(out.slotPlayers);
    a("turns").toIntegerList(out.slotStates);
    a("scores").toIntegerList(out.slotScores);
}
