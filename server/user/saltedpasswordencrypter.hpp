/**
  *  \file server/user/saltedpasswordencrypter.hpp
  *  \brief Class server::user::SaltedPasswordEncrypter
  */
#ifndef C2NG_SERVER_USER_SALTEDPASSWORDENCRYPTER_HPP
#define C2NG_SERVER_USER_SALTEDPASSWORDENCRYPTER_HPP

#include "server/user/passwordencrypter.hpp"
#include "server/common/idgenerator.hpp"

namespace server { namespace user {

    /** Salted password encrypter.
        Generates password hashes using a (possibly cryptographically secure) salt.
        This should be more secure than the ClassicEncrypter that relies on a single
        system-specific salt only. */
    class SaltedPasswordEncrypter : public PasswordEncrypter {
     public:
        /** Constructor.
            \param saltGenerator IdGenerator to generate salts */
        SaltedPasswordEncrypter(server::common::IdGenerator& saltGenerator);

        // PasswordEncrypter:
        virtual String_t encryptPassword(String_t password, String_t userId);
        virtual Result checkPassword(String_t password, String_t hash, String_t userId);

     private:
        server::common::IdGenerator& m_saltGenerator;
    };

} }

#endif
