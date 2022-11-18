/**
  *  \file game/msg/file.hpp
  *  \brief Message File Access
  */
#ifndef C2NG_GAME_MSG_FILE_HPP
#define C2NG_GAME_MSG_FILE_HPP

#include "afl/io/textfile.hpp"
#include "game/playerlist.hpp"
#include "afl/string/translator.hpp"

namespace game { namespace msg {

    class Mailbox;
    class Inbox;

    /** Write messages to file.
        @param [out] out      File
        @param [in]  mbox     Mailbox
        @param [in]  first    First message index, inclusive
        @param [in]  last     Last message index, exclusive
        @param [in]  players  Player list (for message headers)
        @param [in]  tx       Translator (for message headers) */
    void writeMessages(afl::io::TextFile& out, const Mailbox& mbox, size_t first, size_t last, const PlayerList& players, afl::string::Translator& tx);

    /** Load messages from file.
        @param [in]  in       File
        @param [out] mbox     Mailbox */
    void loadMessages(afl::io::TextFile& in, Inbox& mbox);

} }

#endif
