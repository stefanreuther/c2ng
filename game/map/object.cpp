/**
  *  \file game/map/object.cpp
  */

#include "game/map/object.hpp"


game::map::Object::Object()
    : m_playability(NotPlayable),
      m_isMarked(false),
      m_isDirty(false)
{ }

game::map::Object::Object(const Object& other)
    : m_playability(other.m_playability),
      m_isMarked(other.m_isMarked),
      m_isDirty(false)
{ }

// Destructor.
game::map::Object::~Object()
{ }

bool
game::map::Object::isPlayable(Playability p) const
{
    return m_playability >= p;
}

void
game::map::Object::setPlayability(Playability p)
{
    m_playability = p;
}

game::map::Object::Playability
game::map::Object::getPlayability() const
{
    return m_playability;
}

void
game::map::Object::markClean()
{
    m_isDirty = false;
}

void
game::map::Object::markDirty()
{
    m_isDirty = true;
}

bool
game::map::Object::isDirty() const
{
    return m_isDirty;
}


void
game::map::Object::notifyListeners()
{
    if (m_isDirty) {
        m_isDirty = false;
        sig_change.raise(getId());
    }
}

bool
game::map::Object::isMarked() const
{
    return m_isMarked;
}

void
game::map::Object::setIsMarked(bool n)
{
    markDirty();
    m_isMarked = n;
}
