/**
  *  \file game/map/objectobserver.cpp
  *  \brief Class game::map::ObjectObserver
  */

#include "game/map/objectobserver.hpp"
#include "game/map/objectcursor.hpp"
#include "game/map/object.hpp"

game::map::ObjectObserver::ObjectObserver(ObjectCursor& cursor)
    : m_cursor(cursor),
      conn_cursorChange(),
      conn_objectChange()
{
    attachCursor();
    attachObject();
}

game::map::ObjectObserver::~ObjectObserver()
{ }

game::map::Object*
game::map::ObjectObserver::getCurrentObject() const
{
    // ex GObjectSelectionObserver::getCurrentObject
    return m_cursor.getCurrentObject();
}

game::map::ObjectCursor&
game::map::ObjectObserver::cursor() const
{
    // ex GObjectSelectionObserver::getObjectSelection
    return m_cursor;
}

game::map::ObjectType*
game::map::ObjectObserver::getObjectType() const
{
    // ex GObjectSelectionObserver::getObjectType
    return m_cursor.getObjectType();
}

void
game::map::ObjectObserver::onCurrentChange()
{
    // ex GObjectSelectionObserver::onCurrentChanged
    attachObject();
    sig_objectChange.raise();
}

void
game::map::ObjectObserver::attachObject()
{
    // ex GObjectSelectionObserver::attachObject
    if (Object* obj = getCurrentObject()) {
        conn_objectChange = obj->sig_change.add(&sig_objectChange, &afl::base::Signal<void()>::raise);
    } else {
        conn_objectChange.disconnect();
    }
}

void
game::map::ObjectObserver::attachCursor()
{
    // ex GObjectSelectionObserver::attachSelection
    conn_cursorChange = m_cursor.sig_indexChange.add(this, &ObjectObserver::onCurrentChange);
}
