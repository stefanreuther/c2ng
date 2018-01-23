/**
  *  \file server/mailout/configuration.hpp
  *  \brief Structure server::mailout::Configuration
  */
#ifndef C2NG_SERVER_MAILOUT_CONFIGURATION_HPP
#define C2NG_SERVER_MAILOUT_CONFIGURATION_HPP

#include "afl/string/string.hpp"

namespace server { namespace mailout {

    /** Mailout configuration.
        This structure contains passive configuration elements. */
    struct Configuration {
        /** Default constructor. */
        Configuration();

        /// Base URL. Used to build URLs; must end in a slash.
        String_t baseUrl;         // ex web_root

        /// Confirmation key. Secret key to generate confirmation tokens.
        String_t confirmationKey; // ex system_key

        /// Maximum message age in minutes. A message is discarded when it exceeds this age.
        int32_t maximumAge;       // ex max_msg_age

        /// Transmitter configuration. Mailout can be run without a transmitter.
        bool useTransmitter;
    };

} }

#endif
