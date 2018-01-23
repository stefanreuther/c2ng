/**
  *  \file server/host/hostschedule.cpp
  *  \brief Class server::host::HostSchedule
  *
  *  PCC2 Comment:
  *
  *  A game can have multiple schedules. Each schedule can have an
  *  expiration condition (time or turn) after which it is dropped.
  *  This is used to model rules
  *  - "thrice a week until turn 25, then twice a week"
  *  - "pause until <date>"
  */

#include "server/host/hostschedule.hpp"
#include "server/host/schedule.hpp"
#include "server/host/game.hpp"
#include "server/host/root.hpp"
#include "server/host/session.hpp"
#include "server/host/gamecreator.hpp"
#include "afl/string/format.hpp"
#include "server/errors.hpp"
#include "afl/net/redis/stringlistkey.hpp"

using server::interface::HostSchedule;

namespace {
    void convertSchedule(server::host::Schedule& out,
                         const server::interface::HostSchedule::Schedule& in)
    {
        // ex planetscentral/host/cmdsched.cc:parseSchedule
        // Use 'in.daytime.isValid()' to replace the hadDaytime parameter.
        if (const server::interface::HostSchedule::Type* p = in.type.get()) {
            out.setType(*p);
        }
        if (const int32_t* p = in.weekdays.get()) {
            out.setWeekDays(afl::bits::SmallSet<int8_t>::fromInteger(*p));
        }
        if (const int32_t* p = in.interval.get()) {
            out.setInterval(*p);
        }
        if (const int32_t* p = in.daytime.get()) {
            out.setDaytime(*p);
        }
        if (const bool* p = in.hostEarly.get()) {
            out.setHostEarly(*p);
        }
        if (const int32_t* p = in.hostDelay.get()) {
            out.setHostDelay(*p);
        }
        if (const int32_t* p = in.hostLimit.get()) {
            out.setHostLimit(*p);
        }
        if (const server::host::HostSchedule::Condition* p = in.condition.get()) {
            switch (*p) {
             case HostSchedule::None:
                out.setCondition(*p, 0);
                break;
             case HostSchedule::Turn:
                out.setCondition(*p, in.conditionTurn.orElse(0));
                break;
             case HostSchedule::Time:
                out.setCondition(*p, in.conditionTime.orElse(0));
                break;
            }
        }
    }
}

server::host::HostSchedule::HostSchedule(Session& session, Root& root)
    : m_session(session),
      m_root(root)
{ }

void
server::host::HostSchedule::add(int32_t gameId, const Schedule& sched)
{
    // ex doScheduleAdd
    doAddReplace(gameId, sched, true);
}

void
server::host::HostSchedule::replace(int32_t gameId, const Schedule& sched)
{
    // ex doScheduleSet
    doAddReplace(gameId, sched, false);
}

void
server::host::HostSchedule::modify(int32_t gameId, const Schedule& sched)
{
    // ex doScheduleModify

    // Obtain critical access; schedule modifications cannot parallel anything
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Critical);

    // Check existence and permission
    Game game(m_root, gameId);
    m_session.checkPermission(game, Game::ConfigPermission);

    // Load existing schedule
    afl::net::redis::Subtree sroot(game.getSchedule());
    if (sroot.stringListKey("list").size() == 0) {
        throw std::runtime_error(NO_SCHEDULE);
    }
    String_t scheduleName = sroot.stringListKey("list")[0];

    server::host::Schedule parsedSchedule;
    parsedSchedule.loadFrom(sroot.hashKey(scheduleName));
    convertSchedule(parsedSchedule, sched);

    // Save it back
    game.removeConfig("hostRunNow");
    parsedSchedule.saveTo(sroot.hashKey(scheduleName));
    game.lastScheduleChangeTime().set(m_root.getTime());
    game.scheduleChanged().set(1);
    m_root.handleGameChange(gameId);
}

void
server::host::HostSchedule::getAll(int32_t gameId, std::vector<Schedule>& result)
{
    // ex doScheduleList

    // Obtain simple access; read-only
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Simple);

    // Check existence and permission
    Game game(m_root, gameId);
    m_session.checkPermission(game, Game::ReadPermission);

    // Create result
    afl::net::redis::Subtree sroot(game.getSchedule());
    afl::data::StringList_t schedules;
    sroot.stringListKey("list").getAll(schedules);
    for (size_t i = 0; i < schedules.size(); ++i) {
        server::host::Schedule sch;
        sch.loadFrom(sroot.hashKey(schedules[i]));
        result.push_back(sch.describe(m_root.config()));
    }
}

void
server::host::HostSchedule::drop(int32_t gameId)
{
    // ex doScheduleDrop
    
    // Obtain critical access; schedule modifications cannot parallel anything
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Critical);

    // Check existence and permission
    Game game(m_root, gameId);
    m_session.checkPermission(game, Game::ConfigPermission);

    // Database work
    afl::net::redis::Subtree sroot(game.getSchedule());
    String_t removedSchedule = sroot.stringListKey("list").popFront();
    if (!removedSchedule.empty()) {
        sroot.hashKey(removedSchedule).remove();
        game.removeConfig("hostRunNow");
    }
    game.scheduleChanged().set(1);
    game.lastScheduleChangeTime().set(m_root.getTime());
    m_root.handleGameChange(gameId);
}

void
server::host::HostSchedule::preview(int32_t gameId,
                                    afl::base::Optional<Time_t> timeLimit,
                                    afl::base::Optional<int32_t> turnLimit,
                                    afl::data::IntegerList_t& result)
{
    // ex doScheduleShow

    // Time limit, if given, is relative to current time
    if (timeLimit.isValid()) {
        *timeLimit.get() += m_root.getTime();
    }

    // Turn limit must be given; 0 means none. For simplicity, just return.
    int32_t actualTurnLimit = turnLimit.orElse(0);
    if (actualTurnLimit <= 0) {
        return;
    }

    // Obtain simple access; read-only
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Simple);

    // Check existence and permission
    Game game(m_root, gameId);
    m_session.checkPermission(game, Game::ReadPermission);

    // Figure out last host date
    int32_t lastHost = game.lastHostTime().get();
    int32_t turn     = game.turnNumber().get();
    if (lastHost == 0) {
        // Host never run, so pretend we're hosting now.
        lastHost = m_root.getTime();
        ++turn;
        result.push_back(m_root.config().getUserTimeFromTime(lastHost));
    }

    // Process all schedules
    afl::net::redis::Subtree sroot(game.getSchedule());
    afl::net::redis::StringListKey schedules(sroot.stringListKey("list"));
    int32_t numSchedules = schedules.size();
    int32_t currentSchedule = 0;
    while (currentSchedule < numSchedules
           && (actualTurnLimit > int32_t(result.size()))
           && (!timeLimit.isValid() || (*timeLimit.get() > lastHost))
           && lastHost != 0)
    {
        // Process one schedule
        // FIXME: this does not seem to handle properly transitions from
        // one DAILY/WEEKLY schedule to another with a time condition.
        server::host::Schedule sched;
        sched.loadFrom(sroot.hashKey(schedules[currentSchedule]));
        while ((actualTurnLimit > int32_t(result.size()))
               && (!timeLimit.isValid() || (*timeLimit.get() > lastHost))
               && lastHost != 0
               && !sched.isExpired(turn, lastHost))
        {
            lastHost = sched.getNextHost(lastHost);
            ++turn;
            result.push_back(m_root.config().getUserTimeFromTime(lastHost));
        }
        ++currentSchedule;
    }
}

/** Common implementation of add() and replace().
    \param gameId Game Id
    \param sched  Provided schedule
    \param add    true if this is add(), false if it is replace() */
void
server::host::HostSchedule::doAddReplace(int32_t gameId, const Schedule& sched, bool add)
{
    // ex planetscentral/host/cmdsched.cc:doSchedule
    // Convert incoming schedule
    server::host::Schedule parsedSchedule;
    convertSchedule(parsedSchedule, sched);

    // Obtain critical access; schedule modifications cannot parallel anything
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Critical);

    // Check existence and permission
    Game game(m_root, gameId);
    m_session.checkPermission(game, Game::ConfigPermission);

    // If the daytime was not set, try to derive it from the existing schedule.
    afl::net::redis::Subtree sroot(game.getSchedule());
    bool anySchedule = sroot.stringListKey("list").size() > 0;
    if (!sched.daytime.isValid()) {
        if (anySchedule) {
            String_t currentSchedule = sroot.stringListKey("list")[0];
            parsedSchedule.setDaytime(sroot.hashKey(currentSchedule).intField("daytime").get());
        } else {
            parsedSchedule.setDaytime(GameCreator(m_root).pickDayTime());
        }
    }

    // Process command.
    if (add || !anySchedule) {
        String_t newSchedule = afl::string::Format("%d", ++sroot.intKey("lastId"));
        parsedSchedule.saveTo(sroot.hashKey(newSchedule));
        sroot.stringListKey("list").pushFront(newSchedule);
    } else {
        parsedSchedule.saveTo(sroot.hashKey(sroot.stringListKey("list")[0]));
    }
    game.removeConfig("hostRunNow");
    game.lastScheduleChangeTime().set(m_root.getTime());
    game.scheduleChanged().set(1);
    m_root.handleGameChange(gameId);
}
