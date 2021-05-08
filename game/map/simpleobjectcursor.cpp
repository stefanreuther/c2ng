/**
  *  \file game/map/simpleobjectcursor.cpp
  */

#include "game/map/simpleobjectcursor.hpp"
#include "game/map/objecttype.hpp"

game::map::SimpleObjectCursor::SimpleObjectCursor()
    : m_currentIndex(0),
      m_objectType(0),
      conn_setChange()
{
    // ex GSimpleObjectSelection::GSimpleObjectSelection
}

game::map::SimpleObjectCursor::SimpleObjectCursor(const ObjectCursor& other)
    : m_currentIndex(0),
      m_objectType(0),
      conn_setChange()
{
    // ex GSimpleObjectSelection::initFrom, sort-of
    // Copy type
    setObjectType(other.getObjectType());

    // Copy position
    if (Id_t index = other.getCurrentIndex()) {
        if (m_objectType != 0) {
            if (m_objectType->getObjectByIndex(index)) {
                setCurrentIndex(index);
            }
        }
    }
}

game::map::SimpleObjectCursor::~SimpleObjectCursor()
{
    // ex GSimpleObjectSelection::~GSimpleObjectSelection
}

game::map::ObjectType*
game::map::SimpleObjectCursor::getObjectType() const
{
    // ex GSimpleObjectSelection::getObjectType
    return m_objectType;
}

void
game::map::SimpleObjectCursor::setCurrentIndex(Id_t index)
{
    // ex GSimpleObjectSelection::setCurrentIndex
    if (index != m_currentIndex) {
        m_currentIndex = index;
        sig_indexChange.raise();
    }
}

game::Id_t
game::map::SimpleObjectCursor::getCurrentIndex() const
{
    // ex GSimpleObjectSelection::getCurrentIndex
    return m_currentIndex;
}

void
game::map::SimpleObjectCursor::setObjectType(ObjectType* type)
{
    // ex GSimpleObjectSelection::setObjectType
    if (type != m_objectType) {
        m_objectType = type;
        if (type != 0) {
            // We're selecting an object type
            conn_setChange = type->sig_setChange.add(this, &SimpleObjectCursor::onSetChange);

            // onSetChange happens to do whatever we want to do here
            onSetChange(0);
        } else {
            // We're selecting the null type
            conn_setChange.disconnect();
            m_currentIndex = 0;
            sig_indexChange.raise();
        }
    }
}

void
game::map::SimpleObjectCursor::onSetChange(Id_t hint)
{
    // ex GSimpleObjectSelection::onSetChanged
    if (m_objectType != 0) {
        if (m_objectType->getObjectByIndex(m_currentIndex) != 0) {
            // OK, keep this one; still valid
        } else if (m_objectType->getObjectByIndex(hint) != 0) {
            // OK, go to new position
            m_currentIndex = hint;
        } else {
            // Find something new
            m_currentIndex = m_objectType->findNextIndexWrap(m_currentIndex, false);
        }
        sig_indexChange.raise();
    }
}
