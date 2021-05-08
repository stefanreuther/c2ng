/**
  *  \file game/spec/hull.cpp
  *  \brief Class game::spec::Hull
  */

#include <algorithm>
#include "game/spec/hull.hpp"
#include "util/math.hpp"

using game::config::HostConfiguration;

// Constructor.
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

// Destructor.
game::spec::Hull::~Hull()
{ }

// Get external picture number.
int
game::spec::Hull::getExternalPictureNumber() const
{
    return m_externalPictureNumber;
}

// Set external picture number.
void
game::spec::Hull::setExternalPictureNumber(int nr)
{
    m_externalPictureNumber = nr;
}

// Get internal picture number.
int
game::spec::Hull::getInternalPictureNumber() const
{
    return m_internalPictureNumber;
}

// Set internal picture number.
void
game::spec::Hull::setInternalPictureNumber(int nr)
{
    m_internalPictureNumber = nr;
}

// Get fuel capacity.
int
game::spec::Hull::getMaxFuel() const
{
    return m_maxFuel;
}

// Set fuel capacity.
void
game::spec::Hull::setMaxFuel(int maxFuel)
{
    m_maxFuel = maxFuel;
}

// Get maximum crew.
int
game::spec::Hull::getMaxCrew() const
{
    return m_maxCrew;
}

// Set maximum crew.
void
game::spec::Hull::setMaxCrew(int maxCrew)
{
    m_maxCrew = maxCrew;
}

// Get number of engines.
int
game::spec::Hull::getNumEngines() const
{
    return m_numEngines;
}

// Set number of engines.
void
game::spec::Hull::setNumEngines(int numEngines)
{
    m_numEngines = numEngines;
}

// Get cargo capacity.
int
game::spec::Hull::getMaxCargo() const
{
    return m_maxCargo;
}

// Set cargo capacity.
void
game::spec::Hull::setMaxCargo(int maxCargo)
{
    m_maxCargo = maxCargo;
}

// Get number of fighter bays.
int
game::spec::Hull::getNumBays() const
{
    return m_numBays;
}

// Set number of fighter bays.
void
game::spec::Hull::setNumBays(int numBays)
{
    m_numBays = numBays;
}

// Get maximum number of torpedo launchers.
int
game::spec::Hull::getMaxLaunchers() const
{
    return m_maxLaunchers;
}

// Set maximum number of torpedo launchers.
void
game::spec::Hull::setMaxLaunchers(int maxTorpedoLaunchers)
{
    m_maxLaunchers = maxTorpedoLaunchers;
}

// Get maximum number of beams.
int
game::spec::Hull::getMaxBeams() const
{
    return m_maxBeams;
}

// Set maximum number of beams.
void
game::spec::Hull::setMaxBeams(int maxBeams)
{
    m_maxBeams = maxBeams;
}

// Clear hull function assignments.
void
game::spec::Hull::clearHullFunctions()
{
    // ex GHull::clearSpecialFunctions
    m_hullFunctions.clear();
    m_shipFunctions.clear();
}

// Modify (add/remove) hull function assignments.
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

// Get hull function assignment list.
game::spec::HullFunctionAssignmentList&
game::spec::Hull::getHullFunctions(bool assignedToHull)
{
    return assignedToHull ? m_hullFunctions : m_shipFunctions;
}

// Get hull function assignment list.
const game::spec::HullFunctionAssignmentList&
game::spec::Hull::getHullFunctions(bool assignedToHull) const
{
    return assignedToHull ? m_hullFunctions : m_shipFunctions;
}

// Get cloak fuel usage.
int
game::spec::Hull::getCloakFuelUsage(int forPlayer, const game::config::HostConfiguration& config) const
{
    const int cfb = config[HostConfiguration::CloakFuelBurn](forPlayer);
    return std::max(getMass() * cfb / 100, cfb);
}

// Get per-turn fuel usage.
int
game::spec::Hull::getTurnFuelUsage(int forPlayer, bool fight, const game::config::HostConfiguration& config) const
{
    // ex getTurnFuelUsage (sort-of)
    return (config[fight
                   ? HostConfiguration::FuelUsagePerFightFor100KT
                   : HostConfiguration::FuelUsagePerTurnFor100KT](forPlayer) * getMass() + 99) / 100;
}

// Get mine hit damage.
int
game::spec::Hull::getMineHitDamage(int /*forPlayer*/, bool web, const HostVersion& host, const game::config::HostConfiguration& config) const
{
    const int mass = getMass();
    return (mass <= 0
            ? 100
            : host.isPHost()
            ? 100 * config[web
                           ? HostConfiguration::WebHitDamageFor100KT
                           : HostConfiguration::MineHitDamageFor100KT](/*forPlayer*/) / mass
            : (web
               // I assume this needs to be divideAndRoundToEven, but don't yet have a testcase to prove it.
               // For the standard ship list, this produces the same values as divideAndRound (which is used in PCC2/PCC1).
               ? util::divideAndRoundToEven(1000, mass + 1, 0)
               : util::divideAndRoundToEven(10000, mass + 1, 0)));
}

int
game::spec::Hull::getPointsToBuild(int forPlayer, const HostVersion& host, const game::config::HostConfiguration& config) const
{
    if (host.isPHost()) {
        return std::max(getMass() * config[HostConfiguration::PBPCostPer100KT](forPlayer) / 100,
                        config[HostConfiguration::PBPMinimumCost](forPlayer));
    } else {
        /* Build:
           - Vendetta (100 kt) => 2
           - Loki (101 kt) => 3 */
        return (getMass() + 49) / 50;
    }
}

int
game::spec::Hull::getPointsForKilling(int forPlayer, const HostVersion& host, const game::config::HostConfiguration& config) const
{
    if (host.isPHost()) {
        /* Kill, estimation (since there are many ways to destroy it) */
        return getMass() * (config[HostConfiguration::PALAggressorPointsPer10KT](forPlayer) + config[HostConfiguration::PALAggressorKillPointsPer10KT](forPlayer)) / 10
            + config[HostConfiguration::PALCombatAggressor](forPlayer);
    } else {
        /* Kill:
           - Dwarfstar (100 kt) => 2 */
        return (getMass() / 100) + 1;
    }
}

int
game::spec::Hull::getPointsForScrapping(int forPlayer, const HostVersion& host, const game::config::HostConfiguration& config) const
{
    if (host.isPHost()) {
        return getMass() * config[HostConfiguration::PALRecyclingPer10KT](forPlayer) / 10;
    } else {
        return 1;
    }
}
