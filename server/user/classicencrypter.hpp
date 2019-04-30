/**
  *  \file server/user/classicencrypter.hpp
  *  \brief Class server::user::ClassicEncrypter
  */
#ifndef C2NG_SERVER_USER_CLASSICENCRYPTER_HPP
#define C2NG_SERVER_USER_CLASSICENCRYPTER_HPP

#include "server/user/passwordencrypter.hpp"

namespace server { namespace user {

    /** Classic (un-secure) password encrypter.
        Password are hashed with a system-wide key.
        This is PlanetsCentral's original password scheme. */
    class ClassicEncrypter : public PasswordEncrypter {
     public:
        /** Constructor.
            \param userKey System-wide key */
        ClassicEncrypter(String_t userKey);

        // PasswordEncrypter:
        virtual String_t encryptPassword(String_t password, String_t userId);
        virtual Result checkPassword(String_t password, String_t hash, String_t userId);

     private:
        String_t m_userKey;
    };

} }

#endif
