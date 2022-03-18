/**
  *  \file server/talk/spam.hpp
  */
#ifndef C2NG_SERVER_TALK_SPAM_HPP
#define C2NG_SERVER_TALK_SPAM_HPP

#include "server/types.hpp"
#include "afl/string/string.hpp"
#include "server/talk/user.hpp"
#include "afl/sys/loglistener.hpp"

namespace server { namespace talk {

    class InlineRecognizer;

    /** Check for spam.
        \param subject post subject
        \param text post text
        \param time time of post
        \param u posting user
        \param recog Link recognizer
        \param log Logger
        \return true if this posting is spam */
    bool checkSpam(const String_t& subject, const String_t& text, Time_t time, User& u, const InlineRecognizer& recog, afl::sys::LogListener& log);

} }


#endif
