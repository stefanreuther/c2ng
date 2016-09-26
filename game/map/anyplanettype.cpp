/**
  *  \file game/map/anyplanettype.cpp
  *  \brief Class game::map::AnyPlanetType
  */

#include "game/map/anyplanettype.hpp"
#include "game/map/universe.hpp"
#include "game/map/planet.hpp"

game::map::AnyPlanetType::AnyPlanetType(Universe& univ)
    : ObjectVectorType<Planet>(univ, univ.planets())
{ }

bool
game::map::AnyPlanetType::isValid(const Planet& p) const
{
    // ex GAnyPlanetType::isValidIndex
    return p.isVisible();
}
