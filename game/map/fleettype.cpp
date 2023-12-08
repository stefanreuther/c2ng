/**
  *  \file game/map/fleettype.cpp
  *  \brief Class game::map::FleetType
  */

#include "game/map/fleettype.hpp"
#include "game/map/universe.hpp"

game::map::FleetType::FleetType(ObjectVector<Ship>& vec)
    : ObjectVectorType<Ship>(vec)
{ }

bool
game::map::FleetType::isValid(const Ship& s) const
{
    // ex GFleetType::isValidIndex
    return s.isPlayable(Object::Playable) && s.isFleetLeader();
}

void
game::map::FleetType::handleFleetChange(Id_t hint)
{
    // ex GFleetType::handleFleetChange
    sig_setChange.raise(hint);
}
