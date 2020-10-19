/**
  *  \file game/sim/classresult.cpp
  *  \brief Class game::sim::ClassResult
  */

#include <cassert>
#include "game/sim/classresult.hpp"
#include "game/sim/setup.hpp"
#include "game/sim/object.hpp"

game::sim::ClassResult::ClassResult(const Setup& newState, const Result& result)
    : m_ownedUnits(),
      m_weight(result.this_battle_weight),
      m_sampleBattle(result.battles)
{
    for (Setup::Slot_t i = 0, n = newState.getNumObjects(); i < n; ++i) {
        if (const Object* obj = newState.getObject(i)) {
            if (int* p = m_ownedUnits.at(obj->getOwner())) {
                ++*p;
            }
        }
    }
}

game::sim::ClassResult::~ClassResult()
{ }

const game::PlayerArray<int>&
game::sim::ClassResult::getClass() const
{
    return m_ownedUnits;
}

int32_t
game::sim::ClassResult::getWeight() const
{
    return m_weight;
}

game::sim::Database_t
game::sim::ClassResult::getSampleBattle() const
{
    return m_sampleBattle;
}

void
game::sim::ClassResult::changeWeight(int32_t oldWeight, int32_t newWeight)
{
    // GSimResultSummary::ClassResult::changeWeight
    m_weight = m_weight * newWeight / oldWeight;
}

bool
game::sim::ClassResult::isSameClass(const ClassResult& other) const
{
    // GSimResultSummary::ClassResult::isSameClass
    for (int i = 1; i <= MAX_PLAYERS; ++i) {
        if (m_ownedUnits.get(i) != other.m_ownedUnits.get(i)) {
            return false;
        }
    }
    return true;
}

void
game::sim::ClassResult::addSameClassResult(const ClassResult& other)
{
    // GSimResultSummary::ClassResult::addNewSameClassResult
    assert(isSameClass(other));

    m_weight += other.m_weight;
    m_sampleBattle = other.m_sampleBattle;
}
