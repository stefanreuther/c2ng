/**
  *  \file game/map/playedbasetype.cpp
  *  \brief Class game::map::PlayedBaseType
  */

#include "game/map/playedbasetype.hpp"

game::map::PlayedBaseType::PlayedBaseType(ObjectVector<Planet>& vec)
    : ObjectVectorType<Planet>(vec)
{ }

bool
game::map::PlayedBaseType::isValid(const Planet& p) const
{
    // ex GPlayedBaseType::isValidIndex
    return p.isPlayable(Object::ReadOnly) && p.hasBase();
}
