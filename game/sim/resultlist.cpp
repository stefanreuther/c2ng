/**
  *  \file game/sim/resultlist.cpp
  *  \brief Class game::sim::ResultList
  */

#include "game/sim/resultlist.hpp"
#include "game/sim/setup.hpp"
#include "game/sim/ship.hpp"
#include "game/sim/planet.hpp"
#include "afl/string/format.hpp"

namespace {
    afl::base::Ptr<game::vcr::Database> pickSample(const game::sim::UnitResult::Item& item, bool max)
    {
        return max ? item.maxSpecimen : item.minSpecimen;
    }
}

const size_t game::sim::ResultList::UnitInfo::MAX_TYPE;

// Make blank ResultList.
game::sim::ResultList::ResultList()
    : m_totalWeight(0), m_cumulativeWeight(0), m_numBattles(0), m_lastClassResultIndex(0), m_unitResults(), m_classResults()
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
            m_lastClassResultIndex = updateClassResultSortOrder(i - m_classResults.begin());
            break;
        }
    }
    if (!have_this_class) {
        m_classResults.pushBackNew(new ClassResult(this_class));
        m_lastClassResultIndex = updateClassResultSortOrder(m_classResults.size() - 1);
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

// Describe class result.
game::sim::ResultList::ClassInfo
game::sim::ResultList::describeClassResult(size_t index, const util::NumberFormatter& fmt) const
{
    ClassInfo result;
    if (const ClassResult* p = getClassResult(index)) {
        // Label
        double perc = (getCumulativeWeight() == 0 ? 0.0 : 100.0 * p->getWeight() / getCumulativeWeight());
        if (getTotalWeight() == 1) {
            result.label = afl::string::Format("%d\xC3\x97 (%.1f%%)", fmt.formatNumber(p->getWeight()), perc);
        } else {
            result.label = afl::string::Format("%.1f%%", perc);
        }

        // Rest
        result.weight     = p->getWeight();
        result.ownedUnits = p->getClass();
        result.hasSample  = p->getSampleBattle().get() != 0;
    }
    return result;
}

// Describe unit result.
game::sim::ResultList::UnitInfo
game::sim::ResultList::describeUnitResult(size_t index, const Setup& setup) const
{
    UnitInfo result;
    const Object* obj = setup.getObject(index);
    const UnitResult* r = getUnitResult(index);
    if (obj != 0 && r != 0) {
        // Scalars
        result.numFightsWon      = r->getNumFightsWon();
        result.numFights         = r->getNumFights();
        result.numCaptures       = r->getNumCaptures();
        result.cumulativeWeight  = getCumulativeWeight();
        result.hasAbsoluteCounts = getTotalWeight() <= 1;

        // Formatting logic taken from WSimUnitStat::render
        result.info.push_back(packItem(UnitInfo::Damage, r->getDamage()));
        result.info.push_back(packItem(UnitInfo::Shield, r->getShield()));
        if (dynamic_cast<const Planet*>(obj) != 0) {
            result.info.push_back(packItem(UnitInfo::DefenseLost, r->getCrewLeftOrDefenseLost()));
            result.info.push_back(packItem(UnitInfo::NumBaseFightersLost, r->getNumFightersLost()));
            if (r->getNumFights() != 0) {
                result.info.push_back(packItem(UnitInfo::MinFightersAboard, r->getMinFightersAboard()));
            }
        }
        if (const Ship* sh = dynamic_cast<const Ship*>(obj)) {
            result.info.push_back(packItem(UnitInfo::Crew, r->getCrewLeftOrDefenseLost()));
            if (sh->getNumBays() != 0) {
                result.info.push_back(packItem(UnitInfo::NumFightersLost, r->getNumFightersLost()));
                result.info.push_back(packItem(UnitInfo::NumFightersRemaining, UnitResult::Item(r->getNumFightersLost(), sh->getAmmo(), getCumulativeWeight())));
                result.info.push_back(packItem(UnitInfo::MinFightersAboard, r->getMinFightersAboard()));
            }
            if (sh->getNumLaunchers() != 0) {
                result.info.push_back(packItem(UnitInfo::NumTorpedoesFired, r->getNumTorpedoesFired()));
                result.info.push_back(packItem(UnitInfo::NumTorpedoesRemaining, UnitResult::Item(r->getNumTorpedoesFired(), sh->getAmmo(), getCumulativeWeight())));
                result.info.push_back(packItem(UnitInfo::NumTorpedoHits, r->getNumTorpedoHits()));
            }
        }
    }
    return result;
}

// Get sample battle.
afl::base::Ptr<game::vcr::Database>
game::sim::ResultList::getUnitSampleBattle(size_t index, UnitInfo::Type type, bool max) const
{
    if (const UnitResult* r = getUnitResult(index)) {
        switch (type) {
         case ResultList::UnitInfo::Damage:                return pickSample(r->getDamage(),                max);
         case ResultList::UnitInfo::Shield:                return pickSample(r->getShield(),                max);
         case ResultList::UnitInfo::DefenseLost:           return pickSample(r->getCrewLeftOrDefenseLost(), max);
         case ResultList::UnitInfo::NumBaseFightersLost:   return pickSample(r->getNumFightersLost(),       max);
         case ResultList::UnitInfo::MinFightersAboard:     return pickSample(r->getMinFightersAboard(),     max);
         case ResultList::UnitInfo::Crew:                  return pickSample(r->getCrewLeftOrDefenseLost(), max);
         case ResultList::UnitInfo::NumFightersLost:       return pickSample(r->getNumFightersLost(),       max);
         case ResultList::UnitInfo::NumFightersRemaining:  return pickSample(r->getNumFightersLost(),      !max);
         case ResultList::UnitInfo::NumTorpedoesFired:     return pickSample(r->getNumTorpedoesFired(),     max);
         case ResultList::UnitInfo::NumTorpedoesRemaining: return pickSample(r->getNumTorpedoesFired(),    !max);
         case ResultList::UnitInfo::NumTorpedoHits:        return pickSample(r->getNumTorpedoHits(),        max);
        }
    }
    return 0;
}

// Get number of battles fought.
size_t
game::sim::ResultList::getNumBattles() const
{
    // ex GSimResultSummary::getNumBattles
    return m_numBattles;
}

// Get class result index of last result added.
size_t
game::sim::ResultList::getLastClassResultIndex() const
{
    return m_lastClassResultIndex;
}


/** Update class result sort order. Assuming value at change_index was
    modified (count increased), sort it into its place. */
size_t
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
    return change_index;
}

/** Pack UnitResult::Item into UnitInfo::Item. */
game::sim::ResultList::UnitInfo::Item
game::sim::ResultList::packItem(UnitInfo::Type type, const UnitResult::Item& item) const
{
    return UnitInfo::Item(type,
                          item.min,
                          item.max,
                          double(item.totalScaled) / getCumulativeWeight(),
                          item.minSpecimen.get() != 0,
                          item.maxSpecimen.get() != 0);
}

// Get human-readable string representation of a UnitInfo::Type.
String_t
game::sim::toString(ResultList::UnitInfo::Type type, afl::string::Translator& tx)
{
    switch (type) {
     case ResultList::UnitInfo::Damage:                return tx("Damage");
     case ResultList::UnitInfo::Shield:                return tx("Shield");
     case ResultList::UnitInfo::DefenseLost:           return tx("Defense Lost");
     case ResultList::UnitInfo::NumBaseFightersLost:   return tx("SB Ftrs Lost");
     case ResultList::UnitInfo::MinFightersAboard:     return tx("Min Ftr Aboard");
     case ResultList::UnitInfo::Crew:                  return tx("Crew Left");
     case ResultList::UnitInfo::NumFightersLost:       return tx("Fighters Lost");
     case ResultList::UnitInfo::NumFightersRemaining:  return tx("Fighters Left");
     case ResultList::UnitInfo::NumTorpedoesFired:     return tx("Torps Fired");
     case ResultList::UnitInfo::NumTorpedoesRemaining: return tx("Torps Left");
     case ResultList::UnitInfo::NumTorpedoHits:        return tx("Torps Hit");
    }
    return String_t();
}
