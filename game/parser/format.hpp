/**
  *  \file game/parser/format.hpp
  */
#ifndef C2NG_GAME_PARSER_FORMAT_HPP
#define C2NG_GAME_PARSER_FORMAT_HPP

#include "game/reference.hpp"
#include "util/rich/text.hpp"
#include "game/playerset.hpp"
#include "game/playerlist.hpp"

namespace game { namespace parser {

    struct Format {
        Reference firstLink;

        PlayerSet_t reply;
        PlayerSet_t replyAll;

        util::rich::Text text;
    };

    /** Format a message for display, extracting information. */
    void formatMessage(Format& out, const String_t& in, const PlayerList& players);

} }

#endif
