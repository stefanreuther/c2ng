/**
  *  \file server/user/usertoken.hpp
  */
#ifndef C2NG_SERVER_USER_USERTOKEN_HPP
#define C2NG_SERVER_USER_USERTOKEN_HPP

#include "server/interface/usertoken.hpp"
#include "server/types.hpp"

namespace server { namespace user {

    class Root;

    class UserToken : public server::interface::UserToken {
     public:
        UserToken(Root& root);

        virtual String_t getToken(String_t userId, String_t tokenType);
        virtual Info checkToken(String_t token, afl::base::Optional<String_t> requiredType, bool autoRenew);
        virtual void clearToken(String_t userId, afl::base::Memory<const String_t> tokenTypes);

        void deleteToken(String_t userId, String_t tokenType, String_t token);
        String_t createToken(String_t userId, String_t tokenType, Time_t validUntil);

     private:
        Root& m_root;
    };

} }

#endif
