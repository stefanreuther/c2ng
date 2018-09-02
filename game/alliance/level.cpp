/**
  *  \file game/alliance/level.cpp
  *  \brief Class game::alliance::Level
  */

#include "game/alliance/level.hpp"

// Constructor.
game::alliance::Level::Level(const String_t& name, const String_t& id, Flags_t flags)
    : m_name(name),
      m_id(id),
      m_flags(flags)
{
    // ex GAllianceLevel::GAllianceLevel
}

// Destructor.
game::alliance::Level::~Level()
{ }

// Get human-friendly name.
const String_t&
game::alliance::Level::getName() const
{
    // ex GAllianceLevel::getName
    return m_name;
}

// Get internal identifier.
const String_t&
game::alliance::Level::getId() const
{
    // ex GAllianceLevel::getId
    return m_id;
}

// Get flags.
game::alliance::Level::Flags_t
game::alliance::Level::getFlags() const
{
    // ex GAllianceLevel::getFlags
    return m_flags;
}

// Check for flag.
bool
game::alliance::Level::hasFlag(Flag fl) const
{
    return m_flags.contains(fl);
}
