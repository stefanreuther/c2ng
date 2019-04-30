/**
  *  \file server/interface/usertoken.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_USERTOKEN_HPP
#define C2NG_SERVER_INTERFACE_USERTOKEN_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/memory.hpp"
#include "afl/base/optional.hpp"
#include "afl/string/string.hpp"

namespace server { namespace interface {

    class UserToken : public afl::base::Deletable {
     public:
        struct Info {
            String_t userId;
            String_t tokenType;
            afl::base::Optional<String_t> newToken;
        };

        /** Get or create a token (MAKETOKEN).
            \param userId User Id
            \param tokenType Token type
            \return Token */
        virtual String_t getToken(String_t userId, String_t tokenType) = 0;

        /** Check token.
            \param token Token
            \param requiredType Match only this type
            \param autoRenew Automatically renew token if it is short before expiring (set newToken). */
        virtual Info checkToken(String_t token, afl::base::Optional<String_t> requiredType, bool autoRenew) = 0;

        /** Clear tokens.
            \param userId User Id
            \param tokenType Token types to clear */
        virtual void clearToken(String_t userId, afl::base::Memory<const String_t> tokenTypes) = 0;
    };

} }

#endif
