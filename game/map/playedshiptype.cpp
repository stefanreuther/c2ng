/**
  *  \file game/map/playedshiptype.cpp
  */

#include "game/map/playedshiptype.hpp"
#include "game/map/universe.hpp"
#include "game/map/ship.hpp"

game::map::PlayedShipType::PlayedShipType(Universe& univ)
    : ObjectVectorType<Ship>(univ.ships())
{ }

bool
game::map::PlayedShipType::isValid(const Ship& s) const
{
    // ex GPlayedShipType::isValidIndex (sort-of)
    return s.isPlayable(Ship::ReadOnly);
}
