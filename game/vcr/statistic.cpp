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
