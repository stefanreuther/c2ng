/**
  *  \file game/map/objectcursor.cpp
  */

#include "game/map/objectcursor.hpp"
#include "game/map/objecttype.hpp"

// /** Get currently selected object.
//     \return pointer to object if any selected; null if none */
game::map::Object*
game::map::ObjectCursor::getCurrentObject() const
{
    // ex GObjectSelection::getCurrentObject
    if (ObjectType* type = getObjectType()) {
        return type->getObjectByIndex(getCurrentIndex());
    } else {
        return 0;
    }
}

// /** Get currently selected universe. This is the universe the selected
//     object is in.
//     \return pointer to universe if any object selected; null if none
//     (note that the result can also be null for objects that are not in
//     any universe, notably list dividers). */
game::map::Universe*
game::map::ObjectCursor::getCurrentUniverse() const
{
    // ex GObjectSelection::getCurrentUniverse
    if (ObjectType* type = getObjectType()) {
        return type->getUniverseByIndex(getCurrentIndex());
    } else {
        return 0;
    }
}
