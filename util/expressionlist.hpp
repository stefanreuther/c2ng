/**
  *  \file util/expressionlist.hpp
  *  \brief Class util::ExpressionList
  */
#ifndef C2NG_UTIL_EXPRESSIONLIST_HPP
#define C2NG_UTIL_EXPRESSIONLIST_HPP

#include "afl/string/string.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/base/types.hpp"

namespace util {

    /** Expression list, used for LRU and predefined-expression lists.
        Contains a list of items and operations to work with them.

        Each item has
        - a name
        - a set of flags
        - a value

        The set of flags is an optional, bracket-enclosed string.

        @change In PCC2, flags and value were on attribute and all users performed the split. */
    class ExpressionList {
     public:
        struct Item {
            // ex LRUListItem
            String_t name;            ///< Name of this entry.
            String_t flags;           ///< Flags, including the [], if any.
            String_t value;           ///< Value (=expression).
            Item(String_t name, String_t flags, String_t value)
                : name(name), flags(flags), value(value)
                { }
        };

        /** Create empty list. */
        ExpressionList();

        /** Destructor. */
        ~ExpressionList();


        /** Get number of elements in this list.
            \return number of items */
        size_t size() const;

        /** Check for emptiness.
            \return true if list is empty */
        bool empty() const;

        /** Get item by index.
            \param index [0,size()) Index
            \return item; null if index out of range */
        const Item* get(size_t index) const;

        /** Find value.
            \param [in]  value Value to search
            \param [out] index Found index
            \return true Index [0,size()) if found, size() if not found */
        bool findIndexForValue(const String_t& value, size_t& index) const;

        /** Append new item at end.
            \param item Newly-allocated item. Will be owned by the list. Should not be null. */
        void pushBackNew(Item* item);

        /** Add new item at front, with LRU-style limiting.
            The item will be added at the front.
            Duplicates will be removed (this will make it appear to move the item from its original position to the front,
            but in case the items differ in their flags, the new flags will be used). 
            The list will be limited to @c limit items.
            \param item Newly-allocated item. Will be owned by the list. Should not be null.
            \param limit Maximum number of elements to preserve */
        void pushFrontNew(Item* item, size_t limit);

        /** Move item to front, given an index.
            \param index [0,size()) Item to move to front. */
        void moveToFront(size_t index);

        /** Clear this list. */
        void clear();

     private:
        afl::container::PtrVector<Item> m_items;
    };

}

#endif
