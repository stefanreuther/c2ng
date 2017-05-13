/**
  *  \file game/spec/component.cpp
  *  \brief Class game::spec::Component
  */

#include "game/spec/component.hpp"

// Constructor.
game::spec::Component::Component(ComponentNameProvider::Type type, int id)
    : m_type(type),
      m_id(id),
      m_mass(1),
      m_techLevel(1),
      m_cost(),
      m_name(),
      m_shortName()
{ }

// Get Id.
int
game::spec::Component::getId() const
{
    return m_id;
}

// Get mass of this component.
int
game::spec::Component::getMass() const
{
    return m_mass;
}

// Set mass of this component.
void
game::spec::Component::setMass(int mass)
{
    m_mass = mass;
}

// Get tech level of this component.
int
game::spec::Component::getTechLevel() const
{
    return m_techLevel;
}

// Set tech level of this component.
void
game::spec::Component::setTechLevel(int level)
{
    m_techLevel = level;
}

// Get cost of this component.
game::spec::Cost&
game::spec::Component::cost()
{
    return m_cost;
}

// Get cost of this component.
const game::spec::Cost&
game::spec::Component::cost() const
{
    return m_cost;
}

// Get name of this component.
String_t
game::spec::Component::getName(const ComponentNameProvider& provider) const
{
    return provider.getName(m_type, m_id, m_name);
}

// Set name of this component.
void
game::spec::Component::setName(String_t name)
{
    m_name = name;
}

// Get short name of this component.
String_t
game::spec::Component::getShortName(const ComponentNameProvider& provider) const
{
    return provider.getShortName(m_type, m_id, m_name, m_shortName);
}

// Set short name of this component.
void
game::spec::Component::setShortName(String_t shortName)
{
    m_shortName = shortName;
}
