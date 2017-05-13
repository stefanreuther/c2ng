/**
  *  \file game/spec/beam.cpp
  *  \brief Class game::spec::Beam
  */

#include "game/spec/beam.hpp"

// Constructor.
game::spec::Beam::Beam(int id)
    : Weapon(ComponentNameProvider::Beam, id)
{ }
