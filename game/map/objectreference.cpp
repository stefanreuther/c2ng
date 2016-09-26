/**
  *  \file game/map/objectreference.cpp
  */

#include "game/map/objectreference.hpp"
#include "game/map/objecttype.hpp"

game::map::ObjectReference::ObjectReference()
    : m_type(0),
      m_index(0)
{
    // ex GObjectReference::GObjectReference
}

game::map::ObjectReference::ObjectReference(ObjectType& type, Id_t index)
    : m_type(&type),
      m_index(index)
{
    // ex GObjectReference::GObjectReference
    // FIXME: add this from original code? If it is a link, dereference that, to avoid making a link-to-link
    // if (ObjectList* list = dynamic_cast<ObjectList*>(&type)) {
    //     *this = list->getObjectReferenceByIndex(index);
    // }
}

// Use default operator=, copy constructor

// bool         isValid() const;
// bool         isValidInType() const;
bool
game::map::ObjectReference::isValid() const
{
    // ex GObjectReference::isValidInType (there is no isValid)
    return get() != 0;
}

game::map::ObjectType*
game::map::ObjectReference::getObjectType() const
{
    // ex GObjectReference::getObjectType
    return m_type;
}

game::map::Universe*
game::map::ObjectReference::getUniverse() const
{
    // ex GObjectReference::getUniverse
    if (m_type != 0) {
        return m_type->getUniverseByIndex(m_index);
    } else {
        return 0;
    }
}

game::Id_t
game::map::ObjectReference::getObjectIndex() const
{
    // ex GObjectReference::getObjectIndex
    return m_index;
}

// FIXME: retire:
// inline bool
// GObjectReference::isGameObject() const
// {
//     return type != 0
//         && type->isValidIndex(index)
//         && dynamic_cast<GNoteContainer*>(type) == 0;
// }

game::map::Object*
game::map::ObjectReference::get() const
{
    // ex GObjectReference::operator->()
    if (m_type != 0) {
        return m_type->getObjectByIndex(m_index);
    } else {
        return 0;
    }
}

bool
game::map::ObjectReference::operator==(const ObjectReference& o) const
{
    // ex GObjectReference::operator==
    return m_type == o.m_type
        && m_index == o.m_index;
}

bool
game::map::ObjectReference::operator!=(const ObjectReference& o) const
{
    // ex GObjectReference::operator!=
    return m_type != o.m_type
        || m_index != o.m_index;
}
