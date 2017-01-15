/**
  *  \file game/v3/trn/indexfilter.cpp
  */

#include "game/v3/trn/indexfilter.hpp"

game::v3::trn::IndexFilter::IndexFilter(size_t from, size_t to)
    : Filter(),
      m_from(from),
      m_to(to)
{ }

bool
game::v3::trn::IndexFilter::accept(const TurnFile& /*trn*/, size_t index) const
{
    ++index;
    return index >= m_from && index <= m_to;
}
