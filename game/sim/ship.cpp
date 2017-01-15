/**
  *  \file game/sim/ship.cpp
  */

#include "game/sim/ship.hpp"
#include "game/spec/hullfunction.hpp"
#include "afl/string/format.hpp"

namespace {
    bool checkHullFunction(const game::sim::Ship& sh, const game::spec::ShipList& shipList, const game::config::HostConfiguration& config, int basicFunctionId)
    {
        /* FIXME: PCC 1.1.17 decides upon host version here, and checks ImperialAssault
           on older hosts instead of PlanetImmunity. Ideally, the hull function module
           would isolate us from these differences. */
        // If getPlayersThatCan() receives an unknown hull type (e.g. 0), it will return an empty set.
        // This conveniently makes this function return false, as intended.
        return shipList.getPlayersThatCan(basicFunctionId,
                                          sh.getHullType(),
                                          config,
                                          game::ExperienceLevelSet_t(sh.getExperienceLevel()))
            .contains(sh.getOwner());
    }
}

const int game::sim::Ship::agg_Kill;
const int game::sim::Ship::agg_Passive;
const int game::sim::Ship::agg_NoFuel;


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

game::sim::Ship::~Ship()
{ }

// /** Get crew. */
int
game::sim::Ship::getCrew() const
{
    // ex GSimShip::getCrew
    return m_crew;
}

// /** Change crew. */
void
game::sim::Ship::setCrew(int crew)
{
    // ex GSimShip::setCrew
    m_crew = crew;
    markDirty();
}

// /** Get hull type. Can be 0 for custom ships. */
int
game::sim::Ship::getHullType() const
{
    // ex GSimShip::getHull
    return m_hullType;
}

// /** Set hull type. Can be 0 for custom ships. If this changes the hull
//     type, it will also update the arms and crew to reflect that new
//     type. */
void
game::sim::Ship::setHullType(int hullType, const game::spec::ShipList& shipList)
{
    // ex GSimShip::setHull
    if (hullType != m_hullType) {
        m_hullType = hullType;
        if (const game::spec::Hull* hull = shipList.hulls().get(hullType)) {
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
                if (m_numLaunchers != 0) {
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

void
game::sim::Ship::setHullTypeOnly(int hullType)
{
    m_hullType = hullType;
    markDirty();
}

// /** Get mass. Always kept up-to-date. */
int
game::sim::Ship::getMass() const
{
    // ex GSimShip::getMass
    return m_mass;
}

// /** Change mass. Only used for custom ships. */
void
game::sim::Ship::setMass(int mass)
{
    // ex GSimShip::setMass
    m_mass = mass;
    markDirty();
}

// /** Get beam type. */
int
game::sim::Ship::getBeamType() const
{
    // ex GSimShip::getBeamType
    return m_beamType;
}

// /** Change beam type. */
void
game::sim::Ship::setBeamType(int beamType)
{
    // ex GSimShip::setBeamType
    m_beamType = beamType;
    markDirty();
}

// /** Get beam count. */
int
game::sim::Ship::getNumBeams() const
{
    // ex GSimShip::getBeamCount
    return m_numBeams;
}

// /** Change beam count. */
void
game::sim::Ship::setNumBeams(int numBeams)
{
    // ex GSimShip::setBeamCount
    m_numBeams = numBeams;
    markDirty();
}

// /** Get torpedo type. */
int
game::sim::Ship::getTorpedoType() const
{
    // ex GSimShip::getTorpType
    return m_torpedoType;
}

// /** Change torpedo type. */
void
game::sim::Ship::setTorpedoType(int torpedoType)
{
    // ex GSimShip::setTorpType
    m_torpedoType = torpedoType;
    markDirty();
}

// /** Get torpedo launcher count. */
int
game::sim::Ship::getNumLaunchers() const
{
    // ex GSimShip::getTorpLauncherCount
    return m_numLaunchers;
}

// /** Change torpedo launcher count. */
void
game::sim::Ship::setNumLaunchers(int numLaunchers)
{
    // ex GSimShip::setTorpLauncherCount
    m_numLaunchers = numLaunchers;
    markDirty();
}

// /** Get number of fighter bays. */
int
game::sim::Ship::getNumBays() const
{
    // ex GSimShip::getBayCount
    return m_numBays;
}

// /** Change number of fighter bays. */
void
game::sim::Ship::setNumBays(int numBays)
{
    // ex GSimShip::setBayCount
    m_numBays = numBays;
    markDirty();
}

// /** Get ammunition (torps, fighters). */
int
game::sim::Ship::getAmmo() const
{
    // ex GSimShip::getAmmo
    return m_ammo;
}

// /** Change ammunition (torps, fighters). */
void
game::sim::Ship::setAmmo(int ammo)
{
    // ex GSimShip::setAmmo
    m_ammo = ammo;
    markDirty();
}

// /** Get engine type. */
int
game::sim::Ship::getEngineType() const
{
    // ex GSimShip::getEngineType
    return m_engineType;
}

// /** Change engine type. */
void
game::sim::Ship::setEngineType(int engineType)
{
    // ex GSimShip::setEngineType
    m_engineType = engineType;
    markDirty();
}

/** Get aggressiveness. */
int
game::sim::Ship::getAggressiveness() const
{
    // ex GSimShip::getAggressiveness
    return m_aggressiveness;
}

// /** Change aggressiveness (agg_XXX). */
void
game::sim::Ship::setAggressiveness(int aggressiveness)
{
    // ex GSimShip::setAggressiveness
    m_aggressiveness = aggressiveness;
    markDirty();
}

// /** Get Id for intercept-attack. */
int
game::sim::Ship::getInterceptId() const
{
    // ex GSimShip::getInterceptId
    return m_interceptId;
}

// /** Set Id for intercept-attack. */
void
game::sim::Ship::setInterceptId(int id)
{
    // ex GSimShip::setInterceptId
    m_interceptId = id;
    markDirty();
}

bool
game::sim::Ship::hasDefaultName(afl::string::Translator& tx) const
{
    return getName() == String_t(afl::string::Format(tx.translateString("Ship %d").c_str(), getId()));
}

void
game::sim::Ship::setDefaultName(afl::string::Translator& tx)
{
    setName(afl::string::Format(tx.translateString("Ship %d").c_str(), getId()));
}

// /** Check whether this is a custom ship. Custom ships have some more
//     freedom in configuring. */
bool
game::sim::Ship::isCustomShip() const
{
    // ex GSimShip::isCustomShip
    return getHullType() == 0;
}

// /** Check whether this ship matches the current ship list. */
bool
game::sim::Ship::isMatchingShipList(const game::spec::ShipList& shipList) const
{
    // ex GSimShip::isMatchingShipList
    /* custom ships have full freedom */
    if (isCustomShip()) {
        return true;
    }

    /* valid hull? */
    /* FIXME: we cannot handle these during simulation so we should
       avoid even loading them. */
    const game::spec::Hull* hull = shipList.hulls().get(getHullType());
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

// /** Check whether this ship has a specific hull function from the configuration. */
bool
game::sim::Ship::hasImpliedAbility(Ability which, const game::spec::ShipList& shipList, const game::config::HostConfiguration& config) const
{
    // ex GSimShip::hasImpliedFunction
    using game::spec::HullFunction;
    switch (which) {
     case PlanetImmunityAbility:
        // FIXME: do we need the "|| getPlayerRaceNumber()" part? Should normally be done by the hullfunc engine.
        return checkHullFunction(*this, shipList, config, HullFunction::PlanetImmunity)
            || (config.getPlayerRaceNumber(getOwner()) == 4 && !config[config.PlanetsAttackKlingons]())
            || (config.getPlayerRaceNumber(getOwner()) == 10 && !config[config.PlanetsAttackRebels]());

     case FullWeaponryAbility:
        return checkHullFunction(*this, shipList, config, HullFunction::FullWeaponry);

     case CommanderAbility:
        return checkHullFunction(*this, shipList, config, HullFunction::Commander);

     case TripleBeamKillAbility:
        return config.getPlayerRaceNumber(getOwner()) == 5;

     case DoubleBeamChargeAbility:
     case DoubleTorpedoChargeAbility:
     case ElusiveAbility:
     case SquadronAbility:
        return false;
    }
    return false;
}

