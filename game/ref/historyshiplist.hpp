/**
  *  \file game/ref/historyshiplist.hpp
  *  \brief Class game::ref::HistoryShipList
  */
#ifndef C2NG_GAME_REF_HISTORYSHIPLIST_HPP
#define C2NG_GAME_REF_HISTORYSHIPLIST_HPP

#include "afl/base/optional.hpp"
#include "game/ref/userlist.hpp"

namespace game { namespace ref {

    class SortPredicate;

    /** History ship list.
        Similar in style to UserList, but in addition to the information stored by a UserList, also stores a "turn-last-seen" information.

        Like UserList, this stores a list of Reference's with additional information, ready for rendering.
        Whereas it accepts and fully supports all sorts of references (like UserList),
        it is intended to store only references to ships.

        This is a data class that doesn't keep any (C++) references and can be passed between threads. */
    class HistoryShipList {
     public:
        /** List item. Extends UserList::Item with the turn information. */
        struct Item : public UserList::Item {
            /** Turn number. */
            int turnNumber;

            /** Constructor.
                \param item       UserList::item information
                \param turnNumber Turn number */
            Item(UserList::Item item, int turnNumber)
                : UserList::Item(item), turnNumber(turnNumber)
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
        HistoryShipList();

        /** Destructor. */
        ~HistoryShipList();

        /** Set reference turn number.
            A turn number is useful for interpreting the turn number of the individual items.
            \param turnNumber Turn number */
        void setReferenceTurn(int turnNumber);

        /** Get reference turn number.
            \return turn number */
        int getReferenceTurn() const;

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
            \param ref Reference to find
            \return Found index, if any */
        afl::base::Optional<size_t> find(Reference ref) const;

        /** Sort (HistoryShipList predicate).
            Will sort the items and add appropriate dividers.
            \param p Predicate */
        void sort(const SortPredicate& p);

        /** Sort (Reference predicate).
            Will sort the items and add appropriate dividers.
            \param p Predicate */
        void sort(const game::ref::SortPredicate& p);

        /** Compare for equality.
            \param other Other list
            \return true if equal */
        bool operator==(const HistoryShipList& other) const;
        bool operator!=(const HistoryShipList& other) const;
     private:
        std::vector<Item> m_items;
        int m_turnNumber;
    };

} }

#endif
