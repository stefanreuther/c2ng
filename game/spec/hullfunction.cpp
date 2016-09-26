/**
  *  \file game/spec/hullfunction.cpp
  */

#include "game/spec/hullfunction.hpp"
#include "game/spec/hull.hpp"

// Constructor.
game::spec::HullFunction::HullFunction(int basicFunctionId)
    : m_basicFunctionId(basicFunctionId),
      m_players(PlayerSet_t::allUpTo(MAX_PLAYERS)),
      m_levels(ExperienceLevelSet_t::allUpTo(MAX_EXPERIENCE_LEVELS)),
      m_kind(AssignedToShip),
      m_hostId(-1)
{ }

// Constructor.
game::spec::HullFunction::HullFunction(int basicFunctionId, ExperienceLevelSet_t levels)
    : m_basicFunctionId(basicFunctionId),
      m_players(PlayerSet_t::allUpTo(MAX_PLAYERS)),
      m_levels(levels),
      m_kind(AssignedToShip),
      m_hostId(-1)
{ }

void
game::spec::HullFunction::setPlayers(PlayerSet_t players)
{
    m_players = players;
}

void
game::spec::HullFunction::setLevels(ExperienceLevelSet_t levels)
{
    m_levels = levels;
}

void
game::spec::HullFunction::setKind(Kind kind)
{
    m_kind = kind;
}

void
game::spec::HullFunction::setHostId(int hostId)
{
    m_hostId = hostId;
}

void
game::spec::HullFunction::setBasicFunctionId(int basicFunctionId)
{
    m_basicFunctionId = basicFunctionId;
}

game::PlayerSet_t
game::spec::HullFunction::getPlayers() const
{
    return m_players;
}

game::ExperienceLevelSet_t
game::spec::HullFunction::getLevels() const
{
    return m_levels;
}

game::spec::HullFunction::Kind
game::spec::HullFunction::getKind() const
{
    return m_kind;
}

int
game::spec::HullFunction::getHostId() const
{
    return m_hostId;
}

int
game::spec::HullFunction::getBasicFunctionId() const
{
    return m_basicFunctionId;
}

bool
game::spec::HullFunction::isSame(const HullFunction& other)
{
    // ex GHullFunction::isSame
    return m_basicFunctionId == other.m_basicFunctionId
        && m_levels == other.m_levels;
}

// /** Get default assignment for a function. Some hull functions can have a
//     variable default assignment, depending upon configuration or other
//     properties of a hull. In the host, the "Init=Default" statement will
//     consult the current configuration, and set the functions accordingly.
//     We want to be able to support configuration that changes on the fly
//     without reloading hull functions (i.e. when the player configures
//     AllowOneEngineTowing=Yes, all ships magically receive the 'Tow' ability).
//     This function figures out the current variable default for a hull/device.

//     \param device Device identifier
//     \param hull   Hull object

//     \return players who will have the function by default

//     Note that the classic hull functions assigned to fixed hull numbers
//     (i.e. "44-46 = Gravitonic") are not handled here.

//     Note: all hull functions that can have a nonempty result here must be
//     listed in GHull::clearSpecialFunctions(). */
game::PlayerSet_t
game::spec::HullFunction::getDefaultAssignment(int basicFunctionId, const game::config::HostConfiguration& config, const Hull& hull)
{
    // ex GHullFunctionData::getDefaultAssignment
    if (basicFunctionId == Tow) {
        // if AllowOneEngineTowing enabled or ship has more than one engine, everyone can tow with it
        if (config[config.AllowOneEngineTowing]() || hull.getNumEngines() > 1) {
            return PlayerSet_t::allUpTo(MAX_PLAYERS);
        } else {
            return PlayerSet_t();
        }
    } else if (basicFunctionId == Boarding) {
        // Tholians and Privateers, if enabled
        PlayerSet_t result;
        if (config[config.AllowPrivateerTowCapture]()) {
            result += config.getPlayersOfRace(5);
        }
        if (config[config.AllowCrystalTowCapture]()) {
            result += config.getPlayersOfRace(7);
        }
        return result;
    } else if (basicFunctionId == AntiCloakImmunity) {
        // as configured
        return config.getPlayersWhereEnabled(config.AntiCloakImmunity);
    } else if (basicFunctionId == PlanetImmunity) {
        // Rebels and Klingons, if enabled
        // FIXME:        /* This also applies to the SSD, but that's handled differently */
        PlayerSet_t result;
        if (!config[config.PlanetsAttackKlingons]()) {
            result += config.getPlayersOfRace(4);
        }
        if (!config[config.PlanetsAttackRebels]()) {
            result += config.getPlayersOfRace(10);
        }
        return result;
    } else if (basicFunctionId == FullWeaponry) {
        // Feds, if enabled
        if (config[config.AllowFedCombatBonus]()) {
            return config.getPlayersOfRace(1);
        } else {
            return PlayerSet_t();
        }
    } else {
        // nothing special
        return PlayerSet_t();
    }
}
