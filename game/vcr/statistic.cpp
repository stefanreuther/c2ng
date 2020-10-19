/**
  *  \file game/vcr/statistic.cpp
  *  \brief Class game::vcr::Statistic
  */

#include "game/vcr/statistic.hpp"
#include "game/vcr/object.hpp"


// Default constructor.
game::vcr::Statistic::Statistic()
    : m_minFightersAboard(0),
      m_numTorpedoHits(0),
      m_numFights(0)
{ }

// Initialize from VCR participant.
void
game::vcr::Statistic::init(const Object& obj, int numFights)
{
    // ex VcrPlayer::initStat, sort-of
    m_minFightersAboard = obj.getNumFighters();
    m_numTorpedoHits = 0;
    m_numFights = numFights;
}

// Merge with another statistic.
void
game::vcr::Statistic::merge(const Statistic& other)
{
    m_minFightersAboard = std::min(m_minFightersAboard, other.m_minFightersAboard);
    m_numTorpedoHits += other.m_numTorpedoHits;
    m_numFights += other.m_numFights;
}
