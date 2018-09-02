/**
  *  \file game/v3/writer.hpp
  */
#ifndef C2NG_GAME_V3_WRITER_HPP
#define C2NG_GAME_V3_WRITER_HPP

#include "afl/charset/charset.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/msg/outbox.hpp"
#include "afl/io/stream.hpp"
#include "game/playerlist.hpp"

namespace game { namespace v3 {

    class Writer {
     public:
        Writer(afl::charset::Charset& charset, afl::string::Translator& tx, afl::sys::LogListener& log);

        void saveOutbox(game::msg::Outbox& outbox, int player, const PlayerList& players, afl::io::Stream& file);
        void saveOutbox35(game::msg::Outbox& outbox, int player, afl::io::Stream& file);

     private:
        afl::charset::Charset& m_charset;
        afl::string::Translator& m_translator;
        afl::sys::LogListener& m_log;
    };

} }

#endif
