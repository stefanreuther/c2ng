/**
  *  \file game/v3/writer.cpp
  */

#include "game/v3/writer.hpp"
#include "game/v3/structures.hpp"
#include "game/v3/messagewriter.hpp"
#include "afl/base/growablememory.hpp"

namespace gt = game::v3::structures;
using afl::base::fromObject;

namespace {
    class MessageCounter : public game::v3::MessageWriter {
     public:
        MessageCounter()
            : MessageWriter(), m_numMessages(0)
            { }
        virtual void sendMessageData(int /*from*/, int /*to*/, afl::base::ConstBytes_t /*data*/)
            { ++m_numMessages; }
        size_t getNumMessages()
            { return m_numMessages; }
     private:
        size_t m_numMessages;
    };

    class MessageStorer : public game::v3::MessageWriter {
     public:
        MessageStorer(afl::io::Stream& file,
                      afl::base::Memory<gt::OutgoingMessageHeader> header,
                      afl::io::Stream::FileSize_t pos)
            : m_file(file),
              m_header(header),
              m_pos(pos)
            { }
        virtual void sendMessageData(int from, int to, afl::base::ConstBytes_t data)
            {
                if (gt::OutgoingMessageHeader* pHeader = m_header.eat()) {
                    pHeader->address = static_cast<int32_t>(m_pos);
                    pHeader->length  = static_cast<int16_t>(data.size());
                    pHeader->from    = static_cast<int16_t>(from);
                    pHeader->to      = static_cast<int16_t>(to);
                    m_file.fullWrite(data);
                    m_pos += data.size();
                }
            }
     private:
        afl::io::Stream& m_file;
        afl::base::Memory<gt::OutgoingMessageHeader> m_header;
        afl::io::Stream::FileSize_t m_pos;
    };
}

game::v3::Writer::Writer(afl::charset::Charset& charset, afl::string::Translator& tx, afl::sys::LogListener& log)
    : m_charset(charset),
      m_translator(tx),
      m_log(log)
{ }

void
game::v3::Writer::saveOutbox(game::msg::Outbox& outbox, int player, const PlayerList& players, afl::io::Stream& file)
{
    // ex saveOutbox
    // Count messages
    // Ptr<Stream> f = dir.openFile(format("mess%d.dat", player), Stream::C_CREATE);

    MessageCounter counter;
    counter.sendOutbox(outbox, player, m_translator, players, m_charset);

    // Quick exit for 0-message case
    if (counter.getNumMessages() == 0) {
        gt::Int16_t zero;
        zero = 0;
        file.fullWrite(zero.m_bytes);
        return;
    }

    // FIXME: some kind of overflow check here?

    /* Allocate buffer for message headers. We leave room for at least
       50 messages after those we wrote, but at least 150 in total.
       This is to help programs that don't rewrite the message file
       from scratch. PLANETS.EXE has a fixed limit of 50 here; VPA
       seems to relocate messages dynamically when the directory grows
       too large. */
    size_t nslots = counter.getNumMessages() + 50;
    if (nslots < 150) {
        nslots = 150;
    }

    afl::io::Stream::FileSize_t startPos = file.getPos();

    gt::Int16_t rawCount;
    rawCount = 0;
    file.fullWrite(rawCount.m_bytes);

    afl::base::GrowableMemory<gt::OutgoingMessageHeader> headers;
    headers.resize(nslots);
    headers.toBytes().fill(0);
    file.fullWrite(headers.toBytes());

    // Write messages
    MessageStorer(file, headers, startPos + headers.toBytes().size() + 2 + 1).sendOutbox(outbox, player, m_translator, players, m_charset);

    // Update header
    afl::io::Stream::FileSize_t endPos = file.getPos();
    file.setPos(startPos);

    rawCount = static_cast<int16_t>(counter.getNumMessages());
    file.fullWrite(rawCount.m_bytes);
    file.fullWrite(headers.toBytes());

    file.setPos(endPos);
}

void
game::v3::Writer::saveOutbox35(game::msg::Outbox& outbox, int player, afl::io::Stream& file)
{
    // ex saveOutbox35
    // Ptr<Stream> f = dir.openFile(format("mess35%d.dat", player), Stream::C_CREATE);

    // Count messages
    int numMessages = 0;
    for (size_t i = 0, n = outbox.getNumMessages(); i < n; ++i) {
        if (outbox.getMessageSender(i) == player) {
            ++numMessages;
        }
    }

    // Quick exit for 0-message case
    if (numMessages == 0) {
        gt::Int16_t zero;
        zero = 0;
        file.fullWrite(zero.m_bytes);
        return;
    }

    // Header
    gt::Outbox35FileHeader fileHeader;
    fileHeader.numMessages = static_cast<uint16_t>(numMessages);
    fromObject(fileHeader.pad).fill(0);
    file.fullWrite(fromObject(fileHeader));

    // Content
    for (size_t i = 0, n = outbox.getNumMessages(); i < n; ++i) {
        if (outbox.getMessageSender(i) == player) {
            /* FIXME: Winplan has a limit of 600 characters. PCC 1.x allows
               to exceed that limit if Winplan is not in use. Right now,
               we enforce the limit. */
            // Encode
            afl::base::GrowableBytes_t rawText = MessageWriter::encodeMessage(outbox.getMessageRawText(i), m_charset);

            // Convert to CRLF format
            afl::base::GrowableBytes_t text;
            for (const uint8_t* p = rawText.begin(); p != rawText.end(); ++p) {
                if (*p == 23) {
                    // ignore CR
                } else if (*p == 26) {
                    text.append(*p);
                    text.append(23);
                } else {
                    text.append(*p);
                }
            }

            // Enforce size
            const size_t LIMIT = 600;
            if (text.size() < LIMIT) {
                text.appendN(32+13, LIMIT - text.size());
            } else {
                text.trim(LIMIT);
            }

            // Build header
            PlayerSet_t receivers = outbox.getMessageReceivers(i);
            gt::Outbox35MessageHeader msgHeader;
            msgHeader.pad = 0;
            msgHeader.validFlag = '1';
            msgHeader.messageLength = LIMIT;
            for (int i = 1; i <= gt::NUM_OWNERS; ++i) {
                msgHeader.receivers[i-1] = (i == gt::NUM_OWNERS ? receivers.contains(0) : receivers.contains(i)) ? '1' : '0';
            }
            file.fullWrite(fromObject(msgHeader));
            file.fullWrite(text);
        }
    }
}
