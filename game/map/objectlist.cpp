/**
  *  \file game/map/objectlist.cpp
  */

#include "game/map/objectlist.hpp"

game::map::ObjectList::ObjectList()
    : m_list()
{ }

game::map::ObjectList::~ObjectList()
{ }

// /** Add a single object.
//     \param ref Reference to object
//     \pre ref.isValid() */
void
game::map::ObjectList::addObject(ObjectReference ref)
{
    // ex GObjectList::addObject
    m_list.push_back(ref);
}

// /** Add a single object.
//     \param type Type of object
//     \param index Index into type */
void
game::map::ObjectList::addObject(ObjectType& type, int index)
{
    // ex GObjectList::addObject
    m_list.push_back(ObjectReference(type, index));
}

void
game::map::ObjectList::clear()
{
    Container_t().swap(m_list);
    sig_setChange.raise(0);
}

// FIXME: do we need container access? This does no longer serve as the backend to GUI stuff,
// so ObjectType iteration is probably fine.
// // Container access: (0-based)
// GObjectList::iterator
// GObjectList::begin() const
// {
//     return list.begin();
// }

// GObjectList::iterator
// GObjectList::end() const
// {
//     return list.end();
// }

// GObjectList::size_type
// GObjectList::size() const
// {
//     return list.size();
// }

// const GObjectReference&
// GObjectList::operator[](size_type index) const
// {
//     return list[index];
// }

game::map::Object*
game::map::ObjectList::getObjectByIndex(Id_t index)
{
    // ex GObjectList::getObjectByIndex
    return getObjectReferenceByIndex(index).get();
}

game::map::Universe*
game::map::ObjectList::getUniverseByIndex(Id_t index)
{
    // ex GObjectList::getUniverseByIndex
    return getObjectReferenceByIndex(index).getUniverse();
}

game::Id_t
game::map::ObjectList::getNextIndex(Id_t index) const
{
    // ex GObjectList::getNextIndex
    if (index < Id_t(m_list.size())) {
        return index+1;
    } else {
        return 0;
    }
}

game::Id_t
game::map::ObjectList::getPreviousIndex(Id_t index) const
{
    // ex GObjectList::getPreviousIndex
    if (index > 0) {
        return index - 1;
    } else {
        return Id_t(m_list.size());
    }
}

game::map::ObjectReference
game::map::ObjectList::getObjectReferenceByIndex(Id_t index) const
{
    // ex GObjectList::getObjectReferenceByIndex
    size_t adjustedIndex = index-1;
    if (adjustedIndex < m_list.size()) {
        return m_list[adjustedIndex];
    } else {
        return ObjectReference();
    }
}

game::Id_t
game::map::ObjectList::getIndexFor(ObjectReference ref) const
{
    // ex GObjectList::getIndexFor, rewritten
    for (Id_t i = getNextIndex(0); i != 0; i = getNextIndex(i)) {
        if (getObjectReferenceByIndex(i) == ref) {
            return i;
        }
    }
    return 0;
}

game::Id_t
game::map::ObjectList::getIndexFor(const Object& obj) const
{
    // ex GObjectList::getIndexFor, rewritten
    // FIXME: move to ObjectType?
    for (Id_t i = getNextIndex(0); i != 0; i = getNextIndex(i)) {
        // manually "inline" getObjectByIndex because that is an interface method which isn't const
        if (getObjectReferenceByIndex(i).get() == &obj) {
            return i;
        }
    }
    return 0;
}

// FIXME: retire? GObjectList::countGameObjects
// GObjectList::size_type
// GObjectList::countGameObjects() const
// {
//     size_type count = 0;
//     for (size_type i = 0; i < list.size(); ++i) {
//         if (list[i].isGameObject()) {
//             ++count;
//         }
//     }
//     return count;
// }
