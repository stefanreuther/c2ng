/**
  *  \file game/map/anyshiptype.cpp
  */

#include "game/map/anyshiptype.hpp"
#include "game/map/universe.hpp"
#include "game/map/ship.hpp"

game::map::AnyShipType::AnyShipType(Universe& univ)
    : ObjectVectorType<Ship>(univ, univ.ships())
{ }

bool
game::map::AnyShipType::isValid(const Ship& s) const
{
    // ex GAnyShipType::isValidIndex (sort-of)
    return s.isVisible();
}
