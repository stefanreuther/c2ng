/**
  *  \file game/spec/hullfunction.cpp
  *  \brief Class game::spec::HullFunction
  */

#include "game/spec/hullfunction.hpp"
#include "game/spec/basichullfunction.hpp"
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

// Check whether two functions name the same hull function.
bool
game::spec::HullFunction::isSame(const HullFunction& other)
{
    // ex GHullFunction::isSame
    return m_basicFunctionId == other.m_basicFunctionId
        && m_levels == other.m_levels;
}

// Get default assignments for a basic function.
game::PlayerSet_t
game::spec::HullFunction::getDefaultAssignment(int basicFunctionId, const game::config::HostConfiguration& config, const Hull& hull)
{
    // Note: all hull functions that can have a nonempty result here must be listed in HullFunctionAssignmentList::clear().
    // ex GHullFunctionData::getDefaultAssignment
    if (basicFunctionId == BasicHullFunction::Tow) {
        // if AllowOneEngineTowing enabled or ship has more than one engine, everyone can tow with it
        if (config[config.AllowOneEngineTowing]() || hull.getNumEngines() > 1) {
            return PlayerSet_t::allUpTo(MAX_PLAYERS);
        } else {
            return PlayerSet_t();
        }
    } else if (basicFunctionId == BasicHullFunction::Boarding) {
        // Tholians and Privateers, if enabled
        PlayerSet_t result;
        if (config[config.AllowPrivateerTowCapture]()) {
            result += config.getPlayersOfRace(5);
        }
        if (config[config.AllowCrystalTowCapture]()) {
            result += config.getPlayersOfRace(7);
        }
        return result;
    } else if (basicFunctionId == BasicHullFunction::AntiCloakImmunity) {
        // as configured
        return config.getPlayersWhereEnabled(config.AntiCloakImmunity);
    } else if (basicFunctionId == BasicHullFunction::PlanetImmunity) {
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
    } else if (basicFunctionId == BasicHullFunction::FullWeaponry) {
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
