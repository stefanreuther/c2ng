/**
  *  \file server/talk/ratelimit.hpp
  *  \brief Rate Limiting
  */
#ifndef C2NG_SERVER_TALK_RATELIMIT_HPP
#define C2NG_SERVER_TALK_RATELIMIT_HPP

#include "afl/sys/loglistener.hpp"
#include "server/types.hpp"

namespace server { namespace talk {

    class Configuration;
    class User;

    /** Check rate limit.

        Updates user's rate limit tracking and returns message status.

        @param cost Cost of this message
        @param time Time
        @param config Server configuration
        @param user   User
        @param log    Logger

        @return true if message passes check, false if message should be rejected */
    bool checkRateLimit(int32_t cost, Time_t time, const Configuration& config, User& user, afl::sys::LogListener& log);

} }

#endif
