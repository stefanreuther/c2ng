/**
  *  \file game/ref/list.hpp
  *  \brief Class game::ref::List
  */
#ifndef C2NG_GAME_REF_LIST_HPP
#define C2NG_GAME_REF_LIST_HPP

#include <vector>
#include "afl/bits/smallset.hpp"
#include "game/map/universe.hpp"
#include "game/reference.hpp"
#include "game/types.hpp"

namespace game { namespace ref {

    class SortPredicate;

    /** List of references.
        Wraps a std::vector<Reference> and offers useful methods for creating and accessing it.

        This class makes no assumption about the content of the references.
        In particular, duplicates or references to nonexistant objects are permitted. */
    class List {
     public:
        /*
         *  Types
         */

        /** Option for addObjectsAt(). */
        enum Option {
            IncludeForeignShips,           ///< If set, include foreign ships. Default is only own (=Playable/ReadOnly) ships.
            IncludePlanet,                 ///< If set, include planet. Default is only ships.
            SafeShipsOnly                  ///< If set, only reliable ship. Default is also guessed ships (if permitted using IncludeForeignShips).
        };
        typedef afl::bits::SmallSet<Option> Options_t;

        /** Shortcut for underlying vector. */
        typedef std::vector<Reference> Vector_t;

        /** Shortcut for set of reference types. */
        typedef afl::bits::SmallSet<Reference::Type> Types_t;


        /*
         *  Methods
         */

        /** Constructor.
            Makes an empty list. */
        List();

        /** Destructor. */
        ~List();

        /** Add single reference.
            @param ref Reference to add */
        void add(Reference ref);

        /** Add multiple references.
            @param type Reference type
            @param ids  Ids to add */
        void add(Reference::Type type, const std::vector<Id_t>& ids);

        /** Add all objects at a particular location.
            This is used for certain lists of ships.
            @param univ          Universe
            @param pt            Location
            @param options       Options to select which objects to add
            @param excludeShipId Do not include this ship Id even if it is otherwise eligible (0=exclude none) */
        void addObjectsAt(game::map::Universe& univ, game::map::Point pt, Options_t options, Id_t excludeShipId);

        /** Clear list. */
        void clear();

        /** Access element.
            @param pos Index [0,size())
            @return element. Null reference if index is out of bounds */
        Reference operator[](size_t pos) const;

        /** Set element at position.
            @param pos Index [0,size()). Call is ignored if index is out of bounds.
            @param ref New value */
        void set(size_t pos, Reference ref);

        /** Get size (number of references in vector).
            @return size */
        size_t size() const;

        /** Get set of types of references in this set.
            For example, if this set contains only ship references, the result will be a unit set containing Reference::Ship.
            @return set */
        Types_t getTypes() const;

        /** Get Ids of a given type.
            For example, with type=Reference::Ship, will return a list of ship Ids.
            @param type Type to check
            @return List of Ids, in same order as contained in the list */
        std::vector<Id_t> getIds(Reference::Type type) const;

        /** Sort this list.
            @param pred SortPredicate */
        void sort(const SortPredicate& pred);

     private:
        Vector_t m_content;
    };

} }

#endif
