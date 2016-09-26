/**
  *  \file game/vcr/statistic.cpp
  */

#include "game/vcr/statistic.hpp"
#include "game/vcr/object.hpp"


// Default constructor.
game::vcr::Statistic::Statistic()
    : m_minFightersAboard(0),
      m_torpedoHits(0)
{ }

// Initialize from VCR object.
game::vcr::Statistic::Statistic(const Object& obj)
    : m_minFightersAboard(obj.getNumFighters()),
      m_torpedoHits(0)
{
    // ex VcrPlayer::initStat, sort-of
}

// Handle number of fighters aboard.
void
game::vcr::Statistic::handleFightersAboard(int n)
{
    // ex VcrPlayer::setStatFighters
    if (n < m_minFightersAboard) {
        m_minFightersAboard = n;
    }
}

// Handle torpedo hit.
void
game::vcr::Statistic::handleTorpedoHit()
{
    // ex VcrPlayer::setStatTorpHit
    ++m_torpedoHits;
}
