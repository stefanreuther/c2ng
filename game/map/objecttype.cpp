/**
  *  \file game/map/objecttype.cpp
  *  \brief Base class game::map::ObjectType
  */

#include "game/map/objecttype.hpp"
#include "game/map/mapobject.hpp"
#include "game/map/circularobject.hpp"
#include "game/map/configuration.hpp"

game::map::ObjectType::~ObjectType()
{ }

// Find next object after index.
game::Id_t
game::map::ObjectType::findNextIndex(Id_t index) const
{
    // ex GObjectType::findNextIndex
    // In PCC2, this used isValidIndex(). c2ng no longer has that, requiring an ugly const_cast here.
    do {
        index = getNextIndex(index);
    } while (index != 0 && const_cast<ObjectType*>(this)->getObjectByIndex(index) == 0);
    return index;
}

// Check emptiness.
bool
game::map::ObjectType::isEmpty() const
{
    // ex GObjectType::isEmpty
    return findNextIndex(0) == 0;
}

// Check unit type.
bool
game::map::ObjectType::isUnit() const
{
    // ex GObjectType::isUnit
    Id_t n = findNextIndex(0);
    return n != 0 && findNextIndex(n) == 0;
}

// Count objects.
int
game::map::ObjectType::countObjects() const
{
    // ex GObjectType::countObjects
    int result = 0;
    for (Id_t i = findNextIndex(0); i != 0; i = findNextIndex(i)) {
        ++result;
    }
    return result;
}

// Count objects at position.
int
game::map::ObjectType::countObjectsAt(const Point pt, PlayerSet_t owners)
{
    // ex GUniverse::countShips
    int result = 0;
    for (Id_t i = findNextIndex(0); i != 0; i = findNextIndex(i)) {
        if (MapObject* mo = dynamic_cast<MapObject*>(getObjectByIndex(i))) {
            Point center;
            int owner;
            if (mo->getPosition(center) && mo->getOwner(owner) && center == pt && owners.contains(owner)) {
                ++result;
            }
        }
    }
    return result;
}

// Find nearest object.
game::Id_t
game::map::ObjectType::findNearestIndex(const Point pt, const Configuration& config)
{
    // ex GObjectType::findNearestIndex
    Id_t found = 0;
    int32_t dist2 = 0;
    bool inside = false;

    for (Id_t i = findNextIndex(0); i != 0; i = findNextIndex(i)) {
        if (MapObject* mo = dynamic_cast<MapObject*>(getObjectByIndex(i))) {
            Point center;
            if (mo->getPosition(center)) {
                // Position is known
                int32_t ndist2 = config.getSquaredDistance(pt, center);

                // If it is a circular object, check whether we're inside
                bool ninside;
                if (CircularObject* co = dynamic_cast<CircularObject*>(mo)) {
                    int32_t radiusSquared;
                    ninside = (co->getRadiusSquared(radiusSquared) && radiusSquared <= ndist2);
                } else{
                    ninside = false;
                }
                
                // Pick this object, if
                // - it's the first one
                // - it's closer than our previous choice
                // - we're inside it but not inside the previous one
                if (found == 0
                    || (inside == ninside && ndist2 < dist2)
                    || (ninside && !inside))
                {
                    found  = i;
                    dist2  = ndist2;
                    inside = ninside;
                }
            }
        }
    }
    return found;
}

// Get previous object before index, with wrap.
game::Id_t
game::map::ObjectType::findPreviousIndexWrap(Id_t index, bool marked)
{
    // ex GObjectType::findPreviousIndexWrap
    Id_t n = findPreviousIndexNoWrap(index, marked);
    if (n == 0) {
        n = findPreviousIndexNoWrap(0, marked);
    }
    return n;
}

// Get next object after index, with wrap.
game::Id_t
game::map::ObjectType::findNextIndexWrap(Id_t index, bool marked)
{
    // ex GObjectType::findNextIndexWrap
    Id_t n = findNextIndexNoWrap(index, marked);
    if (n == 0) {
        n = findNextIndexNoWrap(0, marked);
    }
    return n;
}

// Get previous object before index.
game::Id_t
game::map::ObjectType::findPreviousIndexNoWrap(Id_t index, bool marked)
{
    // ex GObjectType::findPreviousIndexNoWrap
    while (1) {
        index = getPreviousIndex(index);
        if (index == 0) {
            return 0;
        }
        if (Object* obj = getObjectByIndex(index)) {
            if (!marked || obj->isMarked()) {
                return index;
            }
        }
    }
}

// Get next object after index.
game::Id_t
game::map::ObjectType::findNextIndexNoWrap(Id_t index, bool marked)
{
    // ex GObjectType::findNextIndexNoWrap
    while (1) {
        index = getNextIndex(index);
        if (index == 0) {
            return 0;
        }
        if (Object* obj = getObjectByIndex(index)) {
            if (!marked || obj->isMarked()) {
                return index;
            }
        }
    }
}

game::Id_t
game::map::ObjectType::findFirstObjectAt(Point pt)
{
    return findNextObjectAt(pt, 0);
}

game::Id_t
game::map::ObjectType::findNextObjectAt(Point pt, int id)
{
    for (Id_t i = findNextIndex(id); i != 0; i = findNextIndex(i)) {
        if (MapObject* mo = dynamic_cast<MapObject*>(getObjectByIndex(i))) {
            Point center;
            if (mo->getPosition(center)) {
                if (center == pt) {
                    return i;
                }
            }
        }
    }
    return 0;
}

game::Id_t
game::map::ObjectType::findIndexForId(Id_t id)
{
    for (Id_t i = findNextIndex(0); i != 0; i = findNextIndex(i)) {
        if (Object* obj = getObjectByIndex(i)) {
            if (obj->getId() == id) {
                return i;
            }
        }
    }
    return 0;
}

// FIXME: do we need this guy? findIndex
// /** Given an object, find its index.
//     \param type Object type
//     \param obj Object to find
//     \return index if found, otherwise 0 */
// static int
// findIndex(GObjectType& type, GObject* obj)
// {
//     for (int i = type.findNextIndex(0); i != 0; i = type.findNextIndex(i)) {
//         if (type.isValidIndex(i) && &type.getObjectByIndex(i) == obj) {
//             return i;
//         }
//     }
//     return 0;
// }

bool
game::map::ObjectType::notifyObjectListeners()
{
    bool changed = false;
    for (Id_t i = findNextIndex(0); i != 0; i = findNextIndex(i)) {
        if (Object* obj = getObjectByIndex(i)) {
            if (obj->isDirty()) {
                obj->notifyListeners();
                changed = true;
            }
        }
    }
    return changed;
}
