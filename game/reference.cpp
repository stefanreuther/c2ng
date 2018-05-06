/**
  *  \file game/reference.cpp
  *
  *  This is partially but not completely modeled after PCC1's ThingId (things.pas).
  *  PCC2 used a completely type-agnostic GObjectReference, which in the end has a few disadvantages:
  *  it cannot be passed across turns or threads.
  *
  *  FIXME: clarify the meaning of type Ufo. Right now, we pass an Id, which can be ambiguous with Hans' ufos.
  *  PCC 1.x passed an index.
  */

#include "game/reference.hpp"
#include "afl/string/format.hpp"
#include "util/translation.hpp"

game::Reference::Reference()
    : m_type(Null),
      m_x(0),
      m_y(0)
{ }

game::Reference::Reference(Type type, Id_t id)
    : m_type(type),
      m_x(id),
      m_y(0)
{ }

game::Reference::Reference(game::map::Point pt)
    : m_type(MapLocation),
      m_x(pt.getX()),
      m_y(pt.getY())
{ }

bool
game::Reference::isSet() const
{
    return m_type != Null;
}

game::Reference::Type
game::Reference::getType() const
{
    return m_type;
}

int
game::Reference::getId() const
{
    return m_x;
}

bool
game::Reference::getPos(game::map::Point& pt) const
{
    if (m_type == MapLocation) {
        pt = game::map::Point(m_x, m_y);
        return true;
    } else {
        return false;
    }
}

String_t
game::Reference::toString(afl::string::Translator& tx) const
{
    const char* fmt = "";
    switch (m_type) {
     case Null:                                       break;
     case Player:       fmt = N_("Player #%d");       break;
     case MapLocation:  fmt = N_("(%d,%d)");          break;
     case Ship:         fmt = N_("Ship #%d");         break;
     case Planet:       fmt = N_("Planet #%d");       break;
     case Starbase:     fmt = N_("Starbase #%d");     break;
     case Storm:        fmt = N_("Ion Storm #%d");    break;
     case Minefield:    fmt = N_("Minefield #%d");    break;
     case Ufo:          fmt = N_("Ufo #%d");          break;
     case Hull:         fmt = N_("Hull #%d");         break;
     case Engine:       fmt = N_("Engine #%d");       break;
     case Beam:         fmt = N_("Beam Weapon #%d");  break;
     case Torpedo:      fmt = N_("Torpedo Type #%d"); break;
    }
    return afl::string::Format(tx.translateString(fmt).c_str(), m_x, m_y);
}
