/**
  *  \file game/map/playedplanettype.cpp
  *  \brief Class game::map::PlayedPlanetType
  */

#include "game/map/playedplanettype.hpp"
#include "game/map/universe.hpp"

game::map::PlayedPlanetType::PlayedPlanetType(Universe& univ)
    : ObjectVectorType<Planet>(univ.planets())
{ }

bool
game::map::PlayedPlanetType::isValid(const Planet& p) const
{
    // ex GPlayedPlanetType::isValidIndex
    return p.isPlayable(Object::ReadOnly);
}
