/**
  *  \file game/v3/trn/constantfilter.cpp
  */

#include "game/v3/trn/constantfilter.hpp"

game::v3::trn::ConstantFilter::ConstantFilter(bool value)
    : m_value(value)
{ }

bool
game::v3::trn::ConstantFilter::accept(const TurnFile& /*trn*/, size_t /*index*/) const
{
    // ex FilterConst::accept
    return m_value;
}

