/**
  *  \file game/msg/format.hpp
  *  \brief Message Formatting
  */
#ifndef C2NG_GAME_MSG_FORMAT_HPP
#define C2NG_GAME_MSG_FORMAT_HPP

#include "afl/string/translator.hpp"
#include "game/playerlist.hpp"
#include "game/playerset.hpp"
#include "game/reference.hpp"
#include "util/rich/text.hpp"

namespace game { namespace msg {

    /** Formatted message. */
    struct Format {
        /** Reference for the message's first link. */
        Reference firstLink;

        /** Receivers for "reply" function. */
        PlayerSet_t reply;

        /** Receivers for "reply all" function. */
        PlayerSet_t replyAll;

        /** Formatted message.
            Clickable coordinates are replaced by links whose target contains an X,Y pair. */
        util::rich::Text text;
    };

    /** Format a message for display, extracting information.

        @param in       Message text (multi-line string)
        @param players  List of players (for names, sets)
        @param tx       Translator (indirectly required for player names)
        @return formatted message */
    Format formatMessage(const String_t& in, const PlayerList& players, afl::string::Translator& tx);

    /** Quote message for reply.
        Assumes the usual v3 message format including a "(-r)<<< >>>" header, and FROM:, TO: headers.
        Removes the headers and superfluous empty lines, and prepends ">".
        @param originalText   Message text (multi-line string)
        @return quoted message (multi-line string) */
    String_t quoteMessageForReply(const String_t& originalText);

} }

#endif
