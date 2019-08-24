/**
  *  \file server/user/configuration.hpp
  *  \brief Structure server::user::Configuration
  */
#ifndef C2NG_SERVER_USER_CONFIGURATION_HPP
#define C2NG_SERVER_USER_CONFIGURATION_HPP

#include "afl/string/string.hpp"

namespace server { namespace user {

    /** Service configuration.
        This structure contains "passive" configuration elements. */
    struct Configuration {
        /** Constructor.
            Sets everything to defaults. */
        Configuration();

        /** User key. Secret seed to encrypt passwords (classic version, ClassicEncrypter). */
        String_t userKey;

        /** Maximum size of a key in user data (UserData interface). */
        size_t userDataMaxKeySize;

        /** Maximum size of a value in user data (UserData interface). */
        size_t userDataMaxValueSize;

        /** Maximum total size of user data (UserData interface). */
        size_t userDataMaxTotalSize;

        /** Maximum size of a value in the profile (UserManagement::setProfile).
            For the profile, we trust the keys and only limit individual values.
            Most values are also trusted, but some can be provided by the user (e.g. identification information),
            so this is an additional safeguard to avoid overloading the database. */
        size_t profileMaxValueSize;
    };

} }

#endif
