/**
  *  \file game/map/objecttype.hpp
  *  \brief Base class game::map::ObjectType
  */
#ifndef C2NG_GAME_MAP_OBJECTTYPE_HPP
#define C2NG_GAME_MAP_OBJECTTYPE_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/deleter.hpp"
#include "afl/base/signal.hpp"
#include "game/map/point.hpp"
#include "game/playerset.hpp"
#include "game/types.hpp"
#include "game/reference.hpp"
#include "game/ref/sortpredicate.hpp"

namespace game { namespace map {

    class Object;
    class Configuration;

    /** Object type descriptor.
        A class derived from ObjectType defines a set of objects ("all ships", "played starbases"), and iteration through them.

        An object is identified by a non-zero index.
        An valid index can be turned into an object using getObjectByIndex(), which returns null for invalid objects.

        The base class provides methods getNextIndex() and getPreviousIndex() for iteration.
        Those are not constrained to return only valid indexes (=isValidIndex() returns true).
        Therefore, users will most likely use findNextIndexWrap() etc.,
        which only return valid object indexes, and can optionally filter for marked objects.

        If the underlying set changes (as opposed to: the underlying objects change),
        the implementor must raise sig_setChange.

        Note: uniform ObjectTypes (i.e. most of them) should derive from TypedObjectType. */
    class ObjectType : public afl::base::Deletable {
     public:
        /** Virtual destructor. */
        virtual ~ObjectType();

        /** Get object, given an index.
            \param index Index
            \return object if it exists, null otherwise */
        virtual Object* getObjectByIndex(Id_t index) = 0;

        /** Get next index.
            The returned index need not be valid as per getObjectByIndex(),
            but the implementation must guarantee that repeated calls to getNextIndex()
            ultimately end up at 0, so that loops actually terminate.
            There is no requirement that the indexes are reported in a particular order, though.
            \param index Starting index. Can be 0 to obtain the first index.
            \return 0 if no more objects, otherwise an index */
        virtual Id_t getNextIndex(Id_t index) const = 0;

        /** Get previous index.
            The returned index need not be valid as per getObjectByIndex(),
            but the implementation must guarantee that repeated calls to getNextIndex()
            ultimately end up at 0, so that loops actually terminate.
            There is no requirement that the indexes are reported in a particular order, though.
            \param index Starting index. Can be 0 to obtain the last index.
            \return 0 if no more objects. Otherwise an index */
        virtual Id_t getPreviousIndex(Id_t index) const = 0;

        /*
         *  Meta-operations
         */

        /** Filter by position.
            Create a view on this ObjectType that contains only the objects from this one which are at the given position.
            Ids are returned unchanged from the underlying ObjectType; filtered-out objects are skipped.

            \param del Deleter to control lifetime of created objects
            \param pt  Position to search
            \return new ObjectType */
        ObjectType& filterPosition(afl::base::Deleter& del, Point pt);

        /** Filter by owner.
            Create a view on this ObjectType that contains only the objects from this one which are owned by one of the given players.
            Ids are returned unchanged from the underlying ObjectType; filtered-out objects are skipped.

            \param del Deleter to control lifetime of created objects
            \param owners Owners to search
            \return new ObjectType */
        ObjectType& filterOwner(afl::base::Deleter& del, PlayerSet_t owners);

        /** Filter by marked status.
            If \c marked is given as true, create a view on this ObjectType that contains only marked objects;
            otherwise, returns this ObjectType unchanged.
            Ids are returned unchanged from the underlying ObjectType; filtered-out objects are skipped.

            \param del Deleter to control lifetime of created objects
            \param marked flag
            \return new ObjectType */
        ObjectType& filterMarked(afl::base::Deleter& del, bool marked);

        /** Sort.
            Create a view on this ObjectType that contains all the objects from this one, sorted by the given predicate.
            Ids are returned unchanged from the underlying ObjectType, objects are returned in sorted order.
            Objects are sorted by the given predicate, then by Id, then by index.

            All objects must have the same type as defined by \c type.

            The result is intended for one-time, temporary objects:
            - Do not use for iteration.
              Iterating over the entire result is an O(n^2) operation.
              Instead, build a game::ref::List and sort that, which is O(nlogn).
            - Use as last in a chain, i.e. do filterOwner().sort(), not sort().filterOwner();
            - Instead of sort(X).sort(Y), use sort(X.then(Y)).

            \param del Deleter to control lifetime of created objects
            \param pred Sort predicate; must live as long as result is being used
            \param type Object type; used to construct references */
        ObjectType& sort(afl::base::Deleter& del, const game::ref::SortPredicate& pred, Reference::Type type);


        /*
         *  Derived functions
         */

        /** Find next object after index.
            Repeatedly calls getNextIndex() until it finds an object that exists (non-null getObjectByIndex()).

            This function is the same as findNextIndexNoWrap(index, false).
            It is intended for iteration.
            \param index Index to start search at. */
        Id_t findNextIndex(Id_t index) const;

        /** Check emptiness.
            \return true this type is empty, i.e. has no objects
            \return false this type is not empty, i.e. has objects */
        bool isEmpty() const;

        /** Check unit type.
            \return true this type has precisely one object
            \return false this type has more or fewer than one objects */
        bool isUnit() const;

        /** Count objects.
            \return Number of objects */
        int countObjects() const;

        /** Count objects at position.
            \param pt Count objects at this location
            \param owners Owners to accept
            \return Number of objects */
        int countObjectsAt(const Point pt, PlayerSet_t owners);

        /** Find nearest object.
            \param pt origin point
            \param config map configuration (for wrap awareness)
            \return Index of nearest object, 0 if none */
        Id_t findNearestIndex(const Point pt, const Configuration& config);

        /** Get previous object before index, with wrap.
            If the last object of a kind is reached, search starts again at the beginning.
            \param index index to start search at.
            \return found index; 0 if none */
        Id_t findPreviousIndexWrap(Id_t index);

        /** Get next object after index, with wrap.
            If the last object of a kind is reached, search starts again at the beginning.
            \param index Index to start search at.
            \return found index; 0 if none */
        Id_t findNextIndexWrap(Id_t index);

        /** Get previous object before index.
            The returned object is guaranteed to exist.
            \param index Index to start search at.
            \return found index; 0 if none */
        Id_t findPreviousIndexNoWrap(Id_t index);

        /** Get next object after index.
            The returned object is guaranteed to exist.
            \param index Index to start search at.
            \return found index; 0 if none */
        Id_t findNextIndexNoWrap(Id_t index);

        /** Get previous object before index, with wrap.
            If the last object of a kind is reached, search starts again at the beginning.
            Can filter marked objects.
            \param index index to start search at.
            \param marked true to return only marked objects
            \return found index; 0 if none */
        Id_t findPreviousIndexWrap(Id_t index, bool marked);

        /** Get next object after index, with wrap.
            If the last object of a kind is reached, search starts again at the beginning.
            Can filter marked objects.
            \param index Index to start search at.
            \param marked true to return only marked objects
            \return found index; 0 if none */
        Id_t findNextIndexWrap(Id_t index, bool marked);

        /** Get previous object before index.
            Can filter marked objects.
            The returned object is guaranteed to exist.
            \param index Index to start search at.
            \param marked true to return only marked objects
            \return found index; 0 if none */
        Id_t findPreviousIndexNoWrap(Id_t index, bool marked);

        /** Get next object after index.
            Can filter marked objects.
            The returned object is guaranteed to exist.
            \param index Index to start search at.
            \param marked true to return only marked objects
            \return found index; 0 if none */
        Id_t findNextIndexNoWrap(Id_t index, bool marked);

        /** Find next object at a given position.
            Can filter marked objects.
            The returned object is guaranteed to exist.
            \param pt Position
            \param index Index to start search at (0=report first matching object)
            \param marked true to return only marked objects
            \return found index; 0 if none */
        Id_t findNextObjectAt(Point pt, int index, bool marked);

        /** Find next object at a given position.
            Can filter marked objects.
            The returned object is guaranteed to exist.
            \param pt Position
            \param index Index to start search at (0=report last matching object)
            \param marked true to return only marked objects
            \return found index; 0 if none */
        Id_t findPreviousObjectAt(Point pt, int index, bool marked);

        /** Find next object at a given position, with wrap.
            Can filter marked objects.
            The returned object is guaranteed to exist.
            \param pt Position
            \param index Index to start search at (0=report first matching object)
            \param marked true to return only marked objects
            \return found index; 0 if none */
        Id_t findNextObjectAtWrap(Point pt, int index, bool marked);

        /** Find next object at a given position, with wrap.
            Can filter marked objects.
            The returned object is guaranteed to exist.
            \param pt Position
            \param index Index to start search at (0=report last matching object)
            \param marked true to return only marked objects
            \return found index; 0 if none */
        Id_t findPreviousObjectAtWrap(Point pt, int index, bool marked);

        /** Find object index, given an Id.
            \param id Id
            \return found index; 0 if none. */
        Id_t findIndexForId(Id_t id);

        /** Find object index, given an object.
            \param obj Object
            \return found index; 0 if none. */
        Id_t findIndexForObject(const Object* obj);

        /** Notify all object listeners.
            Calls Object::notifyListeners() on all objects that are modified (Object::isDirty()).
            \retval false No object was dirty, no listeners notified
            \retval true Some objects were dirty */
        bool notifyObjectListeners();

        /** Called when the underlying set changes, i.e. objects come and go or are replaced by different objects.
            Called after the change.

            For simple changes, the integer can be a hint for users, i.e. the new Id of a renamed object.
            If the emitter doesn't want to give a hint, it can pass 0. */
        afl::base::Signal<void(Id_t)> sig_setChange;
    };

} }

#endif
