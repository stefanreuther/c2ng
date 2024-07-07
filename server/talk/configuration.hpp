/**
  *  \file server/talk/configuration.hpp
  *  \brief Structure server::talk::Configuration
  */
#ifndef C2NG_SERVER_TALK_CONFIGURATION_HPP
#define C2NG_SERVER_TALK_CONFIGURATION_HPP

#include "afl/string/string.hpp"

namespace server { namespace talk {

    /** Service configuration.
        This structure contains "passive" configuration elements. */
    struct Configuration {
        /** Constructor.
            Sets everything to defaults. */
        Configuration();

        /// Message Id suffix. Used to build RfC Message-Ids; must contain "@host" part.
        String_t messageIdSuffix;

        /// Base URL. Used to build URLs when no base URL specified by the command is available.
        String_t baseUrl;

        /// Path host. Hostname of this server to use in RfC "Path" and "Xref" headers.
        String_t pathHost;

        /// Rate limiting: minimum score.
        int32_t rateMinimum;

        /// Rate limiting: maximum score.
        int32_t rateMaximum;

        /// Rate limiting: cooldown per interval.
        int32_t rateCooldown;

        /// Rate limiting: cooldown interval.
        int32_t rateInterval;

        /// Rate limiting: cost per mail receiver.
        int32_t rateCostPerReceiver;

        /// Rate limiting: fixed cost per mail.
        int32_t rateCostPerMail;

        /// Rate limiting: fixed cost per posting.
        int32_t rateCostPerPost;

        /// Maximum number of postings to check for TalkPost::getNewest (POSTLSNEW).
        int32_t getNewestLimit;
    };

} }

#endif
