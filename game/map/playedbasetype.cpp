/**
  *  \file game/map/playedbasetype.cpp
  *  \brief Class game::map::PlayedBaseType
  */

#include "game/map/playedbasetype.hpp"
#include "game/map/universe.hpp"

game::map::PlayedBaseType::PlayedBaseType(Universe& univ)
    : ObjectVectorType<Planet>(univ.planets())
{ }

bool
game::map::PlayedBaseType::isValid(const Planet& p) const
{
    // ex GPlayedBaseType::isValidIndex
    return p.isPlayable(Object::ReadOnly) && p.hasBase();
}
