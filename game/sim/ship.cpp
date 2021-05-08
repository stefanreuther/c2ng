/**
  *  \file game/sim/ship.cpp
  *  \brief Class game::sim::Ship
  */

#include "game/sim/ship.hpp"
#include "afl/string/format.hpp"
#include "game/sim/configuration.hpp"
#include "game/spec/hullfunction.hpp"

using game::spec::Hull;

namespace {
    const int MAX_WEAPONS = 20;
}

const int game::sim::Ship::agg_Kill;
const int game::sim::Ship::agg_Passive;
const int game::sim::Ship::agg_NoFuel;


// Default constructor.
game::sim::Ship::Ship()
    : Object(),
      m_crew(10),
      m_hullType(0),
      m_mass(100),
      m_beamType(0),
      m_numBeams(0),
      m_torpedoType(0),
      m_numLaunchers(0),
      m_numBays(0),
      m_ammo(0),
      m_engineType(1),
      m_aggressiveness(agg_Passive),
      m_interceptId(0)
{
    // ex GSimShip::GSimShip
}

// Destructor.
game::sim::Ship::~Ship()
{ }

// Get crew.
int
game::sim::Ship::getCrew() const
{
    // ex GSimShip::getCrew
    return m_crew;
}

// Set crew.
void
game::sim::Ship::setCrew(int crew)
{
    // ex GSimShip::setCrew
    m_crew = crew;
    markDirty();
}

// Get hull type.
int
game::sim::Ship::getHullType() const
{
    // ex GSimShip::getHull
    return m_hullType;
}

// Set hull type.
void
game::sim::Ship::setHullType(int hullType, const game::spec::ShipList& shipList)
{
    // ex GSimShip::setHull, ccsim.pas:MassOf (sort-of), ccsim.pas:SetHull
    if (hullType != m_hullType) {
        m_hullType = hullType;
        if (const Hull* hull = shipList.hulls().get(hullType)) {
            // beams
            m_numBeams = hull->getMaxBeams();
            if (m_numBeams != 0) {
                if (m_beamType == 0) {
                    m_beamType = shipList.beams().size();
                }
            }

            // torps/fighters
            if (hull->getNumBays() != 0) {
                m_numLaunchers = 0;
                m_torpedoType  = 0;
                m_numBays      = hull->getNumBays();
                m_ammo         = hull->getMaxCargo();
            } else if (hull->getMaxLaunchers() != 0) {
                m_numLaunchers = hull->getMaxLaunchers();
                if (m_torpedoType == 0) {
                    m_torpedoType = shipList.launchers().size();
                }
                m_numBays = 0;
                m_ammo = hull->getMaxCargo();
            } else {
                m_numLaunchers = 0;
                m_torpedoType = 0;
                m_numBays = 0;
                m_ammo = 0;
            }

            // rest
            m_crew = hull->getMaxCrew();
            m_mass = hull->getMass();
        }
        markDirty();
    }
}

// Set hull type only.
void
game::sim::Ship::setHullTypeOnly(int hullType)
{
    m_hullType = hullType;
    markDirty();
}

// Get mass.
int
game::sim::Ship::getMass() const
{
    // ex GSimShip::getMass
    return m_mass;
}

// Set mass.
void
game::sim::Ship::setMass(int mass)
{
    // ex GSimShip::setMass
    m_mass = mass;
    markDirty();
}

// Get beam type.
int
game::sim::Ship::getBeamType() const
{
    // ex GSimShip::getBeamType
    return m_beamType;
}

// Set beam type.
void
game::sim::Ship::setBeamType(int beamType)
{
    // ex GSimShip::setBeamType
    m_beamType = beamType;
    markDirty();
}

// Get number of beams.
int
game::sim::Ship::getNumBeams() const
{
    // ex GSimShip::getBeamCount
    return m_numBeams;
}

// Set number of beams.
void
game::sim::Ship::setNumBeams(int numBeams)
{
    // ex GSimShip::setBeamCount
    m_numBeams = numBeams;
    markDirty();
}

// Get torpedo type.
int
game::sim::Ship::getTorpedoType() const
{
    // ex GSimShip::getTorpType
    return m_torpedoType;
}

// Set torpedo type.
void
game::sim::Ship::setTorpedoType(int torpedoType)
{
    // ex GSimShip::setTorpType
    m_torpedoType = torpedoType;
    markDirty();
}

// Get number of torpedo launchers.
int
game::sim::Ship::getNumLaunchers() const
{
    // ex GSimShip::getTorpLauncherCount
    return m_numLaunchers;
}

// Set number of torpedo launchers.
void
game::sim::Ship::setNumLaunchers(int numLaunchers)
{
    // ex GSimShip::setTorpLauncherCount
    m_numLaunchers = numLaunchers;
    markDirty();
}

// Get number of fighter bays.
int
game::sim::Ship::getNumBays() const
{
    // ex GSimShip::getBayCount
    return m_numBays;
}

// Set number of fighter bays.
void
game::sim::Ship::setNumBays(int numBays)
{
    // ex GSimShip::setBayCount
    m_numBays = numBays;
    markDirty();
}

// Get number of torpedoes/fighters.
int
game::sim::Ship::getAmmo() const
{
    // ex GSimShip::getAmmo
    return m_ammo;
}

// Set number of torpedoes/fighters.
void
game::sim::Ship::setAmmo(int ammo)
{
    // ex GSimShip::setAmmo
    m_ammo = ammo;
    markDirty();
}

// Get engine type.
int
game::sim::Ship::getEngineType() const
{
    // ex GSimShip::getEngineType
    return m_engineType;
}

// Set engine type.
void
game::sim::Ship::setEngineType(int engineType)
{
    // ex GSimShip::setEngineType
    m_engineType = engineType;
    markDirty();
}

// Get aggressiveness.
int
game::sim::Ship::getAggressiveness() const
{
    // ex GSimShip::getAggressiveness
    return m_aggressiveness;
}

// Set aggressiveness.
void
game::sim::Ship::setAggressiveness(int aggressiveness)
{
    // ex GSimShip::setAggressiveness
    m_aggressiveness = aggressiveness;
    markDirty();
}

// Get Id for intercept-attack.
int
game::sim::Ship::getInterceptId() const
{
    // ex GSimShip::getInterceptId
    return m_interceptId;
}

// Set Id for intercept-attack.
void
game::sim::Ship::setInterceptId(int id)
{
    // ex GSimShip::setInterceptId
    m_interceptId = id;
    markDirty();
}

// Check for default name.
bool
game::sim::Ship::hasDefaultName(afl::string::Translator& tx) const
{
    return getName() == String_t(afl::string::Format(tx.translateString("Ship %d").c_str(), getId()));
}

// Set default name.
void
game::sim::Ship::setDefaultName(afl::string::Translator& tx)
{
    setName(afl::string::Format(tx.translateString("Ship %d").c_str(), getId()));
}

// Check for custom ship.
bool
game::sim::Ship::isCustomShip() const
{
    // ex GSimShip::isCustomShip
    return getHullType() == 0;
}

// Get range of number of beams.
util::Range<int>
game::sim::Ship::getNumBeamsRange(const game::spec::ShipList& shipList) const
{
    if (isCustomShip()) {
        return util::Range<int>(0, MAX_WEAPONS);
    } else {
        if (const Hull* p = shipList.hulls().get(getHullType())) {
            return util::Range<int>(0, p->getMaxBeams());
        } else {
            return util::Range<int>::fromValue(0);
        }
    }
}

// Get range of number of torpedo launchers.
util::Range<int>
game::sim::Ship::getNumLaunchersRange(const game::spec::ShipList& shipList) const
{
    if (isCustomShip()) {
        return util::Range<int>(0, MAX_WEAPONS);
    } else {
        if (const Hull* p = shipList.hulls().get(getHullType())) {
            return util::Range<int>(0, p->getMaxLaunchers());
        } else {
            return util::Range<int>::fromValue(0);
        }
    }
}

// Get range of fighter bays.
util::Range<int>
game::sim::Ship::getNumBaysRange(const game::spec::ShipList& shipList) const
{
    if (isCustomShip()) {
        return util::Range<int>(0, MAX_WEAPONS);
    } else {
        if (const Hull* p = shipList.hulls().get(getHullType())) {
            return util::Range<int>::fromValue(p->getNumBays());
        } else {
            return util::Range<int>::fromValue(0);
        }
    }
}

// Check whether this ship matches a ship list.
bool
game::sim::Ship::isMatchingShipList(const game::spec::ShipList& shipList) const
{
    // ex GSimShip::isMatchingShipList
    /* verify equipment */
    if (shipList.engines().get(getEngineType()) == 0) {
        return false;
    }
    if (getNumBeams() > 0 && shipList.beams().get(getBeamType()) == 0) {
        return false;
    }
    if (getNumLaunchers() > 0 && shipList.launchers().get(getTorpedoType()) == 0) {
        return false;
    }

    /* custom ships have full freedom for hull attributes */
    if (isCustomShip()) {
        return true;
    }

    /* valid hull? */
    /* FIXME: we cannot handle these during simulation so we should
       avoid even loading them. */
    const Hull* hull = shipList.hulls().get(getHullType());
    if (hull == 0) {
        return false;
    }

    /* beams */
    if (getNumBeams() > hull->getMaxBeams()) {
        return false;
    }

    /* torps / fighters */
    if (getNumLaunchers() > hull->getMaxLaunchers()) {
        return false;
    }
    if (getNumBays() != hull->getNumBays()) {
        return false;
    }
    if (getAmmo() > hull->getMaxCargo()) {
        return false;
    }

    return true;
}

// Check for implied hull function.
bool
game::sim::Ship::hasImpliedFunction(int basicFunctionId, const game::spec::ShipList& shipList, const game::config::HostConfiguration& config) const
{
    /* FIXME: PCC 1.1.17 decides upon host version here, and checks ImperialAssault
       on older hosts instead of PlanetImmunity. Ideally, the hull function module
       would isolate us from these differences. */
    // If getPlayersThatCan() receives an unknown hull type (e.g. 0), it will return an empty set.
    // This conveniently makes this function return false, as intended.
    return shipList.getPlayersThatCan(basicFunctionId, getHullType(), config, ExperienceLevelSet_t(getExperienceLevel()))
        .contains(getOwner());
}

// Check whether this ship has a specific hull function from the configuration.
bool
game::sim::Ship::hasImpliedAbility(Ability which, const Configuration& opts, const game::spec::ShipList& shipList, const game::config::HostConfiguration& config) const
{
    // ex GSimShip::hasImpliedFunction, ccsim.pas:SimHullDoes
    using game::spec::HullFunction;
    switch (which) {
     case PlanetImmunityAbility:
        // FIXME: do we need the "|| getPlayerRaceNumber()" part? Should normally be done by the hullfunc engine.
        return hasImpliedFunction(HullFunction::PlanetImmunity, shipList, config)
            || (config.getPlayerRaceNumber(getOwner()) == 4 && !config[config.PlanetsAttackKlingons]())
            || (config.getPlayerRaceNumber(getOwner()) == 10 && !config[config.PlanetsAttackRebels]());

     case FullWeaponryAbility:
        return hasImpliedFunction(HullFunction::FullWeaponry, shipList, config);

     case CommanderAbility:
        return hasImpliedFunction(HullFunction::Commander, shipList, config);

     case TripleBeamKillAbility:
        return config.getPlayerRaceNumber(getOwner()) == 5;

     case DoubleBeamChargeAbility:
        return opts.getMode() == Configuration::VcrNuHost
            && config.getPlayerRaceNumber(getOwner()) == 4;

     case DoubleTorpedoChargeAbility:
     case ElusiveAbility:
     case SquadronAbility:
     case ShieldGeneratorAbility:
     case CloakedBaysAbility:
        return false;
    }
    return false;
}


// Check for primary enemy.
bool
game::sim::Ship::isPrimaryEnemy(int agg)
{
    return agg > 0
        && agg != agg_NoFuel;
}
