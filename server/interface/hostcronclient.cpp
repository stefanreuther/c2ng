/**
  *  \file server/interface/hostcronclient.cpp
  */

#include "server/interface/hostcronclient.hpp"
#include "afl/data/segment.hpp"
#include "afl/data/access.hpp"

using afl::data::Segment;
using afl::data::Access;

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
