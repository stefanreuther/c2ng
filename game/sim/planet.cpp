/**
  *  \file game/sim/planet.cpp
  *  \brief Class game::sim::Planet
  */

#include "game/sim/planet.hpp"
#include "game/sim/configuration.hpp"

// Default constructor.
game::sim::Planet::Planet()
    : m_defense(10),
      m_baseDefense(10),
      m_beamTech(0),
      m_torpedoTech(1),
      m_baseFighters(0)
{
    // ex GSimPlanet::GSimPlanet
    for (int i = 0; i < NUM_TORPEDO_TYPES; ++i) {
        m_baseTorpedoes[i] = 0;
    }
}

// Destructor.
game::sim::Planet::~Planet()
{ }

// Assign from other planet.
game::sim::Planet&
game::sim::Planet::operator=(const Planet& other)
{
    Object::operator=(other);
    setDefense(other.m_defense);
    setBaseDefense(other.m_baseDefense);
    setBaseBeamTech(other.m_beamTech);
    setBaseTorpedoTech(other.m_torpedoTech);
    setNumBaseFighters(other.m_baseFighters);
    for (int i = 1; i <= NUM_TORPEDO_TYPES; ++i) {
        setNumBaseTorpedoes(i, other.getNumBaseTorpedoes(i));
    }
    return *this;
}

// Set number of planetary defense posts.
void
game::sim::Planet::setDefense(int defense)
{
    // ex GSimPlanet::setDefense
    if (m_defense != defense) {
        m_defense = defense;
        markDirty();
    }
}

// Set number of starbase defense posts.
void
game::sim::Planet::setBaseDefense(int baseDefense)
{
    // ex GSimPlanet::setBaseDefense
    if (m_baseDefense != baseDefense) {
        m_baseDefense = baseDefense;
        markDirty();
    }
}

// Set starbase beam tech level.
void
game::sim::Planet::setBaseBeamTech(int beamTech)
{
    // ex GSimPlanet::setBaseBeamTech
    if (m_beamTech != beamTech) {
        m_beamTech = beamTech;
        markDirty();
    }
}

// Set starbase torpedo tech level.
void
game::sim::Planet::setBaseTorpedoTech(int torpTech)
{
    // ex GSimPlanet::setBaseTorpTech
    if (m_torpedoTech != torpTech) {
        m_torpedoTech = torpTech;
        markDirty();
    }
}

// Set number of starbase fighters.
void
game::sim::Planet::setNumBaseFighters(int baseFighters)
{
    // ex GSimPlanet::setBaseFighters
    if (m_baseFighters != baseFighters) {
        m_baseFighters = baseFighters;
        markDirty();
    }
}

// Get number of starbase torpedoes of a given type.
int
game::sim::Planet::getNumBaseTorpedoes(int type) const
{
    // ex GSimPlanet::getBaseTorps
    if (type > 0 && type <= NUM_TORPEDO_TYPES) {
        return m_baseTorpedoes[type-1];
    } else {
        return 0;
    }
}

// Set number of starbase torpedoes of a given type.
void
game::sim::Planet::setNumBaseTorpedoes(int type, int amount)
{
    // ex GSimPlanet::setBaseTorps
    if (type > 0 && type <= NUM_TORPEDO_TYPES && m_baseTorpedoes[type-1] != amount) {
        m_baseTorpedoes[type-1] = amount;
        markDirty();
    }
}

// Get total number of starbase torpedoes as one type.
int32_t
game::sim::Planet::getNumBaseTorpedoesAsType(int type, const game::spec::ShipList& shipList) const
{
    // ex GSimPlanet::getBaseTorpsAsType
    using game::spec::Cost;

    // Total cost
    int32_t totalCost = 0;
    for (int i = 1; i <= NUM_TORPEDO_TYPES; ++i) {
        if (const game::spec::TorpedoLauncher* torp = shipList.launchers().get(i)) {
            totalCost += getNumBaseTorpedoes(i) * torp->cost().get(Cost::Money);
        }
    }

    // Individual launcher cost
    if (const game::spec::TorpedoLauncher* torp = shipList.launchers().get(type)) {
        const int32_t cost = torp->cost().get(Cost::Money);
        if (cost != 0) {
            totalCost /= cost;
        }
    }

    // FIXME: some range checking here?
    return totalCost;
}

bool
game::sim::Planet::hasImpliedAbility(Ability which, const Configuration& opts, const game::spec::ShipList& /*shipList*/, const game::config::HostConfiguration& config) const
{
    // ex GSimPlanet::hasImpliedFunction
    switch (which) {
     case PlanetImmunityAbility:
     case FullWeaponryAbility:
     case CommanderAbility:
        return false;

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
