/**
  *  \file game/v3/trn/negatefilter.cpp
  */

#include "game/v3/trn/negatefilter.hpp"

game::v3::trn::NegateFilter::NegateFilter(const Filter& other)
    : Filter(),
      m_other(other)
{ }

bool
game::v3::trn::NegateFilter::accept(const TurnFile& trn, size_t index) const
{
    return !m_other.accept(trn, index);
}
