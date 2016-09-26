/**
  *  \file game/spec/hull.cpp
  */

#include "game/spec/hull.hpp"

game::spec::Hull::Hull(int id)
    : Component(ComponentNameProvider::Hull, id),
      m_externalPictureNumber(0),
      m_internalPictureNumber(0),
      m_maxFuel(0),
      m_maxCrew(0),
      m_numEngines(0),
      m_maxCargo(0),
      m_numBays(0),
      m_maxLaunchers(0),
      m_maxBeams(0)
{ }

game::spec::Hull::~Hull()
{ }

int
game::spec::Hull::getExternalPictureNumber() const
{
    return m_externalPictureNumber;
}

void
game::spec::Hull::setExternalPictureNumber(int nr)
{
    m_externalPictureNumber = nr;
}

int
game::spec::Hull::getInternalPictureNumber() const
{
    return m_internalPictureNumber;
}

void
game::spec::Hull::setInternalPictureNumber(int nr)
{
    m_internalPictureNumber = nr;
}

int
game::spec::Hull::getMaxFuel() const
{
    return m_maxFuel;
}

void
game::spec::Hull::setMaxFuel(int maxFuel)
{
    m_maxFuel = maxFuel;
}

int
game::spec::Hull::getMaxCrew() const
{
    return m_maxCrew;
}

void
game::spec::Hull::setMaxCrew(int maxCrew)
{
    m_maxCrew = maxCrew;
}

int
game::spec::Hull::getNumEngines() const
{
    return m_numEngines;
}

void
game::spec::Hull::setNumEngines(int numEngines)
{
    m_numEngines = numEngines;
}

int
game::spec::Hull::getMaxCargo() const
{
    return m_maxCargo;
}

void
game::spec::Hull::setMaxCargo(int maxCargo)
{
    m_maxCargo = maxCargo;
}

int
game::spec::Hull::getNumBays() const
{
    return m_numBays;
}

void
game::spec::Hull::setNumBays(int numBays)
{
    m_numBays = numBays;
}

int
game::spec::Hull::getMaxLaunchers() const
{
    return m_maxLaunchers;
}

void
game::spec::Hull::setMaxLaunchers(int maxTorpedoLaunchers)
{
    m_maxLaunchers = maxTorpedoLaunchers;
}

int
game::spec::Hull::getMaxBeams() const
{
    return m_maxBeams;
}

void
game::spec::Hull::setMaxBeams(int maxBeams)
{
    m_maxBeams = maxBeams;
}

void
game::spec::Hull::clearHullFunctions()
{
    // ex GHull::clearSpecialFunctions
    m_hullFunctions.clear();
    m_shipFunctions.clear();
}

void
game::spec::Hull::changeHullFunction(ModifiedHullFunctionList::Function_t function, PlayerSet_t add, PlayerSet_t remove, bool assignToHull)
{
    // ex GHull::changeSpecialFunction
    if (assignToHull) {
        m_hullFunctions.change(function, add, remove);
    } else {
        m_shipFunctions.change(function, add, remove);
    }
}

game::spec::HullFunctionAssignmentList&
game::spec::Hull::getHullFunctions(bool assignedToHull)
{
    return assignedToHull ? m_hullFunctions : m_shipFunctions;
}

const game::spec::HullFunctionAssignmentList&
game::spec::Hull::getHullFunctions(bool assignedToHull) const
{
    return assignedToHull ? m_hullFunctions : m_shipFunctions;
}
