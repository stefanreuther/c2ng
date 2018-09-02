/**
  *  \file game/battleorderrule.cpp
  */

#include "game/battleorderrule.hpp"
#include "game/spec/friendlycodelist.hpp"

game::BattleOrderRule::BattleOrderRule(HostVersion host)
    : m_host(host)
{ }

int
game::BattleOrderRule::get(const game::map::Object& obj) const
{
    if (const game::map::Ship* sh = dynamic_cast<const game::map::Ship*>(&obj)) {
        return get(*sh);
    } else if (const game::map::Planet* pl = dynamic_cast<const game::map::Planet*>(&obj)) {
        return get(*pl);
    } else {
        return UNKNOWN;
    }
}

int
game::BattleOrderRule::get(const game::map::Ship& sh) const
{
    // ex game/objl-sort.cc:sortByBattleOrder (part)
    // Friendly code
    String_t friendlyCode;
    if (!sh.getFriendlyCode().get(friendlyCode)) {
        return UNKNOWN;
    }

    // Mission
    int mission;
    bool hasKillMission = (sh.getMission().get(mission) && mission == game::spec::Mission::msn_Kill);

    // Weapons
    int numBeams, numLaunchers, numBays;
    bool hasWeapons
        = (sh.getNumBeams().get(numBeams) && numBeams > 0)
        || (sh.getNumLaunchers().get(numLaunchers) && numLaunchers > 0)
        || (sh.getNumBays().get(numBays) && numBays > 0);

    // Enemy
    int enemy;
    bool hasEnemy = (sh.getPrimaryEnemy().get(enemy) && enemy != 0);

    return getShipBattleOrder(friendlyCode, hasWeapons, hasEnemy, hasKillMission);
}

int
game::BattleOrderRule::get(const game::map::Planet& pl) const
{
    // ex game/objl-sort.cc:sortByBattleOrder (part)
    // Friendly code
    String_t friendlyCode;
    if (!pl.getFriendlyCode().get(friendlyCode)) {
        return UNKNOWN;
    }

    // Defense
    int defense;
    bool hasDefense = (pl.getNumBuildings(DefenseBuilding).get(defense) && defense > 0);

    return getPlanetBattleOrder(friendlyCode, hasDefense);
}

int
game::BattleOrderRule::get(const game::sim::Object& obj) const
{
    // ex game/sim-run.cc:getFCodeValuePHost (sort-of)
    if (const game::sim::Ship* sh = dynamic_cast<const game::sim::Ship*>(&obj)) {
        return get(*sh);
    } else if (const game::sim::Planet* pl = dynamic_cast<const game::sim::Planet*>(&obj)) {
        return get(*pl);
    } else {
        return UNKNOWN;
    }
}

int
game::BattleOrderRule::get(const game::sim::Ship& sh) const
{
    // ex game/sim-run.cc:getFCodeValueTHost
    bool hasWeapons = (sh.getNumBeams() != 0) || (sh.getNumLaunchers() != 0) || (sh.getNumBays() != 0);

    // Aggressiveness/Kill.
    // Treat "Kill" as "has enemy" as well because sim cannot distinguish between kill with or without enemy.
    int agg = sh.getAggressiveness();
    bool hasEnemy = (agg == sh.agg_Kill || (agg > sh.agg_Passive && agg < sh.agg_NoFuel));
    bool hasKillMission = (agg == sh.agg_Kill);

    return getShipBattleOrder(sh.getFriendlyCode(), hasWeapons, hasEnemy, hasKillMission);
}

int
game::BattleOrderRule::get(const game::sim::Planet& pl) const
{
    bool hasDefense = (pl.getDefense() != 0);
    return getPlanetBattleOrder(pl.getFriendlyCode(), hasDefense);
}

int
game::BattleOrderRule::getShipBattleOrder(const String_t& friendlyCode,
                                          bool hasWeapons,
                                          bool hasEnemy,
                                          bool hasKillMission) const
{
    // ex game/objl-sort.cc:sortByBattleOrder (part)
    int value = game::spec::FriendlyCodeList::getNumericValue(friendlyCode, m_host);
    if (m_host.isPHost()) {
        if (value == 1000) {
            // 4.1e rule: Kill gets 1000, capital gets 1002, freighters get 1004.
            // (before: capital gets 1000, freighters get 1002)
            // FIXME: handle <4.1e
            if (hasKillMission) {
                value = 1000;
            } else if (hasWeapons) {
                value = 1002;
            } else {
                value = 1004;
            }
        }
    } else {
        if (value == 1000) {
            if (!hasKillMission) {
                value += 10;
            }
            if (!hasEnemy) {
                value += 5;
            }
        }
    }
    return value;
}

int
game::BattleOrderRule::getPlanetBattleOrder(const String_t& friendlyCode, bool hasDefense) const
{
    // ex game/objl-sort.cc:sortByBattleOrder (part)
    if (!m_host.isPHost()) {
        // Planets have no battle order in non-PHost
        return UNKNOWN;
    }

    int value;
    if (friendlyCode == "ATT" || friendlyCode == "NUK") {
        value = 0;
    } else {
        int value = game::spec::FriendlyCodeList::getNumericValue(friendlyCode, m_host);
        if (value == 1000) {
            if (hasDefense) {
                value = 1001;
            } else {
                value = 1003;
            }
        }
    }
    return value;
}
