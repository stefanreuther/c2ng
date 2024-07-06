/**
  *  \file server/talk/ratelimit.cpp
  *  \brief Rate Limiting
  */

#include <algorithm>
#include "server/talk/ratelimit.hpp"
#include "afl/string/format.hpp"
#include "server/talk/configuration.hpp"
#include "server/talk/user.hpp"

namespace {
    using afl::string::Format;
    using afl::sys::LogListener;

    const char*const LOG_NAME = "ratelimit";
}

bool
server::talk::checkRateLimit(int32_t cost, Time_t time, const Configuration& config, User& user, afl::sys::LogListener& log)
{
    // Fetch time and score
    Time_t lastTime = user.rateTime().get();
    int32_t score = 0;
    int32_t elapsed = 1;
    if (lastTime != 0 && lastTime <= time) {
        score = user.rateScore().get();
        elapsed = time - lastTime;
    }

    // Update score
    // Deliberately no handling of fractional time! If someone spams us with messages, this slows cooldown.
    const int32_t cooldown = std::max(int32_t(0), config.rateCooldown);
    const int32_t interval = std::max(int32_t(1), config.rateInterval);
    score -= int32_t(int64_t(elapsed) * cooldown / interval);
    score = std::max(score, config.rateMinimum);
    score += std::max(0, cost);
    score = std::min(score, config.rateMaximum);

    user.rateTime().set(time);
    user.rateScore().set(score);

    // Produce result
    if (score >= config.rateMaximum) {
        // Score exceeded, reject message
        log.write(LogListener::Info, LOG_NAME, Format("user %s: message rejected: cost=%d, score=%d", user.getUserId(), cost, score));
        return false;
    } else {
        // Score passes; log positive values (=users getting towards limit)
        if (score > 0) {
            log.write(LogListener::Info, LOG_NAME, Format("user %s: message accepted: cost=%d, score=%d", user.getUserId(), cost, score));
        }
        return true;
    }
}
