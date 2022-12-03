/**
  *  \file game/map/objecttype.cpp
  *  \brief Base class game::map::ObjectType
  */

#include "game/map/objecttype.hpp"
#include "game/map/object.hpp"
#include "game/map/circularobject.hpp"
#include "game/map/configuration.hpp"

/*
 *  Local Classes
 */

namespace game { namespace map { namespace {

    /*
     *  Filter base class, to inherit implementation of getNextIndex/getPreviousIndex.
     *  Implementation out-of-line, otherwise gcc pessimizes the generated code by inlining them.
     */
    class Filter : public ObjectType {
     public:
        Filter(ObjectType& parent)
            : m_parent(parent)
            { }
        virtual Id_t getNextIndex(Id_t index) const;
        virtual Id_t getPreviousIndex(Id_t index) const;
     protected:
        ObjectType& m_parent;
    };

    Id_t Filter::getNextIndex(Id_t index) const
    {
        return m_parent.getNextIndex(index);
    }

    Id_t Filter::getPreviousIndex(Id_t index) const
    {
        return m_parent.getPreviousIndex(index);
    }

    /*
     *  Filter by Position
     */
    class ByPosition : public Filter {
     public:
        ByPosition(ObjectType& parent, Point pos)
            : Filter(parent), m_position(pos)
            { }
        virtual Object* getObjectByIndex(Id_t index)
            {
                Object* p = m_parent.getObjectByIndex(index);
                Point center;
                if (p != 0 && p->getPosition().get(center) && center == m_position) {
                    return p;
                } else {
                    return 0;
                }
            }
     private:
        const Point m_position;
    };

    /*
     *  Filter by Owner
     */
    class ByOwner : public Filter {
     public:
        ByOwner(ObjectType& parent, PlayerSet_t owners)
            : Filter(parent), m_owners(owners)
            { }
        virtual Object* getObjectByIndex(Id_t index)
            {
                Object* p = m_parent.getObjectByIndex(index);
                int owner;
                if (p != 0 && p->getOwner().get(owner) && m_owners.contains(owner)) {
                    return p;
                } else {
                    return 0;
                }
            }
     private:
        const PlayerSet_t m_owners;
    };

    /*
     *  Filter by Marked status
     */
    class ByMarked : public Filter {
     public:
        ByMarked(ObjectType& parent, bool marked)
            : Filter(parent), m_marked(marked)
            { }
        virtual Object* getObjectByIndex(Id_t index)
            {
                Object* p = m_parent.getObjectByIndex(index);
                if (p != 0 && (!m_marked || p->isMarked())) {
                    return p;
                } else {
                    return 0;
                }
            }
     private:
        const bool m_marked;
    };


    /*
     *  Sorting
     */
    int isBefore(const game::ref::SortPredicate& pred,
                 const game::Reference& aref, int aidx,
                 const game::Reference& bref, int bidx)
    {
        if (int a = pred.compare(aref, bref)) {
            return a < 0;
        }
        if (aref.getId() != bref.getId()) {
            return aref.getId() < bref.getId();
        }
        return aidx < bidx;
    }

    class Sort : public ObjectType {
     public:
        Sort(ObjectType& parent, const game::ref::SortPredicate& pred, Reference::Type type)
            : m_parent(parent), m_predicate(pred), m_type(type)
            { }

        virtual Id_t getNextIndex(Id_t index) const;
        virtual Id_t getPreviousIndex(Id_t index) const;
        virtual Object* getObjectByIndex(Id_t index);
     private:
        ObjectType& m_parent;
        const game::ref::SortPredicate& m_predicate;
        const Reference::Type m_type;
    };

    Id_t Sort::getNextIndex(Id_t index) const
    {
        const Object* currentObj = m_parent.getObjectByIndex(index);
        const Object* foundObj = 0;
        int foundIndex = 0;
        for (Id_t i = m_parent.findNextIndex(0); i != 0; i = m_parent.findNextIndex(i)) {
            if (i != index) {
                if (Object* obj = m_parent.getObjectByIndex(i)) {
                    if (currentObj == 0
                        || isBefore(m_predicate, Reference(m_type, currentObj->getId()), index, Reference(m_type, obj->getId()), i))
                    {
                        if (foundObj == 0
                            || isBefore(m_predicate, Reference(m_type, obj->getId()), i, Reference(m_type, foundObj->getId()), foundIndex))
                        {
                            foundObj = obj;
                            foundIndex = i;
                        }
                    }
                }
            }
        }
        return foundIndex;
    }

    Id_t Sort::getPreviousIndex(Id_t index) const
    {
        const Object* currentObj = m_parent.getObjectByIndex(index);
        const Object* foundObj = 0;
        int foundIndex = 0;
        for (Id_t i = m_parent.findNextIndex(0); i != 0; i = m_parent.findNextIndex(i)) {
            if (i != index) {
                if (Object* obj = m_parent.getObjectByIndex(i)) {
                    if (currentObj == 0
                        || !isBefore(m_predicate, Reference(m_type, currentObj->getId()), index, Reference(m_type, obj->getId()), i))
                    {
                        if (foundObj == 0
                            || !isBefore(m_predicate, Reference(m_type, obj->getId()), i, Reference(m_type, foundObj->getId()), foundIndex))
                        {
                            foundObj = obj;
                            foundIndex = i;
                        }
                    }
                }
            }
        }
        return foundIndex;
    }

    Object* Sort::getObjectByIndex(Id_t index)
    {
        return m_parent.getObjectByIndex(index);
    }

} } }


/*
 *  ObjectType
 */

game::map::ObjectType::~ObjectType()
{ }

// Filter by position.
game::map::ObjectType&
game::map::ObjectType::filterPosition(afl::base::Deleter& del, Point pt)
{
    return del.addNew(new ByPosition(*this, pt));
}

// Filter by owner.
game::map::ObjectType&
game::map::ObjectType::filterOwner(afl::base::Deleter& del, PlayerSet_t owners)
{
    return del.addNew(new ByOwner(*this, owners));
}

// Filter by marked status.
game::map::ObjectType&
game::map::ObjectType::filterMarked(afl::base::Deleter& del, bool marked)
{
    return del.addNew(new ByMarked(*this, marked));
}

// Sort.
game::map::ObjectType&
game::map::ObjectType::sort(afl::base::Deleter& del, const game::ref::SortPredicate& pred, Reference::Type type)
{
    return del.addNew(new Sort(*this, pred, type));
}

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
    ByOwner f1(*this, owners);
    ByPosition f2(f1, pt);
    return f2.countObjects();
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
        if (const Object* mo = getObjectByIndex(i)) {
            Point center;
            if (mo->getPosition().get(center)) {
                // Position is known
                int32_t ndist2 = config.getSquaredDistance(pt, center);

                // If it is a circular object, check whether we're inside
                bool ninside;
                if (const CircularObject* co = dynamic_cast<const CircularObject*>(mo)) {
                    int32_t radiusSquared;
                    ninside = (co->getRadiusSquared().get(radiusSquared) && ndist2 <= radiusSquared);
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
game::map::ObjectType::findPreviousIndexWrap(Id_t index)
{
    Id_t n = findPreviousIndexNoWrap(index);
    if (n == 0) {
        n = findPreviousIndexNoWrap(0);
    }
    return n;
}

// Get next object after index, with wrap.
game::Id_t
game::map::ObjectType::findNextIndexWrap(Id_t index)
{
    Id_t n = findNextIndexNoWrap(index);
    if (n == 0) {
        n = findNextIndexNoWrap(0);
    }
    return n;
}

// Get previous object before index.
game::Id_t
game::map::ObjectType::findPreviousIndexNoWrap(Id_t index)
{
    while (1) {
        index = getPreviousIndex(index);
        if (index == 0 || getObjectByIndex(index) != 0) {
            return index;
        }
    }
}

// Get next object after index.
game::Id_t
game::map::ObjectType::findNextIndexNoWrap(Id_t index)
{
    return findNextIndex(index);
}

// Get previous object before index, with wrap.
game::Id_t
game::map::ObjectType::findPreviousIndexWrap(Id_t index, bool marked)
{
    // ex GObjectType::findPreviousIndexWrap
    // ex types.pas:CObjectType.Prev
    return ByMarked(*this, marked).findPreviousIndexWrap(index);
}

// Get next object after index, with wrap.
game::Id_t
game::map::ObjectType::findNextIndexWrap(Id_t index, bool marked)
{
    // ex GObjectType::findNextIndexWrap
    // ex types.pas:CObjectType.Next
    return ByMarked(*this, marked).findNextIndexWrap(index);
}

// Get previous object before index.
game::Id_t
game::map::ObjectType::findPreviousIndexNoWrap(Id_t index, bool marked)
{
    // ex GObjectType::findPreviousIndexNoWrap
    return ByMarked(*this, marked).findPreviousIndexNoWrap(index);
}

// Get next object after index.
game::Id_t
game::map::ObjectType::findNextIndexNoWrap(Id_t index, bool marked)
{
    // ex GObjectType::findNextIndexNoWrap
    return ByMarked(*this, marked).findNextIndexNoWrap(index);
}

game::Id_t
game::map::ObjectType::findNextObjectAt(Point pt, int index, bool marked)
{
    return ByPosition(*this, pt).findNextIndexNoWrap(index, marked);
}

game::Id_t
game::map::ObjectType::findPreviousObjectAt(Point pt, int index, bool marked)
{
    return ByPosition(*this, pt).findPreviousIndexNoWrap(index, marked);
}

game::Id_t
game::map::ObjectType::findNextObjectAtWrap(Point pt, int index, bool marked)
{
    return ByPosition(*this, pt).findNextIndexWrap(index, marked);
}

game::Id_t
game::map::ObjectType::findPreviousObjectAtWrap(Point pt, int index, bool marked)
{
    return ByPosition(*this, pt).findPreviousIndexWrap(index, marked);
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

game::Id_t
game::map::ObjectType::findIndexForObject(const Object* obj)
{
    if (obj != 0) {
        for (Id_t i = findNextIndex(0); i != 0; i = findNextIndex(i)) {
            if (getObjectByIndex(i) == obj) {
                return i;
            }
        }
    }
    return 0;
}

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
