/**
  *  \file game/msg/file.cpp
  *  \brief Message File Access
  */

#include "game/msg/file.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/string/format.hpp"
#include "game/msg/inbox.hpp"
#include "game/msg/mailbox.hpp"
#include "game/parser/binarytransfer.hpp"
#include "util/stringparser.hpp"

using afl::string::Format;

namespace {
    class Reader {
     public:
        Reader(game::msg::Inbox& mbox)
            : m_mailbox(mbox), m_text(), m_turnNumber(), m_inMessage(false)
            { }
        void handleLine(const String_t& line);
        void flush();

     private:
        game::msg::Inbox& m_mailbox;
        std::vector<String_t> m_text;
        int m_turnNumber;
        bool m_inMessage;
    };

    String_t joinText(afl::base::Memory<const String_t> msg)
    {
        String_t result;
        while (const String_t* p = msg.eat()) {
            result += *p;
            result += '\n';
        }
        return result;
    }

    game::Reference findLink(afl::base::Memory<const String_t> text)
    {
        // We only want to identify the object, but we do not want to assimilate the data yet.
        // Thus, a fake environment would be enough.
        // Turn number must be big enough that we do not reject legitimate data as coming from the future.
        // Charset must decode everything successfully.
        const int FAKE_TURN_NR = 30000;
        afl::charset::CodepageCharset fakeCharset(afl::charset::g_codepageLatin1);

        afl::container::PtrVector<game::parser::MessageInformation> info;
        if (game::parser::unpackBinaryMessage(text, FAKE_TURN_NR, info, fakeCharset).first == game::parser::UnpackSuccess) {
            for (size_t i = 0, n = info.size(); i < n; ++i) {
                game::Reference ref = info[i]->getObjectReference();
                if (ref.isSet()) {
                    return ref;
                }
            }
        }
        return game::Reference();
    }
}

void
Reader::handleLine(const String_t& line)
{
    util::StringParser p(line);
    if (p.parseString("=== Turn ")) {
        // Usually, "=== Turn 99 ==="
        flush();
        p.parseInt(m_turnNumber);
        m_inMessage = false;
    } else if (p.parseString("--- Message")) {
        // Can be just "--- Message ---", but could also contain a number and/or file name
        flush();
        m_inMessage = true;
    } else {
        // Might be message content
        if (m_inMessage) {
            if (p.parseString("TURN:")) {
                p.parseInt(m_turnNumber);
            }
            m_text.push_back(afl::string::strRTrim(line));
        }
    }
}

void
Reader::flush()
{
    if (m_inMessage) {
        // Strip trailing blank lines.
        while (!m_text.empty() && m_text.back().empty()) {
            m_text.pop_back();
        }

        // Do not add empty messages
        if (!m_text.empty()) {
            size_t idx = m_mailbox.addMessage(joinText(m_text), m_turnNumber);
            m_mailbox.setMessagePrimaryLink(idx, findLink(m_text));
        }
    }
    m_text.clear();
    m_inMessage = false;
}


/*
 *  Public Interface
 */

void
game::msg::writeMessages(afl::io::TextFile& out, const Mailbox& mbox, size_t first, size_t last, const PlayerList& players, afl::string::Translator& tx)
{
    while (first < last) {
        // Determine section
        const int turnNumber = mbox.getMessageMetadata(first, tx, players).turnNumber;
        size_t split = first+1;
        while (split < last && mbox.getMessageMetadata(split, tx, players).turnNumber == turnNumber) {
            ++split;
        }

        // Write section
        out.writeLine(Format("=== Turn %d ===", turnNumber));
        if (split > first+1) {
            out.writeLine(Format("   %d message(s)", split-first));
        }
        for (size_t i = first; i < split; ++i) {
            out.writeLine(Format("--- Message %d ---", i+1));
            out.writeLine(mbox.getMessageText(i, tx, players));
        }

        // Next section
        first = split;
    }
}

void
game::msg::loadMessages(afl::io::TextFile& in, Inbox& mbox)
{
    // ex filembox.pas:LoadFileMbox
    Reader rdr(mbox);
    String_t line;
    while (in.readLine(line)) {
        rdr.handleLine(line);
    }
    rdr.flush();
}
