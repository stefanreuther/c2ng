/**
  *  \file game/turn.cpp
  *  \brief Class game::Turn
  */

#include "game/turn.hpp"
#include "game/extra.hpp"

game::Turn::Turn()
    : m_universe(),
      m_extras(),
      m_inbox(),
      m_outbox(),
      m_battles(),
      m_turnNumber(0),
      m_databaseTurnNumber(0),
      m_timestamp(),
      m_commandPlayers(),
      m_localDataPlayers(),
      m_alliances()
{ }

game::Turn::~Turn()
{ }

void
game::Turn::setDatabaseTurnNumber(int turnNumber)
{
    m_databaseTurnNumber = turnNumber;
}

int
game::Turn::getDatabaseTurnNumber() const
{
    // ex GGameTurn::getDatabaseTurn
    return m_databaseTurnNumber;
}

void
game::Turn::setTimestamp(const Timestamp& ts)
{
    m_timestamp = ts;
}

game::Timestamp
game::Turn::getTimestamp() const
{
    return m_timestamp;
}

void
game::Turn::setBattles(afl::base::Ptr<game::vcr::Database> battles)
{
    m_battles = battles;
}

afl::base::Ptr<game::vcr::Database>
game::Turn::getBattles() const
{
    return m_battles;
}

void
game::Turn::notifyListeners()
{
    m_universe.notifyListeners();
}
