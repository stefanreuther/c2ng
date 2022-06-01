/**
  *  \file game/ref/fleetlist.hpp
  *  \brief Class game::ref::FleetList
  */
#ifndef C2NG_GAME_REF_FLEETLIST_HPP
#define C2NG_GAME_REF_FLEETLIST_HPP

#include "afl/base/optional.hpp"
#include "game/map/point.hpp"
#include "game/map/universe.hpp"
#include "game/ref/userlist.hpp"

namespace game { namespace ref {

    class SortPredicate;

    /** Fleet list.
        Similar in style to UserList, but in addition to the information stored by a UserList, also stores "is-here" information.

        Like UserList, this stores a list of Reference's with additional information, ready for rendering.
        Whereas it accepts and fully supports all sorts of references (like UserList),
        it is intended to store only references to ships (=fleet leaders).

        This is a data class that doesn't keep any (C++) references and can be passed between threads. */
    class FleetList {
     public:
        /** List item. Extends UserList::Item with the turn information. */
        struct Item : public UserList::Item {
            /** true if fleet leader is at reference location. */
            bool isAtReferenceLocation;

            /** Constructor.
                \param item       UserList::item information
                \param isAtReferenceLocation true if fleet leader is at reference location */
            Item(UserList::Item item, bool isAtReferenceLocation)
                : UserList::Item(item), isAtReferenceLocation(isAtReferenceLocation)
                { }

            bool operator==(const Item& other) const;
            bool operator!=(const Item& other) const;
        };

        /** Sort predicate for Item. */
        class SortPredicate {
         public:
            /** Compare two items.
                \param a,b  Items
                \return negative if a goes before b, zero if equivalent positive if a goes after b. */
            virtual int compare(const Item& a, const Item& b) const = 0;

            /** Get class name (for dividers).
                \param a  Item
                \return Class name (can be empty) */
            virtual String_t getClass(const Item& a) const = 0;
        };

        /** Constructor.
            Make an empty list. */
        FleetList();

        /** Destructor. */
        ~FleetList();

        /** Clear.
            \post empty() */
        void clear();

        /** Add an item.
            \param item Item to add */
        void add(const Item& item);

        /** Get item.
            \param index Index [0,size())
            \return Item; null if index is invalid */
        const Item* get(size_t index) const;

        /** Get number of items.
            \return number of items */
        size_t size() const;

        /** Check emptiness.
            \return true if list is empty */
        bool empty() const;

        /** Find a reference.
            \param [in]  ref Reference to find
            \param [out] pos Found index
            \retval true reference found; pos has been set
            \retval false reference not found */
        bool find(Reference ref, size_t& pos) const;

        /** Find initial selection.
            Used to place initial cursor.
            \return index to first fleet marked "here", first fleet, or 0 */
        size_t findInitialSelection() const;

        /** Sort (FleetList predicate).
            Will sort the items and add appropriate dividers.
            \param p Predicate */
        void sort(const SortPredicate& p);

        /** Sort (Reference predicate).
            Will sort the items and add appropriate dividers.
            \param p Predicate */
        void sort(const game::ref::SortPredicate& p);

        /** Add all fleets to this list.
            The reference location is used to filter fleets by fleet leader location, and set the "isAtReferenceLocation" attribute.
            Note that if no reference location is given, and includeAll is not set, no fleets are added!
            \param univ        Universe
            \param refLoc      Reference location; only add fleets whose leader is at this position
            \param except      Do not include this fleet
            \param includeAll  If set, add all fleets (except for except), regardless of position
            \param tx          Translator */
        void addAll(const game::map::Universe& univ, afl::base::Optional<game::map::Point> refLoc, Id_t except, bool includeAll, afl::string::Translator& tx);

        /** Compare for equality.
            \param other Other list
            \return true if equal */
        bool operator==(const FleetList& other) const;
        bool operator!=(const FleetList& other) const;
     private:
        std::vector<Item> m_items;
    };

} }

#endif
