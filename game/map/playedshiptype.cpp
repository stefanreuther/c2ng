/**
  *  \file game/map/playedshiptype.cpp
  *  \brief Class game::map::PlayedShipType
  */

#include "game/map/playedshiptype.hpp"

game::map::PlayedShipType::PlayedShipType(ObjectVector<Ship>& vec)
    : ObjectVectorType<Ship>(vec)
{ }

bool
game::map::PlayedShipType::isValid(const Ship& s) const
{
    // ex GPlayedShipType::isValidIndex (sort-of)
    return s.isPlayable(Ship::ReadOnly);
}
