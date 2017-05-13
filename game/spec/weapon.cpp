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

// Get kill power.
int
game::spec::Weapon::getKillPower() const
{
    return m_killPower;
}

// Set kill power.
void
game::spec::Weapon::setKillPower(int killPower)
{
    m_killPower = killPower;
}

// Get damage power.
int
game::spec::Weapon::getDamagePower() const
{
    return m_damagePower;
}

// Set damage power.
void
game::spec::Weapon::setDamagePower(int damagePower)
{
    m_damagePower = damagePower;
}

