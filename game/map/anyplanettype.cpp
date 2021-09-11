/**
  *  \file game/map/anyplanettype.cpp
  *  \brief Class game::map::AnyPlanetType
  */

#include "game/map/anyplanettype.hpp"

game::map::AnyPlanetType::AnyPlanetType(ObjectVector<Planet>& vec)
    : ObjectVectorType<Planet>(vec)
{ }

bool
game::map::AnyPlanetType::isValid(const Planet& p) const
{
    // ex GAnyPlanetType::isValidIndex
    return p.isVisible();
}
