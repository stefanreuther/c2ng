/**
  *  \file game/ref/userlist.hpp
  *  \brief Class game::ref::UserList
  */
#ifndef C2NG_GAME_REF_USERLIST_HPP
#define C2NG_GAME_REF_USERLIST_HPP

#include "afl/base/optional.hpp"
#include "game/map/object.hpp"
#include "game/reference.hpp"
#include "game/session.hpp"
#include "util/skincolor.hpp"

namespace game { namespace ref {

    class List;
    class SortPredicate;

    /** List of object references, augmented for user-interface use.
        This object represents a list of object references, dividers, and other items.
        It carries sufficient information to render in a user-interface without requiring access to other game data.

        This is a data class that doesn't keep any (C++) references and can be passed between threads. */
    class UserList {
     public:
        /** Item type. */
        enum Type {
            OtherItem,          ///< Other string item.
            ReferenceItem,      ///< Normal reference item.
            DividerItem,        ///< Divider (not selectable).
            SubdividerItem      ///< Subdivider (not selectable).
        };

        /** List item. */
        struct Item {
            Type type;                                         ///< Type.
            String_t name;                                     ///< Name or text to display.
            Reference reference;                               ///< Associated game object reference.
            bool marked;                                       ///< true if item is marked.
            game::map::Object::Playability playability : 8;    ///< Object playability.
            util::SkinColor::Color color : 8;                  ///< Item color, derived from team relation.

            Item(Type type, String_t name, Reference ref, bool marked, game::map::Object::Playability playability, util::SkinColor::Color color)
                : type(type), name(name), reference(ref), marked(marked), playability(playability), color(color)
                { }

            bool operator==(const Item& other) const
                { return type == other.type && name == other.name && reference == other.reference && marked == other.marked && playability == other.playability && color == other.color; }
            bool operator!=(const Item& other) const
                { return !operator==(other); }
        };

        /** Constructor.
            Make an empty list. */
        UserList();

        /** Destructor. */
        ~UserList();

        /** Clear.
            Remove all content.
            @post size() == 0 */
        void clear();

        /** Add single item.
            @param type        Type
            @param name        Name or text to display
            @param ref         Associated game object reference
            @param marked      true if item is marked
            @param playability Object playability
            @param color       Item color, derived from team relation */
        void add(Type type, String_t name, Reference ref, bool marked, game::map::Object::Playability playability, util::SkinColor::Color color);

        /** Add elements from a List.

            The input list must be sorted according to divi.then(subdivi) for to make dividers work correctly.
            If no dividers are to be created (i.e. divi/subdivi = NullPredicate), list may be unsorted.

            @param list    Sorted input list.
            @param session Session (for retrieving object properties)
            @param divi    Predicate for generating major dividers (DividerItem)
            @param subdivi Predicate for generating minor dividers (SubdividerItem) */
        void add(const List& list, Session& session, const SortPredicate& divi, const SortPredicate& subdivi);

        /** Add other list.
            Adds a (copy of) all items of the other list.
            @param list List */
        void add(const UserList& list);

        /** Get item by index.
            @param index Index [0,size())
            @return item; null if index out of range */
        const Item* get(size_t index) const;

        /** Get number of items.
            @return number of items */
        size_t size() const;

        /** Check emptiness.
            @return true if list is empty */
        bool empty() const;

        /** Find reference.
            @param ref Reference to find
            @return Index of reference such that get(pos)->reference == ref, if any. */
        afl::base::Optional<size_t> find(Reference ref) const;

        /** Compare for equality.
            @param other Other list
            @return true if lists are identical */
        bool operator==(const UserList& other) const;

        /** Compare for inequality.
            @param other Other list
            @return true if lists are differnet */
        bool operator!=(const UserList& other) const;

        /** Make a list item, given a reference.
            @param r       Reference
            @param session Session (for retrieving object properties)
            @return Item */
        static Item makeReferenceItem(Reference r, Session& session);

     private:
        std::vector<Item> m_items;
    };

} }

#endif
