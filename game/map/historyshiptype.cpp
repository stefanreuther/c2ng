/**
  *  \file game/map/historyshiptype.cpp
  */

#include "game/map/historyshiptype.hpp"
#include "game/map/universe.hpp"
#include "game/map/ship.hpp"

game::map::HistoryShipType::HistoryShipType(Universe& univ)
    : ObjectVectorType<Ship>(univ, univ.ships())
{ }

bool
game::map::HistoryShipType::isValid(const Ship& s) const
{
    // ex GHistoryShipType::isValidIndex (sort-of)
    return s.getShipKind() != Ship::NoShip;
}
