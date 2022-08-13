/**
  *  \file server/user/usertoken.hpp
  *  \brief Class server::user::UserToken
  */
#ifndef C2NG_SERVER_USER_USERTOKEN_HPP
#define C2NG_SERVER_USER_USERTOKEN_HPP

#include "server/interface/usertoken.hpp"
#include "server/types.hpp"

namespace server { namespace user {

    class Root;

    /** Implementation of UserToken interface.
        This interface allows accessing users' access tokens. */
    class UserToken : public server::interface::UserToken {
     public:
        /** Constructor.
            @param root Service root (database, configuration) */
        UserToken(Root& root);

        // Interface methods:
        virtual String_t getToken(String_t userId, String_t tokenType);
        virtual Info checkToken(String_t token, afl::base::Optional<String_t> requiredType, bool autoRenew);
        virtual void clearToken(String_t userId, afl::base::Memory<const String_t> tokenTypes);

        /** Delete a single token.
            Ensures that the given token is not valid afterwards.
            @param userId     User Id
            @param tokenType  Token type
            @param token      Token */
        void deleteToken(String_t userId, String_t tokenType, String_t token);

        /** Create a new token.
            @param userId     User Id
            @param tokenType  Token type
            @param validUntil Time when token expires
            @return token */
        String_t createToken(String_t userId, String_t tokenType, Time_t validUntil);

     private:
        Root& m_root;
    };

} }

#endif
