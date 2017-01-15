/**
  *  \file game/turn.cpp
  */

#include "game/turn.hpp"
#include "game/extra.hpp"

game::Turn::Turn()
    : m_universe(),
      m_extras(),
      m_inbox(),
      m_battles(),
      m_turnNumber(0),
      m_databaseTurnNumber(0),
      m_timestamp()
{ }

game::Turn::~Turn()
{ }

void
game::Turn::setTurnNumber(int turnNumber)
{
    m_turnNumber = turnNumber;
}

int
game::Turn::getTurnNumber() const
{
    return m_turnNumber;
}

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

game::map::Universe&
game::Turn::universe()
{
    // ex GGameTurn::getCurrentUniverse, GGameTurn::getPreviousUniverse
    return m_universe;
}

const game::map::Universe&
game::Turn::universe() const
{
    // ex GGameTurn::getCurrentUniverse, GGameTurn::getPreviousUniverse
    return m_universe;
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

game::msg::Inbox&
game::Turn::inbox()
{
    return m_inbox;
}

const game::msg::Inbox&
game::Turn::inbox() const
{
    return m_inbox;
}

game::ExtraContainer<game::Turn>&
game::Turn::extras()
{
    return m_extras;
}

const game::ExtraContainer<game::Turn>&
game::Turn::extras() const
{
    return m_extras;
}

void
game::Turn::notifyListeners()
{
    m_universe.notifyListeners();
}
