/**
  *  \file game/map/object.cpp
  *  \brief Base class game::map::Object
  */

#include "game/map/object.hpp"

game::map::Object::Object(Id_t id)
    : m_playability(NotPlayable),
      m_isMarked(false),
      m_isDirty(false),
      m_id(id)
{ }

game::map::Object::Object(const Object& other)
    : m_playability(other.m_playability),
      m_isMarked(other.m_isMarked),
      m_isDirty(false),
      m_id(other.m_id)
{ }

// Destructor.
game::map::Object::~Object()
{ }

void
game::map::Object::notifyListeners()
{
    if (m_isDirty) {
        m_isDirty = false;
        sig_change.raise(getId());
    }
}

void
game::map::Object::setId(Id_t id)
{
    m_id = id;
}
