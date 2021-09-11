/**
  *  \file game/map/playedplanettype.cpp
  *  \brief Class game::map::PlayedPlanetType
  */

#include "game/map/playedplanettype.hpp"

game::map::PlayedPlanetType::PlayedPlanetType(ObjectVector<Planet>& vec)
    : ObjectVectorType<Planet>(vec)
{ }

bool
game::map::PlayedPlanetType::isValid(const Planet& p) const
{
    // ex GPlayedPlanetType::isValidIndex
    return p.isPlayable(Object::ReadOnly);
}
