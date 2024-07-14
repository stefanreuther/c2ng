/**
  *  \file server/talk/newsrc.hpp
  *  \brief Class server::talk::Newsrc
  */
#ifndef C2NG_SERVER_TALK_NEWSRC_HPP
#define C2NG_SERVER_TALK_NEWSRC_HPP

#include "afl/net/redis/subtree.hpp"
#include "afl/net/redis/integerkey.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/string/string.hpp"
#include "afl/base/types.hpp"

namespace server { namespace talk {

    /** Newsrc.
        Stores a set of postings the user already read.
        Optimized for conserving space.

        This implements a simple cache so that not each operation on newsrc hits the database.
        Use save() after modifications. */
    class Newsrc {
     public:
        /** Constructor.
            \param root Database node */
        explicit Newsrc(afl::net::redis::Subtree root);

        /** Save changes to database. */
        void save();

        /** Get message state.
            \param messageId Message Id
            \retval true Forum message has been read
            \retval false Forum message still unread */
        bool get(int32_t messageId);

        /** Set message state (mark read).
            \param messageId Message Id */
        void set(int32_t messageId);

        /** Clear message state (mark unread).
            \param messageId Message Id */
        void clear(int32_t messageId);

     private:
        afl::net::redis::IntegerKey index();
        afl::net::redis::HashKey data();

        void loadCache(int32_t index);
        void doLoad(int32_t index);

        afl::net::redis::Subtree m_root;

        int32_t m_readAllBelowLine;

        /** Cache. */
        String_t m_cache;
        int32_t m_cacheIndex;
        bool m_cacheDirty;
    };

} }

#endif
