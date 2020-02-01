/**
  *  \file game/ref/userlist.hpp
  */
#ifndef C2NG_GAME_REF_USERLIST_HPP
#define C2NG_GAME_REF_USERLIST_HPP

#include "util/skincolor.hpp"
#include "game/reference.hpp"
#include "game/session.hpp"

namespace game { namespace ref {

    class List;
    class SortPredicate;

    class UserList {
     public:
        enum Type {
            OtherItem,          ///< Other string item.
            ReferenceItem,      ///< Normal reference item.
            DividerItem,        ///< Divider (not selectable).
            SubdividerItem      ///< Subdivider (not selectable).
        };

        struct Item {
            Type type;
            String_t name;
            Reference reference;
            bool marked;
            util::SkinColor::Color color;

            Item(Type type, String_t name, Reference ref, bool marked, util::SkinColor::Color color)
                : type(type), name(name), reference(ref), marked(marked), color(color)
                { }

            bool operator==(const Item& other) const
                { return type == other.type && name == other.name && reference == other.reference && marked == other.marked && color == other.color; }
            bool operator!=(const Item& other) const
                { return !operator==(other); }
        };

        UserList();

        ~UserList();

        void clear();

        void add(Type type, String_t name, Reference ref, bool marked, util::SkinColor::Color color);

        void add(const List& list, Session& session, const SortPredicate& divi, const SortPredicate& subdivi);

        void add(const UserList& list);

        const Item* get(size_t index) const;

        size_t size() const;

        bool empty() const;

        bool find(Reference ref, size_t& pos) const;

        bool operator==(const UserList& other) const;
        bool operator!=(const UserList& other) const;

        // FIXME: some synchronizing primitives

     private:
        std::vector<Item> m_items;
    };


} }

#endif
