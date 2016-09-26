/**
  *  \file game/spec/beam.cpp
  */

#include "game/spec/beam.hpp"

game::spec::Beam::Beam(int id)
    : Weapon(ComponentNameProvider::Beam, id)
{ }
