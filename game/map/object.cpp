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

/** Set playability. */
void
game::map::Object::setPlayability(Playability p)
{
    m_playability = p;
}

/** Get playability. */
game::map::Object::Playability
game::map::Object::getPlayability() const
{
    return m_playability;
}

/** Mark object clean. */
void
game::map::Object::markClean()
{
    m_isDirty = false;
}

/** Mark object dirty. */
void
game::map::Object::markDirty()
{
    m_isDirty = true;
}

/** Check whether specific area of object is dirty. */
bool
game::map::Object::isDirty() const
{
    return m_isDirty;
}


/** Notify all listeners. If this object is dirty, raise sig_change
    and reset dirtiness state.

    You should not use this directly. Use GUniverse::doScreenUpdates()
    instead, which offers more flexibility for users to hook into
    universe change. In particular, widgets that hook into
    GUniverse::sig_preUpdate (those that observe lots of objects)
    will not be notified by a lone notifyListeners(). */
void
game::map::Object::notifyListeners()
{
    if (m_isDirty) {
        m_isDirty = false;
        sig_change.raise(getId());
    }
}


/** Check whether object is marked. */
bool
game::map::Object::isMarked() const
{
    return m_isMarked;
}

/** Set selection status. */
void
game::map::Object::setIsMarked(bool n)
{
    markDirty();
    m_isMarked = n;
}
