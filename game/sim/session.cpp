/**
  *  \file game/sim/session.cpp
  *  \brief Class game::sim::Session
  */

#include "game/sim/session.hpp"

game::sim::Session::Session()
    : RefCounted(),
      m_setup(),
      m_config(),
      m_gameInterface(),
      m_usePlayerRelations(true)
{ }

game::sim::Session::~Session()
{ }

void
game::sim::Session::setNewGameInterface(GameInterface* gi)
{
    m_gameInterface.reset(gi);
}

game::sim::GameInterface*
game::sim::Session::getGameInterface() const
{
    return m_gameInterface.get();
}

void
game::sim::Session::getPlayerRelations(PlayerBitMatrix& alliances, PlayerBitMatrix& enemies) const
{
    alliances.clear();
    enemies.clear();
    if (m_gameInterface.get() != 0) {
        m_gameInterface->getPlayerRelations(alliances, enemies);
    }
}

void
game::sim::Session::usePlayerRelations()
{
    if (m_usePlayerRelations) {
        getPlayerRelations(m_config.allianceSettings(), m_config.enemySettings());
    }
}
