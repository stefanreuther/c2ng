/**
  *  \file game/map/ionstormtype.cpp
  */

#include "game/map/ionstormtype.hpp"
#include "game/map/universe.hpp"

game::map::IonStormType::IonStormType(Universe& univ)
    : ObjectVectorType<IonStorm>(univ.ionStorms())
{ }

bool
game::map::IonStormType::isValid(const IonStorm& s) const
{
    // ex GIonStormType::isValidIndex
    return s.isActive();
}
