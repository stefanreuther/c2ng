/**
  *  \file game/vcr/flak/nullvisualizer.cpp
  *  \brief Class game::vcr::flak::NullVisualizer
  */

#include "game/vcr/flak/nullvisualizer.hpp"

void
game::vcr::flak::NullVisualizer::updateTime(int32_t /*time*/)
{ }

void
game::vcr::flak::NullVisualizer::fireBeamFighterFighter(Object_t /*from*/, Object_t /*to*/, bool /*hits*/)
{ }

void
game::vcr::flak::NullVisualizer::fireBeamFighterShip(Object_t /*from*/, Ship_t /*to*/, bool /*hits*/)
{ }

void
game::vcr::flak::NullVisualizer::fireBeamShipFighter(Ship_t /*from*/, int /*beamNr*/, Object_t /*to*/, bool /*hits*/)
{ }

void
game::vcr::flak::NullVisualizer::fireBeamShipShip(Ship_t /*from*/, int /*beamNr*/, Ship_t /*to*/, bool /*hits*/)
{ }

void
game::vcr::flak::NullVisualizer::createFighter(Object_t /*id*/, const Position& /*pos*/, int /*player*/, Ship_t /*enemy*/)
{ }

void
game::vcr::flak::NullVisualizer::killFighter(Object_t /*id*/)
{ }

void
game::vcr::flak::NullVisualizer::landFighter(Object_t /*id*/)
{ }

void
game::vcr::flak::NullVisualizer::moveFighter(Object_t /*id*/, const Position& /*pos*/, Ship_t /*to*/)
{ }

void
game::vcr::flak::NullVisualizer::createFleet(Fleet_t /*fleetNr*/, int32_t /*x*/, int32_t /*y*/, int /*player*/, Ship_t /*firstShip*/, size_t /*numShips*/)
{ }

void
game::vcr::flak::NullVisualizer::setEnemy(Fleet_t /*fleetNr*/, Ship_t /*enemy*/)
{ }

void
game::vcr::flak::NullVisualizer::killFleet(Fleet_t /*fleetNr*/)
{ }

void
game::vcr::flak::NullVisualizer::moveFleet(Fleet_t /*fleetNr*/, int32_t /*x*/, int32_t /*y*/)
{ }

void
game::vcr::flak::NullVisualizer::createShip(Ship_t /*shipNr*/, const Position& /*pos*/, const ShipInfo& /*info*/)
{ }

void
game::vcr::flak::NullVisualizer::killShip(Ship_t /*shipNr*/)
{ }

void
game::vcr::flak::NullVisualizer::moveShip(Ship_t /*shipNr*/, const Position& /*pos*/)
{ }

void
game::vcr::flak::NullVisualizer::createTorpedo(Object_t /*id*/, const Position& /*pos*/, int /*player*/, Ship_t /*enemy*/)
{ }

void
game::vcr::flak::NullVisualizer::hitTorpedo(Object_t /*id*/, Ship_t /*shipNr*/)
{ }

void
game::vcr::flak::NullVisualizer::missTorpedo(Object_t /*id*/)
{ }

void
game::vcr::flak::NullVisualizer::moveTorpedo(Object_t /*id*/, const Position& /*pos*/)
{ }

