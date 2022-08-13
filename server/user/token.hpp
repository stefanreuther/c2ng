/**
  *  \file server/user/token.hpp
  *  \brief Class server::user::Token
  */
#ifndef C2NG_SERVER_USER_TOKEN_HPP
#define C2NG_SERVER_USER_TOKEN_HPP

#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/integerfield.hpp"

namespace server { namespace user {

    /** Database access to a token's metainformation. */
    class Token {
     public:
        /** Constructor.
            @param key Token key ("token:t:<id>") */
        Token(afl::net::redis::HashKey key);

        /** Access user Id.
            @return user Id */
        afl::net::redis::StringField userId();

        /** Access token type.
            @return token type */
        afl::net::redis::StringField tokenType();

        /** Access expiration date.
            @return expiration date */
        afl::net::redis::IntegerField validUntil();

        /** Remove this token. */
        void remove();

     private:
        afl::net::redis::HashKey m_key;
    };

} }

#endif
