/**
  *  \file game/sim/resultlist.cpp
  *  \brief Class game::sim::ResultList
  */

#include "game/sim/resultlist.hpp"
#include "game/sim/setup.hpp"
#include "game/sim/ship.hpp"
#include "game/sim/planet.hpp"

// Make blank ResultList.
game::sim::ResultList::ResultList()
    : m_totalWeight(0), m_cumulativeWeight(0), m_numBattles(0), m_unitResults(), m_classResults()
{
    // ex GSimResultSummary
}

// Destructor.
game::sim::ResultList::~ResultList()
{ }

// Incorporate result into this object.
void
game::sim::ResultList::addResult(const Setup& oldState, const Setup& newState, afl::base::Memory<const game::vcr::Statistic> stats, Result result)
{
    // ex GSimResultSummary::addResult
    // Check validity of parameters
    assert(oldState.getNumObjects() == newState.getNumObjects());

    // Build m_unitResults on first iteration
    if (m_unitResults.empty()) {
        for (Setup::Slot_t i = 0, n = oldState.getNumObjects(); i < n; ++i) {
            m_unitResults.pushBackNew(new UnitResult());
        }
        m_totalWeight = result.total_battle_weight;
    }

    // Adjust weights. This should never be needed if the driver works correctly, but it doesn't hurt.
    if (m_totalWeight < result.total_battle_weight) {
        // The new battle has a higher weight; upgrade existing results
        for (UnitResults_t::iterator i = m_unitResults.begin(); i != m_unitResults.end(); ++i) {
            (*i)->changeWeight(m_totalWeight, result.total_battle_weight);
        }
        for (ClassResults_t::iterator i = m_classResults.begin(); i != m_classResults.end(); ++i) {
            (*i)->changeWeight(m_totalWeight, result.total_battle_weight);
        }

        m_cumulativeWeight = 0;
        for (ClassResults_t::iterator i = m_classResults.begin(); i != m_classResults.end(); ++i) {
            m_cumulativeWeight += (*i)->getWeight();
        }

        m_totalWeight = result.total_battle_weight;
    }

    if (m_totalWeight > result.total_battle_weight) {
        // The battle has a lower weight; upgrade new battle.
        result.changeWeightTo(m_totalWeight);
    }
    assert(result.total_battle_weight == m_totalWeight);

    // Add new unit results
    for (Setup::Slot_t i = 0, n = oldState.getNumObjects(); i < n; ++i) {
        const game::vcr::Statistic* pStat = stats.eat();
        if (const Ship* oldShip = dynamic_cast<const Ship*>(oldState.getObject(i))) {
            const Ship* newShip = dynamic_cast<const Ship*>(newState.getObject(i));
            assert(newShip);
            m_unitResults[i]->addResult(*oldShip, *newShip, pStat ? *pStat : game::vcr::Statistic(), result);
        } else if (const Planet* oldPlanet = dynamic_cast<const Planet*>(oldState.getObject(i))) {
            const Planet* newPlanet = dynamic_cast<const Planet*>(newState.getObject(i));
            assert(newPlanet);
            m_unitResults[i]->addResult(*oldPlanet, *newPlanet, pStat ? *pStat : game::vcr::Statistic(), result);
        } else {
            assert(0);
        }
    }

    /* And add it to the class results */
    ClassResult this_class(newState, result);
    bool have_this_class = false;
    for (ClassResults_t::iterator i = m_classResults.begin(); i != m_classResults.end(); ++i) {
        if ((*i)->isSameClass(this_class)) {
            (*i)->addSameClassResult(this_class);
            have_this_class = true;
            updateClassResultSortOrder(i - m_classResults.begin());
            break;
        }
    }
    if (!have_this_class) {
        m_classResults.pushBackNew(new ClassResult(this_class));
        updateClassResultSortOrder(m_classResults.size() - 1);
    }

    /* Finally, adjust our counters */
    m_cumulativeWeight += result.this_battle_weight;
    ++m_numBattles;
}

// Get cumulative weight.
int32_t
game::sim::ResultList::getCumulativeWeight() const
{
    // ex GSimResultSummary::getCumulativeWeight
    return m_cumulativeWeight;
}

// Get total weight to which weights are normalized.
int32_t
game::sim::ResultList::getTotalWeight() const
{
    // ex GSimResultSummary::getTotalWeight
    return m_totalWeight;
}

// Get number of result classes.
size_t
game::sim::ResultList::getNumClassResults() const
{
    // ex GSimResultSummary::getNumClassResults
    return m_classResults.size();
}

// Get number of unit results.
size_t
game::sim::ResultList::getNumUnitResults() const
{
    // ex GSimResultSummary::getNumUnitResults
    return m_unitResults.size();
}

// Get class result.
const game::sim::ClassResult*
game::sim::ResultList::getClassResult(size_t index) const
{
    // ex GSimResultSummary::getUnitResult
    return m_classResults[index];
}

// Get unit result.
const game::sim::UnitResult*
game::sim::ResultList::getUnitResult(size_t index) const
{
    // ex GSimResultSummary::getUnitResult
    return m_unitResults[index];
}

// Get number of battles fought.
size_t
game::sim::ResultList::getNumBattles() const
{
    // ex GSimResultSummary::getNumBattles
    return m_numBattles;
}


/** Update class result sort order. Assuming value at change_index was
    modified (count increased), sort it into its place. */
void
game::sim::ResultList::updateClassResultSortOrder(size_t change_index)
{
    while (change_index > 0) {
        if (m_classResults[change_index-1]->getWeight() < m_classResults[change_index]->getWeight()) {
            /* swap them */
            m_classResults.swapElements(change_index-1, change_index);
            change_index--;
        } else {
            /* finish */
            break;
        }
    }
}
