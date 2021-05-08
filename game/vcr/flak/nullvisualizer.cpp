/**
  *  \file game/vcr/flak/nullvisualizer.cpp
  */

#include "game/vcr/flak/nullvisualizer.hpp"

void
game::vcr::flak::NullVisualizer::createObject(const Algorithm::Object& /*obj*/, ship_t /*source*/)
{ }

void
game::vcr::flak::NullVisualizer::destroyObject(const Algorithm::Object& /*obj*/, bool /*violent*/)
{ }

void
game::vcr::flak::NullVisualizer::destroyShip(ship_t /*unit*/)
{ }

void
game::vcr::flak::NullVisualizer::fireBeam(const Algorithm::Object& /*obj1*/, const Algorithm::Object& /*obj2*/)
{ }

void
game::vcr::flak::NullVisualizer::fireBeam(const Algorithm::Object& /*obj1*/, ship_t /*unit2*/)
{ }

void
game::vcr::flak::NullVisualizer::fireBeam(ship_t /*unit1*/, const Algorithm::Object& /*obj2*/, bool /*hits*/)
{ }

void
game::vcr::flak::NullVisualizer::fireBeam(ship_t /*unit1*/, ship_t /*unit2*/, bool /*hits*/)
{ }

void
game::vcr::flak::NullVisualizer::hitShip(ship_t /*unit*/, int /*expl*/, int /*kill*/, int /*death_flag*/)
{ }
