/**
  *  \file game/vcr/test/database.cpp
  *  \brief Class game::vcr::test::Database
  */

#include "game/vcr/test/database.hpp"

game::vcr::test::Database::Database()
    : m_battles()
{ }

game::vcr::test::Database::~Database()
{ }

game::vcr::test::Battle&
game::vcr::test::Database::addBattle()
{
    return *m_battles.pushBackNew(new Battle());
}

size_t
game::vcr::test::Database::getNumBattles() const
{
    return m_battles.size();
}

game::vcr::test::Battle*
game::vcr::test::Database::getBattle(size_t nr)
{
    if (nr < m_battles.size()) {
        return m_battles[nr];
    } else {
        return 0;
    }
}
