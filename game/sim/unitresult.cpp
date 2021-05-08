/**
  *  \file game/sim/unitresult.cpp
  *  \brief Class game::sim::UnitResult
  */

#include "game/sim/unitresult.hpp"
#include "game/sim/ship.hpp"
#include "game/sim/planet.hpp"


// Make blank result.
game::sim::UnitResult::Item::Item()
    : min(0), max(0), totalScaled(0), minSpecimen(), maxSpecimen()
{
    // ex GSimStatItem
}

// Make inverted result.
game::sim::UnitResult::Item::Item(const Item& orig, int32_t subtract_from, int32_t scale)
    : min(subtract_from - orig.max),
      max(subtract_from - orig.min),
      totalScaled(subtract_from * scale - orig.totalScaled),
      minSpecimen(orig.maxSpecimen),
      maxSpecimen(orig.minSpecimen)
{ }


game::sim::UnitResult::UnitResult()
    : m_numFightsWon(0), m_numFights(0), m_numCaptures(0)
{ }

int
game::sim::UnitResult::getNumFightsWon() const
{
    return m_numFightsWon;
}

int
game::sim::UnitResult::getNumFights() const
{
    return m_numFights;
}

int
game::sim::UnitResult::getNumCaptures() const
{
    return m_numCaptures;
}

const game::sim::UnitResult::Item&
game::sim::UnitResult::getNumTorpedoesFired() const
{
    return m_numTorpedoesFired;
}

const game::sim::UnitResult::Item&
game::sim::UnitResult::getNumFightersLost() const
{
    return m_numFightersLost;
}

const game::sim::UnitResult::Item&
game::sim::UnitResult::getDamage() const
{
    return m_damage;
}

const game::sim::UnitResult::Item&
game::sim::UnitResult::getShield() const
{
    return m_shield;
}

const game::sim::UnitResult::Item&
game::sim::UnitResult::getCrewLeftOrDefenseLost() const
{
    return m_crewLeftOrDefenseLost;
}

const game::sim::UnitResult::Item&
game::sim::UnitResult::getNumTorpedoHits() const
{
    return m_numTorpedoHits;
}

const game::sim::UnitResult::Item&
game::sim::UnitResult::getMinFightersAboard() const
{
    return m_minFightersAboard;
}

// Change weight of this unit result.
void
game::sim::UnitResult::changeWeight(int32_t oldWeight, int32_t newWeight)
{
    // ex GSimResultSummary::UnitResult::changeWeight
    changeWeight(m_numTorpedoesFired,     oldWeight, newWeight);
    changeWeight(m_numFightersLost,       oldWeight, newWeight);
    changeWeight(m_damage,                oldWeight, newWeight);
    changeWeight(m_shield,                oldWeight, newWeight);
    changeWeight(m_crewLeftOrDefenseLost, oldWeight, newWeight);
    changeWeight(m_numTorpedoHits,        oldWeight, newWeight);
    changeWeight(m_minFightersAboard,     oldWeight, newWeight);

    m_numFights    = m_numFights    * newWeight / oldWeight;
    m_numFightsWon = m_numFightsWon * newWeight / oldWeight;
    m_numCaptures  = m_numCaptures  * newWeight / oldWeight;
}

// Add unit result from ship.
void
game::sim::UnitResult::addResult(const Ship& oldShip, const Ship& newShip, const game::vcr::Statistic& stat, const Result& res)
{
    // ex GSimResultSummary::UnitResult::addResult
    /* Overall result */
    if (stat.getNumFights() != 0) {
        m_numFights += res.this_battle_weight;
    }
    if (newShip.getOwner() != 0) {
        if (newShip.getOwner() == oldShip.getOwner()) {
            m_numFightsWon += res.this_battle_weight;
        } else {
            m_numCaptures += res.this_battle_weight;
        }
    }

    /* Statistics counters */
    // FIXME: NTP?
    if (oldShip.getNumLaunchers() != 0) {
        add(m_numTorpedoesFired, oldShip.getAmmo() - newShip.getAmmo(), res);
    } else {
        add(m_numTorpedoesFired, 0, res);
    }

    if (oldShip.getNumBays() != 0) {
        add(m_numFightersLost, oldShip.getAmmo() - newShip.getAmmo(), res);
    } else {
        add(m_numFightersLost, 0, res);
    }

    add(m_damage, newShip.getDamage(), res);
    add(m_shield, newShip.getShield(), res);
    add(m_crewLeftOrDefenseLost, newShip.getCrew(), res);

    if (oldShip.getNumLaunchers() != 0) {
        add(m_numTorpedoHits, stat.getNumTorpedoHits(), res);
    }
    if (oldShip.getNumBays() != 0) {
        add(m_minFightersAboard, stat.getMinFightersAboard(), res);
    }
}

// Add unit result from planet.
void
game::sim::UnitResult::addResult(const Planet& oldPlanet, const Planet& newPlanet, const game::vcr::Statistic& stat, const Result& res)
{
    // ex GSimResultSummary::UnitResult::addResult
    /* Overall result */
    if (stat.getNumFights() != 0) {
        ++m_numFights;
    }
    if (newPlanet.getOwner() != 0) {
        if (newPlanet.getOwner() == oldPlanet.getOwner()) {
            ++m_numFightsWon;
        } else {
            ++m_numCaptures;
        }
    }

    /* Statistics counters */
    // FIXME: m_numTorpedoesFired
    add(m_numFightersLost, oldPlanet.getNumBaseFighters() - newPlanet.getNumBaseFighters(), res);
    add(m_damage, newPlanet.getDamage(), res);
    add(m_shield, newPlanet.getShield(), res);
    add(m_crewLeftOrDefenseLost, oldPlanet.getDefense() - newPlanet.getDefense(), res);

    add(m_numTorpedoHits,    stat.getNumTorpedoHits(),    res);
    add(m_minFightersAboard, stat.getMinFightersAboard(), res);
}

// Add single result value.
void
game::sim::UnitResult::add(Item& it, int32_t value, const Result& w)
{
    // ex GSimStatItem::add
    if (w.this_battle_index == 0) {
        it.min = it.max = value;
        it.minSpecimen = it.maxSpecimen = w.battles;
    } else {
        if (it.min > value) {
            it.min = value;
            it.minSpecimen = w.battles;
        }
        if (it.max < value) {
            it.max = value;
            it.maxSpecimen = w.battles;
        }
    }
    it.totalScaled += value * w.this_battle_weight;
}

// Change weight proportionally.
void
game::sim::UnitResult::changeWeight(Item& it, int32_t oldWeight, int32_t newWeight)
{
    it.totalScaled = it.totalScaled * newWeight / oldWeight;
}
