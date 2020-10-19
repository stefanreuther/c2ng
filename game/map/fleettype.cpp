/**
  *  \file game/map/fleettype.cpp
  */

#include "game/map/fleettype.hpp"
#include "game/map/universe.hpp"

game::map::FleetType::FleetType(Universe& univ)
    : ObjectVectorType<Ship>(univ.ships())
{ }

bool
game::map::FleetType::isValid(const Ship& p) const
{
    // ex GFleetType::isValidIndex
    return p.isPlayable(Object::Playable) && p.isFleetLeader();
}

void
game::map::FleetType::handleFleetChange(Id_t hint)
{
    // ex GFleetType::handleFleetChange
    sig_setChange.raise(hint);
}
