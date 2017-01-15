/**
  *  \file game/v3/trn/andfilter.cpp
  */

#include "game/v3/trn/andfilter.hpp"

game::v3::trn::AndFilter::AndFilter(const Filter& lhs, const Filter& rhs)
    : m_lhs(lhs), m_rhs(rhs)
{ }

bool
game::v3::trn::AndFilter::accept(const TurnFile& trn, size_t index) const
{
    // FilterAnd::accept
    return m_lhs.accept(trn, index)
        && m_rhs.accept(trn, index);
}
