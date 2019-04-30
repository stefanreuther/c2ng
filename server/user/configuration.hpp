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

        /// User key. Secret seed to encrypt passwords (classic version).
        String_t userKey;

        /// Maximum size of a key in user data.
        size_t userDataMaxKeySize;

        /// Maximum size of a value in user data.
        size_t userDataMaxValueSize;

        /// Maximum total size of user data.
        size_t userDataMaxTotalSize;
    };

} }

#endif
