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
