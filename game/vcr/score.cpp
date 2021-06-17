/**
  *  \file game/vcr/score.cpp
  *  \brief Class game::vcr::Score
  */

#include "game/vcr/score.hpp"

// Constructor.
game::vcr::Score::Score()
    : m_buildMillipoints(0, 0),
      m_experience(0, 0),
      m_tonsDestroyed(0, 0)
{ }

// Add build point range.
void
game::vcr::Score::addBuildMillipoints(Range_t r)
{
    m_buildMillipoints += r;
}

// Add experience points.
void
game::vcr::Score::addExperience(Range_t points)
{
    m_experience += points;
}

// Add destroyed tons.
void
game::vcr::Score::addTonsDestroyed(Range_t tons)
{
    m_tonsDestroyed += tons;
}

// Get build points.
game::vcr::Score::Range_t
game::vcr::Score::getBuildMillipoints() const
{
    return m_buildMillipoints;
}

// Get experience points.
game::vcr::Score::Range_t
game::vcr::Score::getExperience() const
{
    return m_experience;
}

// Get destroyed tons.
game::vcr::Score::Range_t
game::vcr::Score::getTonsDestroyed() const
{
    return m_tonsDestroyed;
}
