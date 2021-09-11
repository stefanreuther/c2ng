/**
  *  \file game/map/historyshiptype.cpp
  *  \brief Class game::map::HistoryShipType
  */

#include "game/map/historyshiptype.hpp"

game::map::HistoryShipType::HistoryShipType(ObjectVector<Ship>& vec)
    : ObjectVectorType<Ship>(vec)
{ }

bool
game::map::HistoryShipType::isValid(const Ship& s) const
{
    // ex GHistoryShipType::isValidIndex (sort-of)
    return s.getShipKind() != Ship::NoShip;
}
