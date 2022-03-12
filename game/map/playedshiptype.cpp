/**
  *  \file game/map/playedshiptype.cpp
  *  \brief Class game::map::PlayedShipType
  */

#include "game/map/playedshiptype.hpp"
#include "game/map/ship.hpp"

game::map::PlayedShipType::PlayedShipType(ObjectVector<Ship>& vec)
    : ObjectVectorType<Ship>(vec)
{ }

bool
game::map::PlayedShipType::isValid(const Ship& s) const
{
    // ex GPlayedShipType::isValidIndex (sort-of)
    return s.isPlayable(Ship::ReadOnly);
}

int
game::map::PlayedShipType::countCapitalShips() const
{
    int n = 0;
    for (Id_t id = findNextIndex(0); id != 0; id = findNextIndex(id)) {
        if (const Ship* sh = const_cast<PlayedShipType*>(this)->getObjectByIndex(id)) {
            if (sh->hasWeapons()) {
                ++n;
            }
        }
    }
    return n;
}
