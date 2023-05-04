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

game::sim::Setup&
game::sim::Session::setup()
{
    return m_setup;
}

const game::sim::Setup&
game::sim::Session::setup() const
{
    return m_setup;
}

game::sim::Configuration&
game::sim::Session::configuration()
{
    return m_config;
}

const game::sim::Configuration&
game::sim::Session::configuration() const
{
    return m_config;
}

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
game::sim::Session::setUsePlayerRelations(bool flag)
{
    m_usePlayerRelations = flag;
}

bool
game::sim::Session::isUsePlayerRelations() const
{
    return m_usePlayerRelations;
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
