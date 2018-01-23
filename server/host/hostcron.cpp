/**
  *  \file server/host/hostcron.cpp
  *  \brief Class server::host::HostCron
  */

#include "server/host/hostcron.hpp"
#include "server/host/cron.hpp"
#include "server/host/root.hpp"
#include "server/host/session.hpp"
#include "server/host/game.hpp"

server::host::HostCron::HostCron(Session& session, Root& root)
    : m_session(session),
      m_root(root)
{ }

server::interface::HostCron::Event
server::host::HostCron::getGameEvent(int32_t gameId)
{
    // ex planetscentral/host/cmdcron.cc:doCronGet

    // Check existence and permission
    Game game(m_root, gameId);
    m_session.checkPermission(game, Game::ReadPermission);

    // Get event
    if (Cron* p = m_root.getCron()) {
        Event e = p->getGameEvent(gameId);
        return Event(e.gameId, e.action, m_root.config().getUserTimeFromTime(e.time));
    } else {
        return Event(gameId, NoAction, 0);
    }
}

void
server::host::HostCron::listGameEvents(afl::base::Optional<int32_t> limit, std::vector<Event>& result)
{
    // ex planetscentral/host/cmdcron.cc:doCronList

    // Fetch schedules
    std::vector<Event> sched;
    if (Cron* p = m_root.getCron()) {
        p->listGameEvents(sched);
    }

    // Generate output. Filter according to logged-in user if required
    for (size_t i = 0, n = sched.size(); i < n; ++i) {
        const Event& e = sched[i];

        // Limit?
        if (int32_t* p = limit.get()) {
            if (*p <= 0) {
                break;
            }
        }

        // Permission check
        if (Game(m_root, e.gameId, Game::NoExistanceCheck).hasPermission(m_session.getUser(), Game::ReadPermission)) {
            result.push_back(Event(e.gameId, e.action, m_root.config().getUserTimeFromTime(e.time)));
            if (int32_t* p = limit.get()) {
                --*p;
            }
        }
    }
}

bool
server::host::HostCron::kickstartGame(int32_t gameId)
{
    // ex planetscentral/host/cmdcron.cc:doCronKick

    // Permission checks. Game must exist (construction of the Game object ensures that),
    // and user must be admin. Mere mortals cannot kick games.
    Game game(m_root, gameId);
    m_session.checkAdmin();

    // Do it
    bool did = m_root.gameRoot().intSetKey("broken").remove(gameId);
    if (did) {
        m_root.handleGameChange(gameId);
    }

    return did;
}
