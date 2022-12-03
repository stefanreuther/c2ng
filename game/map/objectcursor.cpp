/**
  *  \file game/map/objectcursor.cpp
  *  \brief Class game::map::ObjectCursor
  */

#include "game/map/objectcursor.hpp"
#include "game/map/objecttype.hpp"
#include "game/map/object.hpp"

// Get current object.
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

// Browse.
void
game::map::ObjectCursor::browse(Mode mode, bool marked)
{
    if (ObjectType* type = getObjectType()) {
        Id_t id = getCurrentIndex();
        switch (mode) {
         case Next:     id = type->findNextIndexWrap    (id, marked); break;
         case Previous: id = type->findPreviousIndexWrap(id, marked); break;
         case First:    id = type->findNextIndexWrap    (0,  marked); break;
         case Last:     id = type->findPreviousIndexWrap(0,  marked); break;
         case NextHere:
            if (const Object* obj = getCurrentObject()) {
                Point pt;
                if (obj->getPosition().get(pt)) {
                    id = type->findNextObjectAtWrap(pt, id, marked);
                }
            }
            break;
         case PreviousHere:
            if (const Object* obj = getCurrentObject()) {
                Point pt;
                if (obj->getPosition().get(pt)) {
                    id = type->findPreviousObjectAtWrap(pt, id, marked);
                }
            }
            break;
        }
        if (id != 0) {
            setCurrentIndex(id);
        }
    }
}
