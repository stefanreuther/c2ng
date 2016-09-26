/**
  *  \file game/map/objectobserver.cpp
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

// /** Get current object.
//     \return pointer to object, can be null
//     \see GObjectSelection::getCurrentObject() */
game::map::Object*
game::map::ObjectObserver::getCurrentObject()
{
    // ex GObjectSelectionObserver::getCurrentObject
    return m_cursor.getCurrentObject();
}

// /** Get universe for current object.
//     \return pointer to universe, can be null
//     \see GObjectSelection::getCurrentUniverse() */
game::map::Universe*
game::map::ObjectObserver::getCurrentUniverse()
{
    // ex GObjectSelectionObserver::getCurrentUniverse
    return m_cursor.getCurrentUniverse();
}

// /** Get object selection.
//     Returns a reference to the GObjectSelection this object was constructed with. */
game::map::ObjectCursor&
game::map::ObjectObserver::cursor()
{
    // ex GObjectSelectionObserver::getObjectSelection
    return m_cursor;
}

// /** Get object type observed by current selection.
//     \return pointer to GObjectSelection, can be null */
game::map::ObjectType*
game::map::ObjectObserver::getObjectType()
{
    // ex GObjectSelectionObserver::getObjectType
    return m_cursor.getObjectType();
}

// afl::base::Signal<void()> sig_objectChange;

// private:
// ObjectCursor& m_cursor;
// afl::base::SignalConnection conn_cursorChange;
// afl::base::SignalConnection conn_objectChange;

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
