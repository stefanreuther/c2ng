/**
  *  \file game/shipquery.cpp
  *
  *  FIXME: think about making it possible to call enumerateShipFunctions without a universe.
  *  PCC 2.0.5's solution is to make the parameter a pointer. This still links in the equivalent
  *  of the game::map namespace into a program that won't generate a Universe object, ever.
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
    hull_id            = 0;
    ship_id            = 0;
    filter_level_set   = ExperienceLevelSet_t::fromInteger(-1U);
    display_level_set  = ExperienceLevelSet_t::fromInteger(1);
    filter_player_set  = PlayerSet_t::fromInteger(-1U);
    display_player_set = PlayerSet_t::fromInteger(0);
    engine_id          = 0;
    combat_mass        = 0;
    used_esb           = 0;
    crew               = 0;
    owner              = 0;
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
    this->ship_id = shipId;

    // complete() will do most of the work.
    // If the ship Id is valid, it will provide an owner.
    // If the ship Id is not valid, this will leave the ShipQuery object in a match-none state (no hull),
    // so default owner does not matter, and we can easily pass 0.
    complete(univ, shipList, config, scoreDefs, 0);
}

// Complete query.
void
game::ShipQuery::complete(const game::map::Universe& univ,
                          const game::spec::ShipList& shipList,
                          const game::config::HostConfiguration& config,
                          const UnitScoreDefinitionList& scoreDefs,
                          const int defaultOwner)
{
    // ex GShipQuery::complete
    // ex shipacc.pas:FillHullFuncSelection

    // FIXME: split this to get a second signature that takes no universe/scoreDefs for use e.g. in sim.

    /*
      Item                  Derived from
      --------------------  --------------------
      hull_id               ship_id
      ship_id               -
      filter_level_set      defaults to all players
      display_level_set     -
      filter_player_set     defaults to all players
      display_player_set    owner
      engine_id             ship_id
      combat_mass           ship_id or hull_id, optionally engine_id
      crew                  ship_id or hull_id
      owner                 ship_id or display_player_set, fallback to global player Id

      That is, to display information about a single ship, one has to fill in just
      ship_id, display_level_set, owner or display_player_set; the rest is derived by complete().
    */

    int16_t level = -1;
    if (const game::map::Ship* sh = univ.ships().get(ship_id)) {
        // We know the ship Id, so try to complete the request from the real universe
        if (hull_id <= 0) {
            sh->getHull().get(hull_id);
        }

        if (engine_id <= 0) {
            sh->getEngineType().get(engine_id);
        }

        if (crew <= 0) {
            sh->getCrew().get(crew);
        }

        if (owner <= 0) {
            sh->getRealOwner().get(owner);
        }

        UnitScoreDefinitionList::Index_t index;
        int16_t turn;
        if (scoreDefs.lookup(ScoreId_ExpLevel, index)) {
            if (sh->unitScores().get(index, level, turn)) {
                display_level_set = ExperienceLevelSet_t(level);
            }
        }
    }

    // Do we have an owner?
    if (owner <= 0) {
        // Try to derive from display_player_set. FIXME: needed?
        for (int i = 1; i <= MAX_PLAYERS; ++i) {
            if (display_player_set == PlayerSet_t(i)) {
                owner = i;
            }
        }
    }
    if (owner <= 0) {
        // Use global default
        owner = defaultOwner;
    }

    // If we have an owner, try to complete display_player_set
    if (owner > 0 && display_player_set.empty()) {
        display_player_set = PlayerSet_t(owner);
    }

    // Crew
    const game::spec::Hull* hull = shipList.hulls().get(hull_id);
    if (crew <= 0 && hull != 0) {
        crew = hull->getMaxCrew();
    }

    // Now figure out combat mass
    if (combat_mass <= 0 && hull != 0) {
        combat_mass = hull->getMass();

        // Fed bonus
        if (config.getPlayerRaceNumber(owner) == 1 && config[config.AllowFedCombatBonus]()) {
            combat_mass += 50;
        }

        // ESB
        int32_t esb = 0;
        if (config[config.AllowEngineShieldBonus]()) {
            esb += config[config.EngineShieldBonusRate](owner);
        }

        if (level >= 0) {
            esb += config.getExperienceBonus(config.EModEngineShieldBonusRate, level);
        }

        if (const game::spec::Engine* engine = shipList.engines().get(engine_id)) {
            combat_mass += engine->cost().get(game::spec::Cost::Money) * esb / 100;
            used_esb = esb;
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
    // ex GShipQuery::enumShipSpecials
    // ex hullfunc.pas::EnumHullfuncs
    if (shipList.hulls().get(hull_id) != 0) {
        if (const game::map::Ship* ship = univ.ships().get(ship_id)) {
            // We have a ship
            shipList.enumerateHullFunctions(list, hull_id, config, filter_player_set, filter_level_set, false, includeRacialAbilities);

            // FIXME: enumerateShipFunctions should be limited by filter_level_set?
            ship->enumerateShipFunctions(list, shipList);
        } else {
            // We don't have a ship, so list defaults
            shipList.enumerateHullFunctions(list, hull_id, config, filter_player_set, filter_level_set, true, includeRacialAbilities);
        }
    }
}

// Compare for equality.
bool
game::ShipQuery::operator==(const ShipQuery& other) const
{
    // ex GShipQuery::operator==
    return hull_id            == other.hull_id
        && ship_id            == other.ship_id
        && filter_level_set   == other.filter_level_set
        && display_level_set  == other.display_level_set
        && filter_player_set  == other.filter_player_set
        && display_player_set == other.display_player_set
        && engine_id          == other.engine_id
        && combat_mass        == other.combat_mass
        && used_esb           == other.used_esb
        && crew               == other.crew
        && owner              == other.owner;
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
    return hull_id;
}

// Set hull type.
void
game::ShipQuery::setHullType(int id)
{
    // ex GShipQuery::setHullId
    this->hull_id = id;
}

// Get ship Id.
game::Id_t
game::ShipQuery::getShipId() const
{
    // ex GShipQuery::getShipId
    return ship_id;
}

// Set ship Id.
void
game::ShipQuery::setShipId(Id_t id)
{
    // ex GShipQuery::setShipId
    this->ship_id = id;
}

// Get experience level filter.
game::ExperienceLevelSet_t
game::ShipQuery::getLevelFilterSet() const
{
    // ex GShipQuery::getLevelFilterSet
    return filter_level_set;
}

// Set experience level filter.
void
game::ShipQuery::setLevelFilterSet(ExperienceLevelSet_t set)
{
    // ex GShipQuery::setLevelFilterSet
    this->filter_level_set = set;
}

// Get display level set.
game::ExperienceLevelSet_t
game::ShipQuery::getLevelDisplaySet() const
{
    // ex GShipQuery::getLevelDisplaySet
    return display_level_set;
}

// Set display level set.
void
game::ShipQuery::setLevelDisplaySet(ExperienceLevelSet_t set)
{
    // ex GShipQuery::setLevelDisplaySet
    this->display_level_set = set;
}

// Get player filter.
game::PlayerSet_t
game::ShipQuery::getPlayerFilterSet() const
{
    // ex GShipQuery::getPlayerFilterSet
    return filter_player_set;
}

// Set player filter.
void
game::ShipQuery::setPlayerFilterSet(PlayerSet_t set)
{
    // ex GShipQuery::setPlayerFilterSet
    this->filter_player_set = set;
}

// Get display player set.
game::PlayerSet_t
game::ShipQuery::getPlayerDisplaySet() const
{
    // ex GShipQuery::getPlayerDisplaySet
    return display_player_set;
}

// Set display player set.
void
game::ShipQuery::setPlayerDisplaySet(PlayerSet_t set)
{
    // ex GShipQuery::setPlayerDisplaySet
    this->display_player_set = set;
}

// Get engine type.
int
game::ShipQuery::getEngineType() const
{
    // ex GShipQuery::getEngineId
    return engine_id;
}

// Set engine type.
void
game::ShipQuery::setEngineType(int type)
{
    // ex GShipQuery::setEngineId
    this->engine_id = type;
}

// Get combat mass.
int
game::ShipQuery::getCombatMass() const
{
    // ex GShipQuery::getCombatMass
    return combat_mass;
}

// Get used engine shield bonus rate.
int
game::ShipQuery::getUsedESBRate() const
{
    // ex GShipQuery::getUsedESB
    return used_esb;
}

// Set combat mass parameters.
void
game::ShipQuery::setCombatMass(int mass, int usedESB)
{
    // ex GShipQuery::setCombatMass
    this->combat_mass = mass;
    this->used_esb = usedESB;
}

// Get crew size.
int
game::ShipQuery::getCrew() const
{
    // ex GShipQuery::getCrew
    return crew;
}

// Set crew size.
void
game::ShipQuery::setCrew(int crew)
{
    // ex GShipQuery::setCrew
    this->crew = crew;
}

// Get owner.
int
game::ShipQuery::getOwner() const
{
    // ex GShipQuery::getOwner
    return owner;
}

// Set owner.
void
game::ShipQuery::setOwner(int id)
{
    // ex GShipQuery::setOwner
    this->owner = id;
}
