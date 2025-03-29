/**
  *  \file game/reference.cpp
  *  \brief Class game::Reference
  *
  *  This is partially but not completely modeled after PCC1's ThingId (things.pas).
  *  PCC2 used a completely type-agnostic GObjectReference, which in the end has a few disadvantages:
  *  it cannot be passed across turns or threads.
  */

#include "game/reference.hpp"
#include "afl/string/format.hpp"
#include "util/translation.hpp"

// Default constructor.
game::Reference::Reference()
    : m_type(Null),
      m_x(0),
      m_y(0)
{ }

// Construct from type and Id.
game::Reference::Reference(Type type, Id_t id)
    : m_type(type),
      m_x(id),
      m_y(0)
{ }

// Construct from position.
game::Reference::Reference(game::map::Point pt)
    : m_type(MapLocation),
      m_x(pt.getX()),
      m_y(pt.getY())
{ }

// Get position.
afl::base::Optional<game::map::Point>
game::Reference::getPosition() const
{
    if (m_type == MapLocation) {
        return game::map::Point(m_x, m_y);
    } else {
        return afl::base::Nothing;
    }
}

// Format to string.
String_t
game::Reference::toString(afl::string::Translator& tx) const
{
    const char* fmt = 0;
    switch (m_type) {
     case Null:                                       break;
     case Special:      /* do not name it! */         break;
     case Player:       fmt = N_("Player #%d");       break;
     case MapLocation:  fmt = N_("(%d,%d)");          break;
     case Ship:         fmt = N_("Ship #%d");         break;
     case Planet:       fmt = N_("Planet #%d");       break;
     case Starbase:     fmt = N_("Starbase #%d");     break;
     case IonStorm:     fmt = N_("Ion Storm #%d");    break;
     case Minefield:    fmt = N_("Minefield #%d");    break;
     case Ufo:          fmt = N_("Ufo #%d");          break;
     case Hull:         fmt = N_("Hull #%d");         break;
     case Engine:       fmt = N_("Engine #%d");       break;
     case Beam:         fmt = N_("Beam Weapon #%d");  break;
     case Torpedo:      fmt = N_("Torpedo Type #%d"); break;
    }
    return fmt
        ? String_t(afl::string::Format(tx.translateString(fmt).c_str(), m_x, m_y))
        : String_t();
}

// Implementation of makePrintable for testing.
String_t
game::Reference::makePrintable() const
{
    const char* fmt = "?";
    switch (m_type) {
     case Null:         fmt = "Null";             break;
     case Special:      fmt = "Special %d";       break;
     case Player:       fmt = "Player #%d";       break;
     case MapLocation:  fmt = "(%d,%d)";          break;
     case Ship:         fmt = "Ship #%d";         break;
     case Planet:       fmt = "Planet #%d";       break;
     case Starbase:     fmt = "Starbase #%d";     break;
     case IonStorm:     fmt = "Ion Storm #%d";    break;
     case Minefield:    fmt = "Minefield #%d";    break;
     case Ufo:          fmt = "Ufo #%d";          break;
     case Hull:         fmt = "Hull #%d";         break;
     case Engine:       fmt = "Engine #%d";       break;
     case Beam:         fmt = "Beam Weapon #%d";  break;
     case Torpedo:      fmt = "Torpedo Type #%d"; break;
    }
    return String_t(afl::string::Format(fmt, m_x, m_y));
}

// Compare equality.
bool
game::Reference::operator==(const Reference& other) const
{
    return (m_type == other.m_type
            && m_x == other.m_x
            && m_y == other.m_y);
}
