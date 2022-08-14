/**
  *  \file server/interface/hostcronclient.cpp
  *  \brief Class server::interface::HostCronClient
  */

#include "server/interface/hostcronclient.hpp"
#include "afl/data/access.hpp"
#include "afl/data/segment.hpp"

using afl::data::Access;
using afl::data::Segment;

server::interface::HostCronClient::HostCronClient(afl::net::CommandHandler& commandHandler)
    : m_commandHandler(commandHandler)
{ }

server::interface::HostCron::Event
server::interface::HostCronClient::getGameEvent(int32_t gameId)
{
    std::auto_ptr<afl::data::Value> p(m_commandHandler.call(Segment().pushBackString("CRONGET").pushBackInteger(gameId)));
    return unpackEvent(p.get());
}

void
server::interface::HostCronClient::listGameEvents(afl::base::Optional<int32_t> limit, std::vector<Event>& result)
{
    Segment cmd;
    cmd.pushBackString("CRONLIST");
    if (const int* n = limit.get()) {
        cmd.pushBackString("LIMIT");
        cmd.pushBackInteger(*n);
    }
    std::auto_ptr<afl::data::Value> p(m_commandHandler.call(cmd));
    Access a(p);

    for (size_t i = 0, n = a.getArraySize(); i < n; ++i) {
        result.push_back(unpackEvent(a[i].getValue()));
    }
}

bool
server::interface::HostCronClient::kickstartGame(int32_t gameId)
{
    return m_commandHandler.callInt(Segment().pushBackString("CRONKICK").pushBackInteger(gameId));
}

void
server::interface::HostCronClient::suspendScheduler(int32_t relativeTime)
{
    m_commandHandler.callVoid(Segment().pushBackString("CRONSUSPEND").pushBackInteger(relativeTime));
}

void
server::interface::HostCronClient::getBrokenGames(BrokenMap_t& result)
{
    std::auto_ptr<afl::data::Value> p(m_commandHandler.call(Segment().pushBackString("CRONLSBROKEN")));
    afl::data::Access a(p);
    for (size_t i = 0, n = a.getArraySize(); i < n; i += 2) {
        int32_t gameId = a[i].toInteger();
        result[gameId] = a[i+1].toString();
    }
}

server::interface::HostCron::Event
server::interface::HostCronClient::unpackEvent(const afl::data::Value* p)
{
    Access a(p);
    Event result;

    // Action
    const String_t actionName = a("action").toString();
    if (actionName == "none") {
        result.action = NoAction;
    } else if (actionName == "host") {
        result.action = HostAction;
    } else if (actionName == "schedulechange") {
        result.action = ScheduleChangeAction;
    } else if (actionName == "master") {
        result.action = MasterAction;
    } else {
        result.action = UnknownAction;
    }

    // Game, Time
    result.gameId = a("game").toInteger();
    result.time = a("time").toInteger();

    return result;
}
