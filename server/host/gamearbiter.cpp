/**
  *  \file server/host/gamearbiter.cpp
  *  \brief Class server::host::GameArbiter
  */

#include <stdexcept>
#include "server/host/gamearbiter.hpp"
#include "server/errors.hpp"

// Constructor.
server::host::GameArbiter::GameArbiter()
    : m_lockedGames()
{ }

// Destructor.
server::host::GameArbiter::~GameArbiter()
{ }

// Lock a game.
void
server::host::GameArbiter::lock(int32_t gameId, Intent i)
{
    // ex planetscentral/host/lock.h:lockGame
    // ex GameLock::lockOrDie
    /* As of 201708, all code paths that access games are serialized;
       we have just two types of code path:
       - commands: those access the game in one go
       - scheduler: accesses and locks the game, runs host, accesses and unlocks the game.

       Therefore, the only conflict case is an access to a game currently running host.
       Those immediately fail.
       There is no usecase that will need to be solved through waiting. */
    switch (i) {
     case Simple:
        /* Does not conflict with anything so let it go through. */
        break;

     case Critical:
     case Host:
        /* Critical: conflicts with Host. Another Critical cannot be active due to the serialized nature.
           Host: blocks Critical, but Critical cannot be active in parallel due to the serialized nature.
           Another Host cannot be active in parallel because we have only one scheduler. */
        if (!m_lockedGames.insert(gameId).second) {
            throw std::runtime_error(GAME_IN_USE);
        }
        break;
    }
}

// Unlock a game.
void
server::host::GameArbiter::unlock(int32_t gameId, Intent i)
{
    // ex planetscentral/host/lock.h:unlockGame
    switch (i) {
     case Simple:
        break;

     case Critical:
     case Host:
        m_lockedGames.erase(gameId);
        break;
    }
}

// Constructor.
server::host::GameArbiter::Guard::Guard(GameArbiter& a, int32_t gameId, Intent i)
    : m_arbiter(a),
      m_gameId(gameId),
      m_intent(i)
{
    m_arbiter.lock(m_gameId, m_intent);
}

// Destructor.
server::host::GameArbiter::Guard::~Guard()
{
    m_arbiter.unlock(m_gameId, m_intent);
}
