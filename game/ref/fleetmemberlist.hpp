/**
  *  \file game/ref/fleetmemberlist.hpp
  *  \brief Class game::ref::FleetMemberList
  */
#ifndef C2NG_GAME_REF_FLEETMEMBERLIST_HPP
#define C2NG_GAME_REF_FLEETMEMBERLIST_HPP

#include "afl/base/optional.hpp"
#include "afl/bits/smallset.hpp"
#include "game/map/universe.hpp"
#include "game/ref/userlist.hpp"

namespace game { namespace ref {

    class SortPredicate;

    /** Fleet member list.
        Similar in style to UserList, but in addition to the information stored by a UserList, also stores additional information for each member.

        Like UserList, this stores a list of Reference's with additional information, ready for rendering.
        Whereas it accepts and fully supports all sorts of references (like UserList),
        it is intended to store only references to ships (=fleet members).

        This is a data class that doesn't keep any (C++) references and can be passed between threads. */
    class FleetMemberList {
     public:
        /** Flag for a fleet member. */
        enum Flag {
            Leader,                ///< This is the fleet leader. ex fLeader
            Towed,                 ///< This ship is being towed by another fleet member. ex fTowed
            Towing,                ///< This ship is towing another fleet member. ex fTower
            Away                   ///< This ship is not at the same position as the fleet leader. ex fAway
        };

        /** Set of flags. */
        typedef afl::bits::SmallSet<Flag> Flags_t;

        /** List item. Extends UserList::Item with the turn information. */
        struct Item : public UserList::Item {
            /** Flags for this unit. */
            Flags_t flags;

            /** Friendly code. */
            String_t friendlyCode;

            /** Member position. */
            game::map::Point position;

            /** Constructor.
                @param item         UserList::item information
                @param flags        Flags
                @param friendlyCode Ship friendly code
                @param position     Ship position */
            Item(UserList::Item item, Flags_t flags, String_t friendlyCode, game::map::Point position)
                : UserList::Item(item), flags(flags), friendlyCode(friendlyCode), position(position)
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
        FleetMemberList();

        /** Destructor. */
        ~FleetMemberList();

        /** Clear.
            @post empty() */
        void clear();

        /** Add an item.
            @param item Item to add */
        void add(const Item& item);

        /** Get item.
            @param index Index [0,size())
            @return Item; null if index is invalid */
        const Item* get(size_t index) const;

        /** Get number of items.
            @return number of items */
        size_t size() const;

        /** Check emptiness.
            @return true if list is empty */
        bool empty() const;

        /** Find a reference.
            @param ref Reference to find
            @return Found index, if any */
        afl::base::Optional<size_t> find(Reference ref) const;

        /** Sort (FleetList predicate).
            Will sort the items and add appropriate dividers.
            @param p Predicate */
        void sort(const SortPredicate& p);

        /** Sort (Reference predicate).
            Will sort the items and add appropriate dividers.
            @param p Predicate */
        void sort(const game::ref::SortPredicate& p);

        /** Add all fleets to this list.
            Updates this object's content to contain the given fleet.
            If this changes the object's content, emits a sig_change signal.

            If the given fleet number is 0, clears the list.
            If the given fleet number is not a valid fleet Id, lists just the given ship.
            Otherwise, lists all fleet members.

            @param univ        Universe
            @param fleetNumber Number of fleet to add */
        void setFleet(const game::map::Universe& univ, Id_t fleetNumber);

        /** Compare for equality.
            @param other Other list
            @return true if equal */
        bool operator==(const FleetMemberList& other) const;
        bool operator!=(const FleetMemberList& other) const;
     private:
        std::vector<Item> m_items;
    };

} }

#endif
