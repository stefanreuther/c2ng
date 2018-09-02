/**
  *  \file game/v3/messagewriter.hpp
  */
#ifndef C2NG_GAME_V3_MESSAGEWRITER_HPP
#define C2NG_GAME_V3_MESSAGEWRITER_HPP

#include "afl/base/memory.hpp"
#include "afl/base/growablememory.hpp"
#include "afl/charset/charset.hpp"
#include "game/msg/outbox.hpp"
#include "afl/string/translator.hpp"
#include "game/playerlist.hpp"

namespace game { namespace v3 {

    class MessageWriter {
     public:
        virtual ~MessageWriter()
            { }

        virtual void sendMessageData(int from, int to, afl::base::ConstBytes_t data) = 0;

        void sendMessage(int from, int to, const String_t& text, afl::charset::Charset& cs);

        void sendOutbox(game::msg::Outbox& outbox, int from, afl::string::Translator& tx, const PlayerList& players, afl::charset::Charset& cs);

        static afl::base::GrowableBytes_t encodeMessage(const String_t& text, afl::charset::Charset& cs);
    };

} }

#endif
