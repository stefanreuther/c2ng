/**
  *  \file game/spec/costsummary.cpp
  *  \brief Class game::spec::CostSummary
  */

#include "game/spec/costsummary.hpp"

game::spec::CostSummary::CostSummary()
    : m_items()
{ }

game::spec::CostSummary::~CostSummary()
{ }

void
game::spec::CostSummary::clear()
{
    m_items.clear();
}

void
game::spec::CostSummary::add(const Item& item)
{
    m_items.push_back(item);
}

size_t
game::spec::CostSummary::getNumItems() const
{
    return m_items.size();
}

const game::spec::CostSummary::Item*
game::spec::CostSummary::get(size_t index) const
{
    return (index < m_items.size()
            ? &m_items[index]
            : 0);
}

const game::spec::CostSummary::Item*
game::spec::CostSummary::find(Id_t id, size_t* pIndex) const
{
    for (size_t i = 0, n = m_items.size(); i < n; ++i) {
        if (m_items[i].id == id) {
            if (pIndex != 0) {
                *pIndex = i;
            }
            return &m_items[i];
        }
    }
    return 0;
}

game::spec::Cost
game::spec::CostSummary::getTotalCost() const
{
    // ex cost.pas:SumBill
    Cost sum;
    for (size_t i = 0, n = m_items.size(); i < n; ++i) {
        sum += m_items[i].cost;
    }
    return sum;
}
