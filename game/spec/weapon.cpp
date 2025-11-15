/**
  *  \file game/spec/weapon.cpp
  *  \brief Class game::spec::Weapon
  */

#include "game/spec/weapon.hpp"

// Constructor.
game::spec::Weapon::Weapon(ComponentNameProvider::Type type, int id)
    : Component(type, id),
      m_killPower(0),
      m_damagePower(0)
{ }

// Check for death ray.
bool
game::spec::Weapon::isDeathRay(const HostVersion& host) const
{
    return host.hasDeathRays()
        && getDamagePower() == 0;
}
