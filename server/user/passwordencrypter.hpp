/**
  *  \file server/user/passwordencrypter.hpp
  *  \brief Interface server::user::PasswordEncrypter
  */
#ifndef C2NG_SERVER_USER_PASSWORDENCRYPTER_HPP
#define C2NG_SERVER_USER_PASSWORDENCRYPTER_HPP

#include "afl/base/deletable.hpp"
#include "afl/string/string.hpp"

namespace server { namespace user {

    /** Interface for a password encryption algorithm. */
    class PasswordEncrypter : public afl::base::Deletable {
     public:
        /** Password checking result. */
        enum Result {
            /** Password is invalid. Do not allow user to log-in. */
            Invalid,

            /** Password is valid. Allow user to log-in, no further action needed. */
            ValidCurrent,

            /** Password is valid, but uses an obsolete encoding.
                Allow user to log-in, but re-hash the password by calling encryptPassword. */
            ValidNeedUpdate
        };

        /** Encrypt a password.
            \param password Password
            \param userId User Id (can be used for salting) */
        virtual String_t encryptPassword(String_t password, String_t userId) = 0;

        /** Check password.
            \param password Password received from user
            \param hash Hashed pasword
            \param userId User Id (can be used for salting)
            \return Result */
        virtual Result checkPassword(String_t password, String_t hash, String_t userId) = 0;
    };

} }

#endif
