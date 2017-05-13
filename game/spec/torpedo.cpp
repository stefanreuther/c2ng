/**
  *  \file game/spec/torpedo.cpp
  *  \brief Class game::spec::Torpedo
  */

#include "game/spec/torpedo.hpp"
#include "game/spec/torpedolauncher.hpp"

game::spec::Torpedo::Torpedo(const TorpedoLauncher& launcher)
    : Weapon(launcher)
{
    cost() = launcher.torpedoCost();
    setMass(1);
}
