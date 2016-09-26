/**
  *  \file game/spec/weapon.cpp
  */

#include "game/spec/weapon.hpp"

game::spec::Weapon::Weapon(ComponentNameProvider::Type type, int id)
    : Component(type, id),
      m_killPower(0),
      m_damagePower(0)
{ }

int
game::spec::Weapon::getKillPower() const
{
    return m_killPower;
}

void
game::spec::Weapon::setKillPower(int killPower)
{
    m_killPower = killPower;
}

int
game::spec::Weapon::getDamagePower() const
{
    return m_damagePower;
}

void
game::spec::Weapon::setDamagePower(int damagePower)
{
    m_damagePower = damagePower;
}

