/**
  *  \file server/interface/hostcronserver.cpp
  */

#include <stdexcept>
#include "server/interface/hostcronserver.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "server/errors.hpp"

server::interface::HostCronServer::HostCronServer(HostCron& impl)
    : ComposableCommandHandler(),
      m_implementation(impl)
{ }

server::interface::HostCronServer::~HostCronServer()
{ }

bool
server::interface::HostCronServer::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
{
    if (upcasedCommand == "CRONGET") {
        /* @q CRONGET game:GID (Host Command)
           Get next scheduler action for a game.
           @retkey action:Str (next action name, one of "unknown", "none", "host", "schedulechange", "master")
           @retkey game:GID (game Id)
           @retkey time:Time (action time)

           Permissions: read-access to game.

           @uses game:$GID:schedule:list */
        args.checkArgumentCount(1);
        int32_t gameId = toInteger(args.getNext());
        result.reset(packEvent(m_implementation.getGameEvent(gameId)));
        return true;
    } else if (upcasedCommand == "CRONLIST") {
        /* @q CRONLIST [LIMIT n:Int] (Host Command)
           Get next scheduler actions.
           Returns an array of actions, optionally limited to a certain size.
           Each action is described by a hash.
           @retkey action:Str (next action name, one of "unknown", "none", "host", "schedulechange", "master")
           @retkey game:GID (game Id)
           @retkey time:Time (action time)

           Permissions: returns only games for which user has read access.

           @see CRONGET
           @uses game:$GID:schedule:list */
        afl::base::Optional<int32_t> limit;
        while (args.getNumArgs() > 0) {
            String_t keyword = afl::string::strUCase(toString(args.getNext()));
            if (keyword == "LIMIT") {
                args.checkArgumentCountAtLeast(1);
                limit = toInteger(args.getNext());
            } else {
                throw std::runtime_error(INVALID_OPTION);
            }
        }

        std::vector<HostCron::Event> events;
        m_implementation.listGameEvents(limit, events);

        afl::data::Vector::Ref_t resultVector = afl::data::Vector::create();
        for (size_t i = 0, n = events.size(); i < n; ++i) {
            resultVector->pushBackNew(packEvent(events[i]));
        }
        result.reset(new afl::data::VectorValue(resultVector));
        return true;
    } else if (upcasedCommand == "CRONKICK") {
        /* @q CRONKICK game:GID (Host Command)
           Restart scheduler for a game.
           This is used to get the scheduler going after admin intervention fixed a problem that declared the game broken.

           Permissions: admin

           @retval Int 1=game has been restarted, 0=game was not broken
           @uses game:broken */
        args.checkArgumentCount(1);
        result.reset(makeIntegerValue(m_implementation.kickstartGame(toInteger(args.getNext()))));
        return true;
    } else {
        return false;
    }
}

server::Value_t*
server::interface::HostCronServer::packEvent(const HostCron::Event& event)
{
    // ex planetscentral/host/cmdcron.cc:describeJob
    afl::data::Hash::Ref_t result = afl::data::Hash::create();
    switch (event.action) {
     case HostCron::UnknownAction:                                                                     break;
     case HostCron::NoAction:             result->setNew("action", makeStringValue("none"));           break;
     case HostCron::HostAction:           result->setNew("action", makeStringValue("host"));           break;
     case HostCron::ScheduleChangeAction: result->setNew("action", makeStringValue("schedulechange")); break;
     case HostCron::MasterAction:         result->setNew("action", makeStringValue("master"));         break;
    }
    result->setNew("game", makeIntegerValue(event.gameId));
    result->setNew("time", makeIntegerValue(event.time));
    return new afl::data::HashValue(result);
}
