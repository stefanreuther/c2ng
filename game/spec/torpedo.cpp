/**
  *  \file game/spec/torpedo.cpp
  */

#include "game/spec/torpedo.hpp"
#include "game/spec/torpedolauncher.hpp"

game::spec::Torpedo::Torpedo(const TorpedoLauncher& launcher)
    : Weapon(launcher)
{
    cost() = launcher.torpedoCost();
    setMass(1);
}
