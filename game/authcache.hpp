/**
  *  \file game/authcache.hpp
  *  \brief Class game::AuthCache
  */
#ifndef C2NG_GAME_AUTHCACHE_HPP
#define C2NG_GAME_AUTHCACHE_HPP

#include <vector>
#include "afl/container/ptrvector.hpp"
#include "afl/string/string.hpp"
#include "afl/base/optional.hpp"

namespace game {

    /** Authentication cache.
        As of October 2019, this is just intended to support the "AuthPlayer" script command.
        It could possibly be extended to cache other temporary authentication information. */
    class AuthCache {
     public:
        /** Item.
            Members are of kind "match" (determine which items are returned from a query)
            and "result" (provide passwords).
            Default-constructing provides a match-all, provide-nothing item.
            This is a structure with no parameterized constructor to allow adding new fields
            without disturbing existing code.

            To add new authentication information, create an Item and populate all match/result
            fields before calling addNew().

            To query authentication information, create an Item and populate all match fields
            before calling find(). In the result, check each item whether it has the desired
            result information. */
        struct Item {
            /** Match: player number. */
            afl::base::Optional<int> playerNr;

            /** Result: password. */
            afl::base::Optional<String_t> password;

            Item()
                : playerNr(), password()
                { }
        };

        typedef std::vector<const Item*> Items_t;

        /** Default constructor.
            Makes empty cache. */
        AuthCache();

        /** Destructor. */
        ~AuthCache();

        /** Clear cache.
            Discards all content and invalidates all find() results. */
        void clear();

        /** Add new item.
            \param pItem Newly-allocated item */
        void addNew(Item* pItem);

        /** Find items.
            \param match Item containing fields to match
            \return vector of matching items */
        Items_t find(const Item& match) const;

     private:
        afl::container::PtrVector<Item> m_content;
    };

}

#endif
