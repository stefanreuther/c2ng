/**
  *  \file game/player.cpp
  *  \brief Class game::Player
  */

#include "game/player.hpp"
#include "util/translation.hpp"
#include "afl/string/format.hpp"

// Constructor.
game::Player::Player(int id)
    : m_id(id),
      m_isReal(true),
      m_changed(false),
      m_kind(Normal),
      m_names()
{ }

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
String_t
game::Player::getName(Name which, afl::string::Translator& tx) const
{
    String_t result = m_names[which];
    if (result.empty()) {
        result = getDefaultName(m_id, which, m_kind, tx);
    }
    return result;
}

// Initialize for standard "unowned" slot.
void
game::Player::initUnowned()
{
    m_isReal = false;
    m_kind = Unowned;
    clearNames();
}

// Initialize for standard "aliens" slot.
void
game::Player::initAlien()
{
    m_isReal = false;
    m_kind = Alien;
    clearNames();
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

String_t
game::Player::getDefaultName(int playerNr, Name which, afl::string::Translator& tx)
{
    return getDefaultName(playerNr, which, Normal, tx);
}

void
game::Player::clearNames()
{
    m_names[ShortName].clear();
    m_names[AdjectiveName].clear();
    m_names[LongName].clear();
    m_names[OriginalShortName].clear();
    m_names[OriginalAdjectiveName].clear();
    m_names[OriginalLongName].clear();
    m_changed = true;

}

const char*
game::Player::pickTemplate(Kind k, const char* normal, const char* alien, const char* unowned)
{
    switch (k) {
     case Normal:  return normal;
     case Alien:   return alien;
     case Unowned: return unowned;
    }
    return normal;
}

String_t
game::Player::getDefaultName(int playerNr, Name which, Kind k, afl::string::Translator& tx)
{
    const char* tpl = 0;
    switch (which) {
     case Player::ShortName:
     case Player::OriginalShortName:
        tpl = pickTemplate(k, N_("Player %d"), N_("Alien Marauders"), N_("Nobody"));
        break;

     case Player::AdjectiveName:
     case Player::OriginalAdjectiveName:
        tpl = pickTemplate(k, N_("Player %d"), N_("Alien"), N_("unowned"));
        break;

     case Player::LongName:
     case Player::OriginalLongName:
        tpl = pickTemplate(k, N_("Player %d"), N_("The Alien Marauder Alliance"), N_("Nobody"));
        break;

     case Player::UserName:
     case Player::NickName:
     case Player::EmailAddress:
        break;
    }
    if (tpl != 0) {
        return afl::string::Format(tx(tpl), playerNr);
    } else {
        return String_t();
    }
}
