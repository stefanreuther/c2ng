/**
  *  \file game/v3/trn/orfilter.cpp
  */

#include "game/v3/trn/orfilter.hpp"

game::v3::trn::OrFilter::OrFilter(const Filter& lhs, const Filter& rhs)
    : m_lhs(lhs),
      m_rhs(rhs)
{ }

bool
game::v3::trn::OrFilter::accept(const TurnFile& trn, size_t index) const
{
    // ex FilterOr::accept
    return m_lhs.accept(trn, index)
        || m_rhs.accept(trn, index);
}
