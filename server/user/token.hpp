/**
  *  \file server/user/token.hpp
  */
#ifndef C2NG_SERVER_USER_TOKEN_HPP
#define C2NG_SERVER_USER_TOKEN_HPP

#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/integerfield.hpp"

namespace server { namespace user {

    class Token {
     public:
        Token(afl::net::redis::HashKey key);

        afl::net::redis::StringField userId();

        afl::net::redis::StringField tokenType();

        afl::net::redis::IntegerField validUntil();

        void remove();

     private:
        afl::net::redis::HashKey m_key;
    };

} }

#endif
