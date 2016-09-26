/**
  *  \file game/map/playedplanettype.cpp
  *  \brief Class game::map::PlayedPlanetType
  */

#include "game/map/playedplanettype.hpp"
#include "game/map/universe.hpp"

game::map::PlayedPlanetType::PlayedPlanetType(Universe& univ)
    : ObjectVectorType<Planet>(univ, univ.planets())
{ }

bool
game::map::PlayedPlanetType::isValid(const Planet& p) const
{
    // ex GPlayedPlanetType::isValidIndex
    return p.isPlayable(Object::ReadOnly);
}


void foo(game::map::Universe& u)
{
    game::map::PlayedPlanetType t(u);
}
