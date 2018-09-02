/**
  *  \file game/v3/messagewriter.cpp
  */

#include "game/v3/messagewriter.hpp"
#include "game/v3/structures.hpp"

void
game::v3::MessageWriter::sendMessage(int from, int to, const String_t& text, afl::charset::Charset& cs)
{
    sendMessageData(from, to, encodeMessage(text, cs));
}

void
game::v3::MessageWriter::sendOutbox(game::msg::Outbox& outbox, int from, afl::string::Translator& tx, const PlayerList& players, afl::charset::Charset& cs)
{
    for (size_t i = 0, n = outbox.getNumMessages(); i < n; ++i) {
        if (outbox.getMessageSender(i) == from) {
            PlayerSet_t rec = outbox.getMessageReceiverMask(i);
            for (int p = 0; p <= game::v3::structures::NUM_PLAYERS; ++p) {
                if (rec.contains(p)) {
                    sendMessage(from, p, outbox.getMessageSendPrefix(i, p, tx, players) + outbox.getMessageRawText(i), cs);
                }
            }
        }
    }
}

afl::base::GrowableBytes_t
game::v3::MessageWriter::encodeMessage(const String_t& text, afl::charset::Charset& cs)
{
    // ex game/msg-out.cc:encodeMessage
    // Convert to game character set
    afl::base::GrowableBytes_t result = cs.encode(afl::string::toMemory(text));

    // Encrypt
    afl::base::Bytes_t it = result;
    while (uint8_t* p = it.eat()) {
        if (*p == '\n') {
            *p = 26;
        } else {
            *p = static_cast<int8_t>(*p + 13U);
        }
    }

    // terminate with a carriage return to avoid that people make fun of Akseli :-)
    if (const uint8_t* p = result.atEnd(0)) {
        if (*p != 26) {
            result.append(26);
        }
    }
    return result;
}
