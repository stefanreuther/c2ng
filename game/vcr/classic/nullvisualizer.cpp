/**
  *  \file game/vcr/classic/nullvisualizer.cpp
  */

#include "game/vcr/classic/nullvisualizer.hpp"

game::vcr::classic::NullVisualizer::NullVisualizer()
{ }

game::vcr::classic::NullVisualizer::~NullVisualizer()
{ }

void
game::vcr::classic::NullVisualizer::startFighter(Side /*side*/, int /*track*/)
{ }

void
game::vcr::classic::NullVisualizer::landFighter(Side /*side*/, int /*track*/)
{ }

void
game::vcr::classic::NullVisualizer::killFighter(Side /*side*/, int /*track*/)
{ }

void
game::vcr::classic::NullVisualizer::fireBeam(Side /*side*/, int /*track*/, int /*target*/, int /*hit*/, int, int)
{ }

void
game::vcr::classic::NullVisualizer::fireTorpedo(Side /*side*/, int /*hit*/, int)
{ }

void
game::vcr::classic::NullVisualizer::updateBeam(Side /*side*/, int /*id*/)
{ }

void
game::vcr::classic::NullVisualizer::updateLauncher(Side /*side*/, int /*id*/)
{ }

void
game::vcr::classic::NullVisualizer::killObject(Side /*side*/)
{ }

// void
// game::vcr::classic::NullVisualizer::redraw()
// { }

// void
// game::vcr::classic::NullVisualizer::init()
// { }
