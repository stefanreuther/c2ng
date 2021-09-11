/**
  *  \file game/map/anyshiptype.cpp
  *  \brief Class game::map::AnyShipType
  */

#include "game/map/anyshiptype.hpp"

game::map::AnyShipType::AnyShipType(ObjectVector<Ship>& vec)
    : ObjectVectorType<Ship>(vec)
{ }

bool
game::map::AnyShipType::isValid(const Ship& s) const
{
    // ex GAnyShipType::isValidIndex (sort-of)
    return s.isVisible();
}
