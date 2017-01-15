/**
  *  \file game/v3/trn/idfilter.cpp
  */

#include "game/v3/trn/idfilter.hpp"

game::v3::trn::IdFilter::IdFilter(Id_t lower, Id_t upper)
    : Filter(),
      m_lower(lower),
      m_upper(upper)
{ }

bool
game::v3::trn::IdFilter::accept(const TurnFile& trn, size_t index) const
{
    // ex FilterId::accept
    TurnFile::CommandType cmdType;
    int id;
    if (trn.getCommandType(index, cmdType)
        && (cmdType == TurnFile::ShipCommand
            || cmdType == TurnFile::PlanetCommand
            || cmdType == TurnFile::BaseCommand)
        && trn.getCommandId(index, id))
    {
        return (id >= m_lower) && (id <= m_upper);
    } else {
        return false;
    }
}
