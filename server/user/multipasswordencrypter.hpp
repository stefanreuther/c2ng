/**
  *  \file server/user/multipasswordencrypter.hpp
  *  \brief Class server::user::MultiPasswordEncrypter
  */
#ifndef C2NG_SERVER_USER_MULTIPASSWORDENCRYPTER_HPP
#define C2NG_SERVER_USER_MULTIPASSWORDENCRYPTER_HPP

#include "server/user/passwordencrypter.hpp"

namespace server { namespace user {

    /** Alternative between two password encrypters.
        Wraps two password encrypters.
        Normally, passwords are encrypted using the primary encrypter.
        If the secondary encrypter recognizes a password, it is still accepted but should be re-encrypted (ValidNeedUpdate). */
    class MultiPasswordEncrypter : public PasswordEncrypter {
     public:
        /** Constructor.
            \param primary Primary encrypter. Needs to live longer than MultiPasswordEncrypter.
            \param secondary Secondary encrypter. Needs to live longer than MultiPasswordEncrypter. */
        MultiPasswordEncrypter(PasswordEncrypter& primary, PasswordEncrypter& secondary);

        // PasswordEncrypter:
        virtual String_t encryptPassword(String_t password, String_t userId);
        virtual Result checkPassword(String_t password, String_t hash, String_t userId);

     private:
        PasswordEncrypter& m_primary;
        PasswordEncrypter& m_secondary;
    };

} }

#endif
