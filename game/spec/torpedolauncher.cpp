/**
  *  \file game/spec/torpedolauncher.cpp
  *  \brief Class game::spec::TorpedoLauncher
  */

#include "game/spec/torpedolauncher.hpp"

game::spec::TorpedoLauncher::TorpedoLauncher(int id)
    : Weapon(ComponentNameProvider::Torpedo, id),
      m_torpedoCost()
{ }

game::spec::Cost&
game::spec::TorpedoLauncher::torpedoCost()
{
    return m_torpedoCost;
}

const game::spec::Cost&
game::spec::TorpedoLauncher::torpedoCost() const
{
    return m_torpedoCost;
}
