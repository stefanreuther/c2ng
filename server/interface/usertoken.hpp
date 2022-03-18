/**
  *  \file server/interface/usertoken.hpp
  *  \brief Interface server::interface::UserToken
  */
#ifndef C2NG_SERVER_INTERFACE_USERTOKEN_HPP
#define C2NG_SERVER_INTERFACE_USERTOKEN_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/memory.hpp"
#include "afl/base/optional.hpp"
#include "afl/string/string.hpp"

namespace server { namespace interface {

    /** User token server interface.
        This interface allows creation, verification, and invalidation of tokens. */
    class UserToken : public afl::base::Deletable {
     public:
        /** Information about a token. */
        struct Info {
            /** Owner User Id. */
            String_t userId;

            /** Type of token. */
            String_t tokenType;

            /** New token.
                The current token is valid but about to expire; caller shall use new token in future requests. */
            afl::base::Optional<String_t> newToken;
        };

        /** Get or create a token (MAKETOKEN).
            \param userId User Id
            \param tokenType Token type
            \return Token */
        virtual String_t getToken(String_t userId, String_t tokenType) = 0;

        /** Check token (CHECKTOKEN).
            \param token Token
            \param requiredType Match only this type
            \param autoRenew Automatically renew token if it is short before expiring (set newToken). */
        virtual Info checkToken(String_t token, afl::base::Optional<String_t> requiredType, bool autoRenew) = 0;

        /** Clear tokens (RESETTOKEN).
            \param userId User Id
            \param tokenTypes Token types to clear */
        virtual void clearToken(String_t userId, afl::base::Memory<const String_t> tokenTypes) = 0;
    };

} }

#endif
