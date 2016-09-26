/**
  *  \file game/player.cpp
  *  \brief Class game::Player
  */

#include "game/player.hpp"
#include "util/translation.hpp"

// Constructor.
game::Player::Player(int id)
    : m_id(id),
      m_isReal(true),
      m_changed(false),
      m_names()
{ }

// Get player number.
int
game::Player::getId() const
{
    return m_id;
}

// Set player status.
void
game::Player::setIsReal(bool flag)
{
    m_isReal = flag;
    m_changed = true;
}

// Get player status.
bool
game::Player::isReal() const
{
    return m_isReal;
}

// Set name.
void
game::Player::setName(Name which, String_t name)
{
    m_names[which] = name;
    m_changed = true;
}

// Set original names.
void
game::Player::setOriginalNames()
{
    m_names[OriginalShortName]     = m_names[ShortName];
    m_names[OriginalAdjectiveName] = m_names[AdjectiveName];
    m_names[OriginalLongName]      = m_names[LongName];
    m_changed = true;
}

// Get name.
const String_t&
game::Player::getName(Name which) const
{
    return m_names[which];
}

// Initialize for standard "unowned" slot.
void
game::Player::initUnowned()
{
    m_isReal = false;
    m_names[ShortName]     = m_names[OriginalShortName]     = _("Nobody");
    m_names[AdjectiveName] = m_names[OriginalAdjectiveName] = _("unowned");
    m_names[LongName]      = m_names[OriginalLongName]      = _("Nobody");
    m_changed = true;
}

// Initialize for standard "aliens" slot.
void
game::Player::initAlien()
{
    m_isReal = false;
    m_names[ShortName]     = m_names[OriginalShortName]     = _("Alien Marauders");
    m_names[AdjectiveName] = m_names[OriginalAdjectiveName] = _("Alien");
    m_names[LongName]      = m_names[OriginalLongName]      = _("The Alien Marauder Alliance");
    m_changed = true;
}

// Mark this player changed.
void
game::Player::markChanged(bool state)
{
    m_changed = state;
}

// Check whether this player was changed.
bool
game::Player::isChanged() const
{
    return m_changed;
}
