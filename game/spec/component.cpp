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
      m_shortName(),
      m_description()
{ }

// Set name of this component.
void
game::spec::Component::setName(String_t name)
{
    m_name = name;
}

// Set short name of this component.
void
game::spec::Component::setShortName(String_t shortName)
{
    m_shortName = shortName;
}

// Set description/flavor text.
void
game::spec::Component::setDescription(String_t text)
{
    m_description = text;
}
