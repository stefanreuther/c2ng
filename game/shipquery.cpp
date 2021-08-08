/**
  *  \file game/shipquery.cpp
  *  \brief Class game::ShipQuery
  */

#include "game/shipquery.hpp"
#include "game/map/ship.hpp"
#include "game/spec/hull.hpp"
#include "game/limits.hpp"

// Constructor.
game::ShipQuery::ShipQuery()
{
    // ex GShipQuery::GShipQuery
    clear();
}

// Clear. Makes this object blank.
void
game::ShipQuery::clear()
{
    // ex GShipQuery::clear
    m_hullType         = 0;
    m_shipId           = 0;
    m_levelFilterSet   = ExperienceLevelSet_t::fromInteger(-1U);
    m_levelDisplaySet  = ExperienceLevelSet_t::fromInteger(1);
    m_playerFilterSet  = PlayerSet_t::fromInteger(-1U);
    m_playerDisplaySet = PlayerSet_t::fromInteger(0);
    m_engineType       = 0;
    m_combatMass       = 0;
    m_usedESBRate      = 0;
    m_crew             = 0;
    m_owner            = 0;
    m_damage           = 0;
}

// Initialize for existing ship.
void
game::ShipQuery::initForExistingShip(const game::map::Universe& univ,
                                     int shipId,
                                     const game::spec::ShipList& shipList,
                                     const game::config::HostConfiguration& config,
                                     const UnitScoreDefinitionList& scoreDefs)
{
    // ex GShipQuery::initForExistingShip
    clear();
    m_shipId = shipId;

    // complete() will do most of the work.
    // If the ship Id is valid, it will provide an owner.
    // If the ship Id is not valid, this will leave the ShipQuery object in a match-none state (no hull),
    // so default owner does not matter, and we can easily pass 0.
    complete(univ, shipList, config, scoreDefs, 0);
}

// Complete query, full version.
void
game::ShipQuery::complete(const game::map::Universe& univ,
                          const game::spec::ShipList& shipList,
                          const game::config::HostConfiguration& config,
                          const UnitScoreDefinitionList& scoreDefs,
                          const int defaultOwner)
{
    // ex GShipQuery::complete
    // ex shipacc.pas:FillHullFuncSelection, shipspec.pas:ShowWeaponEffects

    /*
      Item                  Derived from
      --------------------  --------------------
      m_hullType            m_shipId
      m_shipId              -
      m_levelFilterSet      defaults to all players
      m_levelDisplaySet     -
      m_playerFilterSet     defaults to all players
      m_playerDisplaySet    owner
      m_engineType          m_shipId
      m_combatMass          m_shipId or m_hullType, optionally m_engineType
      m_crew                m_shipId or m_hullType
      m_owner               m_shipId or m_playerDisplaySet, fallback to global player Id

      That is, to display information about a single ship, one has to fill in just
      m_shipId, m_levelDisplaySet, m_owner or m_playerDisplaySet; the rest is derived by complete().
    */

    int16_t level = -1;
    if (const game::map::Ship* sh = univ.ships().get(m_shipId)) {
        // We know the ship Id, so try to complete the request from the real universe
        if (m_hullType <= 0) {
            sh->getHull().get(m_hullType);
        }

        if (m_engineType <= 0) {
            sh->getEngineType().get(m_engineType);
        }

        if (m_crew <= 0) {
            sh->getCrew().get(m_crew);
        }

        if (m_owner <= 0) {
            sh->getRealOwner().get(m_owner);
        }

        UnitScoreDefinitionList::Index_t index;
        int16_t turn;
        if (scoreDefs.lookup(ScoreId_ExpLevel, index)) {
            if (sh->unitScores().get(index, level, turn)) {
                m_levelDisplaySet = ExperienceLevelSet_t(level);
            }
        }

        m_damage = sh->getDamage().orElse(0);
    }

    complete(shipList, config, defaultOwner, level);
}

// Complete query, non-universe version.
void
game::ShipQuery::complete(const game::spec::ShipList& shipList,
                          const game::config::HostConfiguration& config,
                          int defaultOwner,
                          int16_t level)
{
    // Do we have an owner?
    if (m_owner <= 0) {
        // Try to derive from m_playerDisplaySet. FIXME: needed?
        for (int i = 1; i <= MAX_PLAYERS; ++i) {
            if (m_playerDisplaySet == PlayerSet_t(i)) {
                m_owner = i;
            }
        }
    }
    if (m_owner <= 0) {
        // Use global default
        m_owner = defaultOwner;
    }

    // If we have an owner, try to complete m_playerDisplaySet
    if (m_owner > 0 && m_playerDisplaySet.empty()) {
        m_playerDisplaySet = PlayerSet_t(m_owner);
    }

    // Crew
    const game::spec::Hull* hull = shipList.hulls().get(m_hullType);
    if (m_crew <= 0 && hull != 0) {
        m_crew = hull->getMaxCrew();
    }

    // Now figure out combat mass
    if (m_combatMass <= 0 && hull != 0) {
        m_combatMass = hull->getMass();

        // Fed bonus
        if (config.getPlayerRaceNumber(m_owner) == 1 && config[config.AllowFedCombatBonus]()) {
            m_combatMass += 50;
        }

        // ESB
        int32_t esb = 0;
        if (config[config.AllowEngineShieldBonus]()) {
            esb += config[config.EngineShieldBonusRate](m_owner);
        }

        if (level >= 0) {
            esb += config.getExperienceBonus(config.EModEngineShieldBonusRate, level);
        }

        if (const game::spec::Engine* engine = shipList.engines().get(m_engineType)) {
            m_combatMass += engine->cost().get(game::spec::Cost::Money) * esb / 100;
            m_usedESBRate = esb;
        }
    }
}

// Enumerate ship functions.
void
game::ShipQuery::enumerateShipFunctions(game::spec::HullFunctionList& list,
                                        const game::map::Universe& univ,
                                        const game::spec::ShipList& shipList,
                                        const game::config::HostConfiguration& config,
                                        const bool includeRacialAbilities) const
{
    // ex GShipQuery::enumShipSpecials, hullfunc.pas::EnumHullfuncs
    if (shipList.hulls().get(m_hullType) != 0) {
        if (const game::map::Ship* ship = univ.ships().get(m_shipId)) {
            // We have a ship
            shipList.enumerateHullFunctions(list, m_hullType, config, m_playerFilterSet, m_levelFilterSet, false, includeRacialAbilities);

            // FIXME: enumerateShipFunctions should be limited by m_levelFilterSet?
            ship->enumerateShipFunctions(list, shipList);
        } else {
            // We don't have a ship, so list defaults
            shipList.enumerateHullFunctions(list, m_hullType, config, m_playerFilterSet, m_levelFilterSet, true, includeRacialAbilities);
        }
    }
}

// Compare for equality.
bool
game::ShipQuery::operator==(const ShipQuery& other) const
{
    // ex GShipQuery::operator==
    return m_hullType         == other.m_hullType
        && m_shipId           == other.m_shipId
        && m_levelFilterSet   == other.m_levelFilterSet
        && m_levelDisplaySet  == other.m_levelDisplaySet
        && m_playerFilterSet  == other.m_playerFilterSet
        && m_playerDisplaySet == other.m_playerDisplaySet
        && m_engineType       == other.m_engineType
        && m_combatMass       == other.m_combatMass
        && m_usedESBRate      == other.m_usedESBRate
        && m_crew             == other.m_crew
        && m_owner            == other.m_owner
        && m_damage           == other.m_damage;
}

// Compare for inequality.
bool
game::ShipQuery::operator!=(const ShipQuery& other) const
{
    return !operator==(other);
}

// Get hull type.
int
game::ShipQuery::getHullType() const
{
    // ex GShipQuery::getHullId
    return m_hullType;
}

// Set hull type.
void
game::ShipQuery::setHullType(int id)
{
    // ex GShipQuery::setHullId
    m_hullType = id;
}

// Get ship Id.
game::Id_t
game::ShipQuery::getShipId() const
{
    // ex GShipQuery::getShipId
    return m_shipId;
}

// Set ship Id.
void
game::ShipQuery::setShipId(Id_t id)
{
    // ex GShipQuery::setShipId
    m_shipId = id;
}

// Get experience level filter.
game::ExperienceLevelSet_t
game::ShipQuery::getLevelFilterSet() const
{
    // ex GShipQuery::getLevelFilterSet
    return m_levelFilterSet;
}

// Set experience level filter.
void
game::ShipQuery::setLevelFilterSet(ExperienceLevelSet_t set)
{
    // ex GShipQuery::setLevelFilterSet
    m_levelFilterSet = set;
}

// Get display level set.
game::ExperienceLevelSet_t
game::ShipQuery::getLevelDisplaySet() const
{
    // ex GShipQuery::getLevelDisplaySet
    return m_levelDisplaySet;
}

// Set display level set.
void
game::ShipQuery::setLevelDisplaySet(ExperienceLevelSet_t set)
{
    // ex GShipQuery::setLevelDisplaySet
    m_levelDisplaySet = set;
}

// Get player filter.
game::PlayerSet_t
game::ShipQuery::getPlayerFilterSet() const
{
    // ex GShipQuery::getPlayerFilterSet
    return m_playerFilterSet;
}

// Set player filter.
void
game::ShipQuery::setPlayerFilterSet(PlayerSet_t set)
{
    // ex GShipQuery::setPlayerFilterSet
    m_playerFilterSet = set;
}

// Get display player set.
game::PlayerSet_t
game::ShipQuery::getPlayerDisplaySet() const
{
    // ex GShipQuery::getPlayerDisplaySet
    return m_playerDisplaySet;
}

// Set display player set.
void
game::ShipQuery::setPlayerDisplaySet(PlayerSet_t set)
{
    // ex GShipQuery::setPlayerDisplaySet
    m_playerDisplaySet = set;
}

// Get engine type.
int
game::ShipQuery::getEngineType() const
{
    // ex GShipQuery::getEngineId
    return m_engineType;
}

// Set engine type.
void
game::ShipQuery::setEngineType(int type)
{
    // ex GShipQuery::setEngineId
    m_engineType = type;
}

// Get combat mass.
int
game::ShipQuery::getCombatMass() const
{
    // ex GShipQuery::getCombatMass
    return m_combatMass;
}

// Get used engine shield bonus rate.
int
game::ShipQuery::getUsedESBRate() const
{
    // ex GShipQuery::getUsedESB
    return m_usedESBRate;
}

// Set combat mass parameters.
void
game::ShipQuery::setCombatMass(int mass, int usedESB)
{
    // ex GShipQuery::setCombatMass
    m_combatMass = mass;
    m_usedESBRate = usedESB;
}

// Get crew size.
int
game::ShipQuery::getCrew() const
{
    // ex GShipQuery::getCrew
    return m_crew;
}

// Set crew size.
void
game::ShipQuery::setCrew(int crew)
{
    // ex GShipQuery::setCrew
    m_crew = crew;
}

// Get owner.
int
game::ShipQuery::getOwner() const
{
    // ex GShipQuery::getOwner
    return m_owner;
}

// Set owner.
void
game::ShipQuery::setOwner(int id)
{
    // ex GShipQuery::setOwner
    m_owner = id;
}

// Get damage.
int
game::ShipQuery::getDamage() const
{
    // ex GShipQuery::getDamage
    return m_damage;
}

// Set damage.
void
game::ShipQuery::setDamage(int damage)
{
    // ex GShipQuery::setDamage
    m_damage = damage;
}
