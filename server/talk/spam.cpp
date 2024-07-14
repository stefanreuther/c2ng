/**
  *  \file server/talk/spam.cpp
  *  \brief Anti-Spam
  */

#include "server/talk/spam.hpp"
#include "afl/string/string.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "server/talk/inlinerecognizer.hpp"
#include "afl/string/format.hpp"

using server::talk::InlineRecognizer;

namespace {
    const char SPAM_USER_PROFILE[] = "spam";

    const char LOG_NAME[] = "spam";

    size_t countLinks(const InlineRecognizer& recog, const String_t& text)
    {
        String_t::size_type count = 0;
        String_t::size_type pos = 0;
        InlineRecognizer::Info info;
        while (recog.find(text, pos, InlineRecognizer::Kinds_t(InlineRecognizer::Link), info)) {
            if (info.length == 0 || info.start < pos) {
                // cannot happen; avoid deadlock if it does anyway
                break;
            }
            pos = info.start + info.length;
            ++count;
        }
        return count;
    }
}

bool
server::talk::checkSpam(const String_t& /*subject*/,
                        const String_t& text,
                        Time_t time,
                        User& u,
                        const InlineRecognizer& recog,
                        afl::sys::LogListener& log)
{
    // User check
    // SPAM_USER_PROFILE can be
    // - not present (=default behaviour)
    // - 0=user is excempt from spam check but does not see spam
    // - 1=user is a spammer
    // - 2=user is excempt from spam check and can see spam

    afl::net::redis::HashKey profile = u.profile();
    if (profile.intField(SPAM_USER_PROFILE).exists()) {
        if (profile.intField(SPAM_USER_PROFILE).get() == 1) {
            // recognized spammer
            log.write(afl::sys::LogListener::Info, LOG_NAME, "marking posting as spam because user is a spammer");
            return true;
        } else {
            // user is excempt from spam check
            return false;
        }
    }

    // Compute score
    int score = 0;

    // - Chinese language
    // Matches 22/22 spams, 8/8 spammers, with no false positives.
    // Question is how racist we want to be.
    if (afl::string::strUCase(profile.stringField("createacceptlanguage").get()).substr(0, 2) == "ZH") {
        score += 20;
    }

    // - Freemail provider.
    // Matches 22/22 spams, 8/8 spammers, but @hotmail has some false positives.
    String_t mail(afl::string::strLCase(profile.stringField("email").get()));
    if (mail.find("@outlook") != String_t::npos || mail.find("@hotmail") != String_t::npos) {
        score += 10;
    }

    // - Post age
    // 8/8 spammers send their first spam within 15 minutes, 6/8 within 5 minutes.
    Time_t age = time - profile.intField("createtime").get() / 60;
    if (age <= 5) {
        score += 5;
    }
    if (age <= 15) {
        score += 5;
    }
    if (age <= 60) {
        score += 5;
    }

    // - Post size
    // 22/22 spams are over 3k, which is pretty large for a user's initial posting.
    // Use 2.5k for some margin.
    if (text.size() > 2500) {
        score += 10;
    }

    // - Links
    // 20/22 spams have 3 or more links.
    if (countLinks(recog, text) >= 3) {
        score += 20;
    }

    // Recorded spams yield values between 55 and 75 here.
    // Set the margin at 65 for now.
    if (score >= 65) {
        // This is spam.
        log.write(afl::sys::LogListener::Info, LOG_NAME, afl::string::Format("marking post+user as spam, score is %d", score));
        profile.intField(SPAM_USER_PROFILE).set(1);
        return true;
    } else if (score >= 20) {
        // Log weak candidates, just for analysis
        log.write(afl::sys::LogListener::Info, LOG_NAME, afl::string::Format("score is %d", score));
        return false;
    } else {
        // No indication of spam.
        return false;
    }
}
