/**
  *  \file game/vcr/score.cpp
  */

#include "game/vcr/score.hpp"

// Constructor.
game::vcr::Score::Score()
    : m_buildMillipointsMin(0),
      m_buildMillipointsMax(0),
      m_experience(0),
      m_tonsDestroyed(0)
{ }

// Add build point range.
void
game::vcr::Score::addBuildMillipoints(int32_t min, int32_t max)
{
    m_buildMillipointsMin += min;
    m_buildMillipointsMax += max;
}

// Add experience points.
void
game::vcr::Score::addExperience(int32_t points)
{
    m_experience += points;
}

// Add destroyed tons.
void
game::vcr::Score::addTonsDestroyed(int32_t tons)
{
    m_tonsDestroyed += tons;
}

// Get minimum build points.
int
game::vcr::Score::getBuildMillipointsMin() const
{
    return m_buildMillipointsMin;
}

// Get maximum build points.
int
game::vcr::Score::getBuildMillipointsMax() const
{
    return m_buildMillipointsMax;
}

// Get experience points.
int
game::vcr::Score::getExperience() const
{
    return m_experience;
}

// Get destroyed tons.
int
game::vcr::Score::getTonsDestroyed() const
{
    return m_tonsDestroyed;
}
