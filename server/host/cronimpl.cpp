/**
  *  \file server/host/cronimpl.cpp
  *  \brief Class server::host::CronImpl
  */

#include <algorithm>
#include "server/host/cronimpl.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/net/redis/integersetkey.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/mutexguard.hpp"
#include "server/host/exec.hpp"
#include "server/host/game.hpp"
#include "server/host/gamearbiter.hpp"
#include "server/host/root.hpp"
#include "server/host/schedule.hpp"
#include "server/interface/hostcron.hpp"

using server::interface::HostSchedule;
using server::interface::HostCron;
using server::host::Game;
using server::host::Schedule;

namespace {
    const char LOG_NAME[] = "host.cron";

    /** Grace period after schedule changes.
        Host is delayed by this many minutes after an explicit schedule change
        to avoid running immediately in case the change was an error. */
    const int32_t SCHEDULE_CHANGE_GRACE_PERIOD = 10;

    /** Delay from last join to game actually starting. */
    const int32_t MASTER_DELAY = 15;

    /** Predicate to find a game in a list of ScheduleItems. */
    class IsGame {
     public:
        IsGame(int32_t gameId)
            : m_gameId(gameId) { }
        bool operator()(const HostCron::Event& i)
            { return i.gameId == m_gameId; }
     private:
        int32_t m_gameId;
    };

    /** Sort predicate to sort a list of ScheduleItems by time. */
    class ByTime {
     public:
        bool operator()(const HostCron::Event& a, const HostCron::Event& b)
            {
                if (a.time != b.time) {
                    return a.time < b.time;
                }
                if (a.gameId != b.gameId) {
                    return a.gameId < b.gameId;
                }
                return a.action < b.action;
            }
    };


    /** Check that all turns are in for a game. */
    bool checkAllTurnsIn(Game& game)
    {
        bool haveAnyTurns = false;
        for (int i = 1; i <= Game::NUM_PLAYERS; ++i) {
            Game::Slot slot(game.getSlot(i));
            int32_t turnStatus = slot.turnStatus().get();
            if (turnStatus == Game::TurnGreen || turnStatus == Game::TurnYellow) {
                // Turn is green or yellow, that's ok for hosting.
                haveAnyTurns = true;
            } else if (turnStatus == Game::TurnDead
                       || (turnStatus == Game::TurnMissing
                           && (slot.slotStatus().get() == 0
                               || slot.players().size() == 0)))
            {
                // There is no turn because the slot is empty or not played.
                // That's ok for hosting, but we still need some turns for other players.
            } else {
                // Turn is missing (or temporary)
                return false;
            }
        }
        return haveAnyTurns;
    }

    /** Compute time for a running game.
        Database lock must be held.
        The schedule item will be produced on the given list, \c sch.

        \param currentTime [in] Current time
        \param root   [in] Service root
        \param gameId [in] Game to work on
        \param sch    [out] Schedule will be produced here */
    void computeGameHostTimes(const int32_t currentTime, server::host::Root& root, const int32_t gameId, std::list<HostCron::Event>& sch)
    {
        // DB objects
        Game gg(root, gameId, Game::NoExistanceCheck);
        afl::net::redis::Subtree schedules = gg.getSchedule();
        const int32_t turn = gg.turnNumber().get();

        // Handle grace period
        const int32_t lastScheduleChange = gg.lastScheduleChangeTime().get() + SCHEDULE_CHANGE_GRACE_PERIOD;
        const int32_t initialTime = std::max(currentTime, lastScheduleChange);

        // If this game was never hosted, we must master it before! Games will be in state "running"
        // when explicitly placed in that state, even though they have not been mastered yet.
        // So, master it immediately. We cannot check masterHasRun, because that is already set
        // for premastered games where we still must run through our master script to complete the
        // game directory.
        if (turn == 0) {
            sch.push_back(HostCron::Event(gameId, HostCron::MasterAction, initialTime));
            return;
        }

        // Start by expiring obsolete schedules
        afl::net::redis::StringListKey scheduleList = schedules.stringListKey("list");

        Schedule currentSchedule;
        bool currentScheduleValid = false;
        bool haveDroppedSchedule = false;
        while (!currentScheduleValid && scheduleList.size() > 0) {
            String_t currentScheduleId = scheduleList[0];
            currentSchedule.loadFrom(schedules.hashKey(currentScheduleId));
            if (currentSchedule.isExpired(turn, currentTime)) {
                // This schedule is expired, drop it
                scheduleList.popFront();
                schedules.hashKey(currentScheduleId).remove();
                currentScheduleValid = false;
                haveDroppedSchedule = true;
            } else {
                // This schedule is valid
                currentScheduleValid = true;
            }
        }

        // Create a schedule expiration event
        int32_t scheduleChangeTime = 0;
        if (currentScheduleValid && currentSchedule.getCondition() == HostSchedule::Time) {
            scheduleChangeTime = currentSchedule.getConditionArg();
        }

        // Now compute game events.
        int32_t nextHostTime = 0;
        int32_t lastHostTime = gg.lastHostTime().get();

        // If we have dropped a schedule, adjust lastHostTime.
        // Assuming we're changing from a slow schedule to a "Monday, Thursday" schedule on a Sunday,
        // the scheduler would otherwise see that the Thursday host is overdue and immediately run it.
        // Players still expect next host to run on Monday.
        if (haveDroppedSchedule && lastHostTime > 0 && currentScheduleValid) {
            server::Time_t virtualTime = currentSchedule.getPreviousVirtualHost(initialTime);
            if (virtualTime != 0 && virtualTime > lastHostTime) {
                lastHostTime = virtualTime;
                gg.lastHostTime().set(lastHostTime);
            }
        }

        if (lastHostTime == 0) {
            // Host never ran, so schedule it for running immediately.
            // FIXME: why? This is an additional requirement for imported games.
            nextHostTime = initialTime;
        } else if (!currentScheduleValid || currentSchedule.getType() == HostSchedule::Stopped) {
            // Game is not hosted
        } else if (currentSchedule.getType() == HostSchedule::Quick) {
            // Game is hosted when all turns are in
            if (checkAllTurnsIn(gg)) {
                nextHostTime = gg.lastTurnSubmissionTime().get() + currentSchedule.getHostDelay();
            }
        } else if (currentSchedule.getType() == HostSchedule::Manual) {
            // Game is hosted when flag is set
            if (gg.getConfigInt("hostRunNow") != 0) {
                nextHostTime = initialTime;
            } else if (currentSchedule.getHostEarly() && checkAllTurnsIn(gg)) {
                nextHostTime = gg.lastTurnSubmissionTime().get() + currentSchedule.getHostDelay();
            }
        } else {
            // Game is hosted on fixed schedule. Note that a schedule may also happen
            // to return time 0 (e.g. if it's a weekly schedule with no days).
            nextHostTime = currentSchedule.getNextHost(lastHostTime);
            if (nextHostTime != 0) {
                if (currentSchedule.getHostEarly() && checkAllTurnsIn(gg)) {
                    int32_t accHostTime = gg.lastTurnSubmissionTime().get() + currentSchedule.getHostDelay();
                    if (accHostTime < nextHostTime) {
                        nextHostTime = accHostTime;
                    }
                }
            }
        }

        // Fix up grace period
        if (nextHostTime > 0 && nextHostTime < initialTime) {
            nextHostTime = initialTime;
        }

        // Generate exactly one event.
        if (nextHostTime > 0 && (scheduleChangeTime == 0 || nextHostTime <= scheduleChangeTime)) {
            sch.push_back(HostCron::Event(gameId, HostCron::HostAction, nextHostTime));
            gg.setConfigInt("nextHostTime", nextHostTime);
        } else if (scheduleChangeTime > 0 && (nextHostTime == 0 || scheduleChangeTime < nextHostTime)) {
            sch.push_back(HostCron::Event(gameId, HostCron::ScheduleChangeAction, scheduleChangeTime));
            gg.removeConfig("nextHostTime");
        } else {
            // no event
            gg.removeConfig("nextHostTime");
        }
    }

    /** Compute master time for a joining game.
        Database lock must be held.
        The schedule item will be produced on the given list, \c sch.

        \param now    [in] Current time
        \param root   [in] Service root
        \param gameId [in] Game to work on
        \param sch    [out] Schedule will be produced here */
    void computeGameMasterTimes(const int32_t now, server::host::Root& root, const int32_t gameId, std::list<HostCron::Event>& sch)
    {
        // Check whether this game should be started
        Game game(root, gameId, Game::NoExistanceCheck);
        if (!game.hasAnyOpenSlot()) {
            int32_t time = game.getConfigInt("lastPlayerJoined");
            if (time == 0) {
                time = now;
            } else {
                time += MASTER_DELAY;  /* FIXME: make this configurable */
            }
            sch.push_back(HostCron::Event(gameId, HostCron::MasterAction, time));
        }
    }
}


// Constructor.
server::host::CronImpl::CronImpl(Root& root, util::ProcessRunner& runner)
    : Cron(), Uncopyable(), Stoppable(),
      m_root(root),
      m_runner(runner),
      m_thread("host.cron", *this),
      m_mutex(),
      m_wake(0),
      m_stopFlag(false),
      m_changedGames(),
      m_futureEvents(),
      m_dueEvents()
{
    // ex Cron::Cron, Cron::start
    m_root.setCron(this);
    m_thread.start();
}

// Destructor.
server::host::CronImpl::~CronImpl()
{
    m_root.setCron(0);
    stop();
    m_thread.join();
}

// Get next action for a game. Looks only at the schedules.
server::host::Cron::Event_t
server::host::CronImpl::getGameEvent(int32_t gameId)
{
    // ex Cron::getNextAction
    afl::sys::MutexGuard g(m_mutex);

    // If game is currently under reconsideration, say that I don't know.
    if (std::find(m_changedGames.begin(), m_changedGames.end(), gameId) != m_changedGames.end()) {
        return Event_t(gameId, HostCron::UnknownAction, 0);
    }

    // Is it overdue?
    std::list<Event_t>::const_iterator p = std::find_if(m_dueEvents.begin(), m_dueEvents.end(), IsGame(gameId));
    if (p != m_dueEvents.end()) {
        // Return time 0 to mean now
        return Event_t(p->gameId, p->action, 0);
    }

    // Is it in the future?
    p = std::find_if(m_futureEvents.begin(), m_futureEvents.end(), IsGame(gameId));
    if (p != m_futureEvents.end()) {
        return *p;
    }

    // Nothing found
    return Event_t(gameId, HostCron::NoAction, 0);
}

// Get all scheduler events.
// \param output [out] Scheduler events will be appended here
void
server::host::CronImpl::listGameEvents(std::vector<Event_t>& result)
{
    // ex Cron::getAllSchedules
    afl::sys::MutexGuard g(m_mutex);
    for (std::list<Event_t>::const_iterator p = m_dueEvents.begin(); p != m_dueEvents.end(); ++p) {
        result.push_back(Event_t(p->gameId, p->action, 0));
    }
    for (std::list<Event_t>::const_iterator p = m_futureEvents.begin(); p != m_futureEvents.end(); ++p) {
        result.push_back(*p);
    }
}

// Handle start of a game. Called on joining->running transition of a game. Runs master and the initial host.
// Handle change of a game's settings. Checks whether this means a new host date.
void
server::host::CronImpl::handleGameChange(int32_t gameId)
{
    // ex Cron::handleGameStart, Cron::handleGameChange
    m_root.log().write(afl::sys::LogListener::Info, LOG_NAME, afl::string::Format("game %d: triggering update", gameId));
    {
        afl::sys::MutexGuard g(m_mutex);
        m_changedGames.push_back(gameId);
    }
    m_wake.post();
}

// Cron main loop.
void
server::host::CronImpl::run()
{
    try {
        // Main loop
        schedulerMain();
    }
    catch (std::exception& e) {
        // Scheduler crashed. Logging that is probably the best we can do.
        // However, if the error is due to a shutdown, i.e. environment is already gone, this is normal and we do not want to log it.
        m_wake.wait(50);
        afl::sys::MutexGuard g(m_mutex);
        if (!m_stopFlag) {
            // Log
            m_root.log().write(afl::sys::LogListener::Error, "host.except", "Exception in Scheduler", e);

            // Clear data, so scheduler reports "nothing to do" to the outside.
            m_changedGames.clear();
            m_futureEvents.clear();
            m_dueEvents.clear();
        }
    }
}

void
server::host::CronImpl::stop()
{
    {
        afl::sys::MutexGuard g(m_mutex);
        m_stopFlag = true;
    }
    m_wake.post();
}

void
server::host::CronImpl::schedulerMain()
{
    // ex Cron::cron
    // Generate initial schedule
    m_root.log().write(afl::sys::LogListener::Info, LOG_NAME, "Generating initial schedule...");
    generateInitialSchedule();
    m_root.log().write(afl::sys::LogListener::Info, LOG_NAME, afl::string::Format("Generated %d events", m_futureEvents.size()));

    // Main loop
    while (!isStopRequested()) {
        // Process incoming requests
        processRequests();

        // Move due items to the m_dueEvents list
        moveDueItems();

        // Figure out what to do
        enum { Nothing, Run, Wait } what = Nothing;
        Event_t item(0, HostCron::NoAction, 0);
        {
            afl::sys::MutexGuard g(m_mutex);
            if (!m_dueEvents.empty()) {
                item = m_dueEvents.front();
                what = Run;
            } else if (!m_futureEvents.empty()) {
                item = m_futureEvents.front();
                what = Wait;
            }
        }

        // Process it
        if (what == Wait) {
            // Wait for scheduled event
            int64_t ms = (m_root.getSystemTimeFromTime(item.time) - afl::sys::Time::getCurrentTime()).getMilliseconds();
            if (ms > 0) {
                m_wake.wait(1 + afl::sys::Timeout_t(std::min(ms, int64_t(0x10000000))));
            }
        } else if (what == Run) {
            // Execute item
            std::list<Event_t> newSchedule;
            runDueItem(item.gameId, newSchedule);

            {
                // Update schedules
                afl::sys::MutexGuard g1(m_root.mutex());
                afl::sys::MutexGuard g2(m_mutex);
                m_dueEvents.remove_if(IsGame(item.gameId));
                m_futureEvents.merge(newSchedule, ByTime());

                // Unlock the game
                m_root.arbiter().unlock(item.gameId, GameArbiter::Host);
            }
        } else {
            // Nothing to do, wait for request
            m_wake.wait();
        }
    }
}

bool
server::host::CronImpl::isStopRequested()
{
    afl::sys::MutexGuard g(m_mutex);
    return m_stopFlag;
}

// Log an action.
// \param what What we'll do with this action
// \param item The action
void
server::host::CronImpl::logAction(const char* what, const Event_t& item)
{
    // ex planetscentral/host/cron.cc:logAction
    const char* action = 0;
    switch (item.action) {
     case HostCron::HostAction:           action = "host";           break;
     case HostCron::MasterAction:         action = "master";         break;
     case HostCron::ScheduleChangeAction: action = "schedulechange"; break;
     case HostCron::NoAction:             action = "none";           break;
     case HostCron::UnknownAction:        action = "UNKNOWN";        break;
    }

    if (action != 0) {
        const afl::sys::Time t = m_root.getSystemTimeFromTime(item.time);
        m_root.log().write(afl::sys::LogListener::Info, LOG_NAME,
                           afl::string::Format("game %d: %s: %s, t=%d [%s, %s]")
                           << item.gameId
                           << what
                           << action
                           << item.time
                           << t.toString(afl::sys::Time::LocalTime, afl::sys::Time::DateFormat)
                           << t.toString(afl::sys::Time::LocalTime, afl::sys::Time::TimeFormat));
    }
}

// Generate initial schedule. Examines all games, and generates their schedule.
// Since this is called once at startup, it needn't be particularily careful
// about maintaining parallelism and examinability.
void
server::host::CronImpl::generateInitialSchedule()
{
    // ex Cron::generateInitialSchedule
    afl::sys::MutexGuard g1(m_root.mutex());
    afl::sys::MutexGuard g2(m_mutex);
    afl::net::redis::Subtree root(m_root.gameRoot());

    int32_t now = m_root.getTime();

    // Broken games
    afl::data::IntegerList_t broken;
    root.intSetKey("broken").getAll(broken);
    if (!broken.empty()) {
        m_root.log().write(afl::sys::LogListener::Warn, LOG_NAME, afl::string::Format("There are %d broken games.", broken.size()));
    }

    // Running games
    {
        afl::data::IntegerList_t list;
        root.intSetKey("state:running").getAll(list);
        for (afl::data::IntegerList_t::const_iterator pi = list.begin(); pi != list.end(); ++pi) {
            if (std::find(broken.begin(), broken.end(), *pi) == broken.end()) {
                computeGameHostTimes(now, m_root, *pi, m_futureEvents);
            }
        }
    }

    // Joining games
    {
        afl::data::IntegerList_t list;
        root.intSetKey("state:joining").getAll(list);
        for (afl::data::IntegerList_t::const_iterator pi = list.begin(); pi != list.end(); ++pi) {
            if (std::find(broken.begin(), broken.end(), *pi) == broken.end()) {
                computeGameMasterTimes(now, m_root, *pi, m_futureEvents);
            }
        }
    }

    m_futureEvents.sort(ByTime());
}

// Process incoming requests. This takes game Ids from the command queue,
// recomputes their schedules, and updates the global schedule.
void
server::host::CronImpl::processRequests()
{
    // ex Cron::processRequests
    while (1) {
        // Get next item to consider. Ignore items that are on the overdue list.
        // (Those will be rescheduled anyway when they get off the overdue list.)
        int32_t gameId;
        while (1) {
            afl::sys::MutexGuard g(m_mutex);
            if (m_changedGames.empty()) {
                // No more notifications
                return;
            }
            gameId = m_changedGames.front();
            if (find_if(m_dueEvents.begin(), m_dueEvents.end(), IsGame(gameId)) == m_dueEvents.end()) {
                // It's not overdue, so process it
                break;
            }
            m_changedGames.pop_front();
        }

        // Compute new schedule
        std::list<HostCron::Event> result;
        {
            afl::sys::MutexGuard g(m_root.mutex());
            // m_root.configureReconnect() - not needed, DB connection is stateless
            // FIXME: lock the game?
            computeGameTimes(m_root.getTime(), m_root, gameId, result);
        }
        if (!result.empty()) {
            logAction("updated", result.front());
        }

        // Update global schedule: replace this game's old schedule item with a
        // new one and take this game out of the m_changedGames list.
        {
            afl::sys::MutexGuard g(m_mutex);
            m_futureEvents.remove_if(IsGame(gameId));
            m_futureEvents.merge(result, ByTime());
            m_changedGames.remove(gameId);
        }
    }
}

// /** Move due items from the schedule to the overdue list.
//     This marks them for immediate processing. */
void
server::host::CronImpl::moveDueItems()
{
    // ex Cron::moveDueItems
    afl::sys::MutexGuard g1(m_root.mutex());
    afl::sys::MutexGuard g2(m_mutex);
    int32_t now = m_root.getTime();
    while (!m_futureEvents.empty() && m_futureEvents.front().time <= now) {
        logAction("due", m_futureEvents.front());
        // FIXME: should we have to deal with lockGame() failing?
        // This would be an internal error.
        m_root.arbiter().lock(m_futureEvents.front().gameId, GameArbiter::Host);
        m_dueEvents.splice(m_dueEvents.end(), m_futureEvents, m_futureEvents.begin());
    }
}

// /** Run due item.
//     \param gameId      [in]  Game to work on
//     \param newSchedule [out] New schedule for next event will be produced here */
void
server::host::CronImpl::runDueItem(const int32_t gameId, std::list<Event_t>& newSchedule)
{
    // ex Cron::runDueItem
    // Check that schedule is still current (it should be because the game is locked).
    // DbSubtree root(dbConnection, "game:");
    const int32_t now = m_root.getTime();
    {
        afl::sys::MutexGuard g(m_root.mutex());
        computeGameTimes(now, m_root, gameId, newSchedule);

        // Remove "run host now" signalisation.
        // If this is the HostAction, we'll find that action in newSchedule and run it.
        // If this is a different action, or the game isn't actually running on a Manual
        // schedule, we'll have to remove it to avoid it leaking through to a future
        // Manual HostAction.
        m_root.gameRoot().subtree(gameId).hashKey("settings").intField("hostRunNow").remove();
    }

    if (newSchedule.empty() || newSchedule.front().time > now) {
        // Schedule is no longer current, event happens in the future.
        // This happens, for example, when
        // - this is a ScheduleChangeAction
        // - a player resigns before the game starts
        if (!newSchedule.empty()) {
            logAction("updated", newSchedule.front());
        }
        return;
    }

    // Action should be performed
    try {
        Event_t& item = newSchedule.front();
        logAction("executing", newSchedule.front());
        if (item.action == HostCron::HostAction) {
            runHost(m_runner, m_root, gameId);
        } else if (item.action == HostCron::MasterAction) {
            runMaster(m_runner, m_root, gameId);
        }
    }
    catch (std::exception& e) {
        m_root.log().write(afl::sys::LogListener::Warn, LOG_NAME, "Exception", e);
        m_root.log().write(afl::sys::LogListener::Warn, LOG_NAME, afl::string::Format("Game %d is now broken", gameId));
        Game(m_root, gameId, Game::NoExistanceCheck).markBroken(e.what(), m_root);
    }

    // Schedule next event
    newSchedule.clear();
    {
        afl::sys::MutexGuard g(m_root.mutex());
        computeGameTimes(now, m_root, gameId, newSchedule);
    }
}


void
server::host::computeGameTimes(const int32_t now, Root& root, const int32_t gameId, std::list<server::interface::HostCron::Event>& sch)
{
    if (!root.gameRoot().intSetKey("broken").contains(gameId)) {
        String_t gameState = root.gameRoot().subtree(gameId).stringKey("state").get();
        if (gameState == "joining") {
            computeGameMasterTimes(now, root, gameId, sch);
        }
        if (gameState == "running") {
            computeGameHostTimes(now, root, gameId, sch);
        }
    }
}
