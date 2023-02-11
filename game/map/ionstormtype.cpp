/**
  *  \file game/map/ionstormtype.cpp
  *  \brief Class game::map::IonStormType
  */

#include "game/map/ionstormtype.hpp"

game::map::IonStormType::IonStormType(ObjectVector<IonStorm>& vec)
    : ObjectVectorType<IonStorm>(vec)
{ }

bool
game::map::IonStormType::isValid(const IonStorm& s) const
{
    // ex GIonStormType::isValidIndex
    return s.isActive();
}
