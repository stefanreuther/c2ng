/**
  *  \file game/spec/component.cpp
  */

#include "game/spec/component.hpp"

game::spec::Component::Component(ComponentNameProvider::Type type, int id)
    : m_type(type),
      m_id(id),
      m_mass(1),
      m_techLevel(1),
      m_cost(),
      m_name(),
      m_shortName()
{ }

int
game::spec::Component::getId() const
{
    return m_id;
}

int
game::spec::Component::getMass() const
{
    return m_mass;
}

void
game::spec::Component::setMass(int mass)
{
    m_mass = mass;
}

int
game::spec::Component::getTechLevel() const
{
    return m_techLevel;
}

void
game::spec::Component::setTechLevel(int level)
{
    m_techLevel = level;
}

game::spec::Cost&
game::spec::Component::cost()
{
    return m_cost;
}

const game::spec::Cost&
game::spec::Component::cost() const
{
    return m_cost;
}

String_t
game::spec::Component::getName(const ComponentNameProvider& provider) const
{
    return provider.getName(m_type, m_id, m_name);
}

void
game::spec::Component::setName(String_t name)
{
    m_name = name;
}

String_t
game::spec::Component::getShortName(const ComponentNameProvider& provider) const
{
    return provider.getShortName(m_type, m_id, m_name, m_shortName);
}

void
game::spec::Component::setShortName(String_t shortName)
{
    m_shortName = shortName;
}
