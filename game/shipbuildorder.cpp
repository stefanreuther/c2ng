/**
  *  \file game/shipbuildorder.cpp
  *  \brief Class game::ShipBuildOrder
  */

#include "game/shipbuildorder.hpp"

game::ShipBuildOrder::ShipBuildOrder()
    : m_hullIndex(0),
      m_engineType(0),
      m_beamType(0),
      m_numBeams(0),
      m_launcherType(0),
      m_numLaunchers(0)
{ }

int
game::ShipBuildOrder::getHullIndex() const
{
    return m_hullIndex;
}

void
game::ShipBuildOrder::setHullIndex(int n)
{
    m_hullIndex = n;
}

int
game::ShipBuildOrder::getEngineType() const
{
    return m_engineType;
}

void
game::ShipBuildOrder::setEngineType(int n)
{
    m_engineType = n;
}

int
game::ShipBuildOrder::getBeamType() const
{
    return m_beamType;
}

void
game::ShipBuildOrder::setBeamType(int n)
{
    m_beamType = n;
}

int
game::ShipBuildOrder::getNumBeams() const
{
    return m_numBeams;
}

void
game::ShipBuildOrder::setNumBeams(int n)
{
    m_numBeams = n;
}

int
game::ShipBuildOrder::getLauncherType() const
{
    return m_launcherType;
}

void
game::ShipBuildOrder::setLauncherType(int n)
{
    m_launcherType = n;
}

int
game::ShipBuildOrder::getNumLaunchers() const
{
    return m_numLaunchers;
}

void
game::ShipBuildOrder::setNumLaunchers(int n)
{
    m_numLaunchers = n;
}

void
game::ShipBuildOrder::canonicalize()
{
    // ex game/actions/baseship.cc:canonicalizeBuildOrder
    if (m_numLaunchers == 0) {
        m_launcherType = 0;
    }
    if (m_numBeams == 0) {
        m_beamType = 0;
    }
}

bool
game::ShipBuildOrder::operator==(const ShipBuildOrder& other) const
{
    return m_hullIndex == other.m_hullIndex
        && m_engineType == other.m_engineType
        && m_beamType == other.m_beamType
        && m_numBeams == other.m_numBeams
        && m_launcherType == other.m_launcherType
        && m_numLaunchers == other.m_numLaunchers;
}

bool
game::ShipBuildOrder::operator!=(const ShipBuildOrder& other) const
{
    return !operator==(other);
}
