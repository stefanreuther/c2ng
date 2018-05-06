/**
  *  \file game/v3/outboxreader.cpp
  *  \brief Class game::v3::OutboxReader
  */

#include "game/v3/outboxreader.hpp"
#include "afl/base/growablememory.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/except/fileformatexception.hpp"
#include "game/limits.hpp"
#include "game/v3/inboxfile.hpp"
#include "game/v3/structures.hpp"

// Load version 3.0 outbox (MESSx.DAT).
void
game::v3::OutboxReader::loadOutbox(afl::io::Stream& s, afl::charset::Charset& cs, afl::string::Translator& tx)
{
    // ex GOutboxReader::loadOutbox
    // Read count. It is not an error if the count cannot be read (0-byte file).
    structures::Int16_t n;
    if (s.read(afl::base::fromObject(n)) < sizeof(n)) {
        return;
    }
    if (n <= 0) {
        return;
    }

    // Read directory
    afl::base::GrowableMemory<structures::OutgoingMessageHeader> headerBuffer;
    headerBuffer.resize(n);
    s.fullRead(headerBuffer.toBytes());

    // Read content
    afl::base::Memory<const structures::OutgoingMessageHeader> headerReader(headerBuffer);
    while (const structures::OutgoingMessageHeader* p = headerReader.eat()) {
        const int32_t address = p->address;
        const int16_t length = p->length;
        const int to = p->to;
        if (address <= 0) {
            throw afl::except::FileFormatException(s, tx.translateString("Invalid message directory"));
        }
        if (length < 0 || length > structures::MAX_MESSAGE_SIZE) {
            throw afl::except::FileFormatException(s, tx.translateString("Message too big"));
        }

        // Convert receiver. In the file, 12 means Host; internally, 0 means Host.
        int effectiveReceiver;
        if (to == structures::NUM_OWNERS) {
            effectiveReceiver = 0;
        } else if (to > 0 && to <= structures::NUM_PLAYERS) {
            effectiveReceiver = to;
        } else {
            throw afl::except::FileFormatException(s, tx.translateString("Invalid message receiver"));
        }

        if (length != 0) {
            s.setPos(address-1);

            afl::base::GrowableBytes_t messageText;
            messageText.resize(length);
            s.fullRead(messageText);

            addMessage(decodeMessage(messageText, cs, false), PlayerSet_t(effectiveReceiver));
        }
    }
}

// Load version 3.5 outbox (MESS35x.DAT).
void
game::v3::OutboxReader::loadOutbox35(afl::io::Stream& s, afl::charset::Charset& cs, afl::string::Translator& tx)
{
    // Read count. It is not an error if the count cannot be read (0-byte file).
    structures::Outbox35FileHeader fileHeader;
    if (s.read(afl::base::fromObject(fileHeader)) < sizeof(fileHeader)) {
        return;
    }
    if (fileHeader.numMessages <= 0) {
        return;
    }

    // Read messages.
    for (int i = 0, n = fileHeader.numMessages; i < n; ++i) {
        // Read header. We allow the file to be truncated.
        structures::Outbox35MessageHeader hdr;
        if (s.read(afl::base::fromObject(hdr)) < sizeof(hdr)) {
            break;
        }
        const int size = hdr.messageLength;

        // Verify
        if (size < 0 || size > structures::MAX_MESSAGE_SIZE) {
            throw afl::except::FileFormatException(s, tx.translateString("Message too big"));
        }

        // Read message
        if (size > 0) {
            afl::base::GrowableBytes_t messageText;
            messageText.resize(size);
            s.fullRead(messageText);

            if (hdr.validFlag == '1') {
                // Message is valid. Gather receivers.
                PlayerSet_t receivers;
                for (int i = 1; i <= structures::NUM_PLAYERS; ++i) {
                    if (hdr.receivers[i-1] == '1') {
                        receivers += i;
                    }
                }
                // Host
                if (hdr.receivers[structures::NUM_OWNERS-1] == '1') {
                    receivers += 0;
                }
                addMessage(decodeMessage(messageText, cs, false), receivers);
            }
        }
    }
}
