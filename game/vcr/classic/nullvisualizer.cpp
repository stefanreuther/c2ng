/**
  *  \file game/vcr/classic/nullvisualizer.cpp
  *  \brief Class game::vcr::classic::NullVisualizer
  */

#include "game/vcr/classic/nullvisualizer.hpp"

game::vcr::classic::NullVisualizer::NullVisualizer()
{ }

game::vcr::classic::NullVisualizer::~NullVisualizer()
{ }

void
game::vcr::classic::NullVisualizer::startFighter(Algorithm& /*algo*/, Side /*side*/, int /*track*/)
{ }

void
game::vcr::classic::NullVisualizer::landFighter(Algorithm& /*algo*/, Side /*side*/, int /*track*/)
{ }

void
game::vcr::classic::NullVisualizer::killFighter(Algorithm& /*algo*/, Side /*side*/, int /*track*/)
{ }

void
game::vcr::classic::NullVisualizer::fireBeam(Algorithm& /*algo*/, Side /*side*/, int /*track*/, int /*target*/, int /*hit*/, int, int)
{ }

void
game::vcr::classic::NullVisualizer::fireTorpedo(Algorithm& /*algo*/, Side /*side*/, int /*hit*/, int)
{ }

void
game::vcr::classic::NullVisualizer::updateBeam(Algorithm& /*algo*/, Side /*side*/, int /*id*/)
{ }

void
game::vcr::classic::NullVisualizer::updateLauncher(Algorithm& /*algo*/, Side /*side*/, int /*id*/)
{ }

void
game::vcr::classic::NullVisualizer::killObject(Algorithm& /*algo*/, Side /*side*/)
{ }
