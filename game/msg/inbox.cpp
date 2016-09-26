/**
  *  \file game/msg/inbox.cpp
  */

#include "game/msg/inbox.hpp"
#include "afl/string/char.hpp"
#include "game/player.hpp"

namespace {
    /** Simplify message header.
        - Remove surrounding angle brackets
        - Remove optional "(n)" produced by PHCc
        - Lowercase all-caps headings */
    String_t tweakHeader(String_t what)
    {
        String_t::size_type n;
        while ((n = what.find_first_of("<>")) != String_t::npos) {
            what.erase(n, 1);
        }
        while (what.size() && what[what.size()-1] == ' ') {
            what.erase(what.size()-1);
        }
        if (what.size() && what[what.size()-1] == ')') {
            // "PHCc game info (1)"
            n = what.rfind('(');
            if (n != String_t::npos && n > 4) {
                what.erase(n);
            }
        }
        return afl::string::strTrim(afl::string::strLCWords(what));
    }
}

/** Message structure. */
struct game::msg::Inbox::Message {
    String_t text;
    int turnNumber;

    Message(const String_t& text, int turnNumber)
        : text(text), turnNumber(turnNumber)
        { }
};

game::msg::Inbox::Inbox()
    : m_messages()
{ }

game::msg::Inbox::~Inbox()
{ }

// Mailbox:
size_t
game::msg::Inbox::getNumMessages()
{
    // ex GInbox::getCount
    return m_messages.size();
}

String_t
game::msg::Inbox::getMessageText(size_t index, afl::string::Translator& /*tx*/, PlayerList& /*players*/)
{
    // ex GInbox::getText
    if (index < m_messages.size()) {
        return m_messages[index]->text;
    } else {
        return String_t();
    }
}

String_t
game::msg::Inbox::getMessageHeading(size_t index, afl::string::Translator& tx, PlayerList& players)
{
    // ex GInbox::getHeading
    // This is the same algorithm as in PCC 1.x.
    // c2ng change: use only one parenized letter.
    String_t line = afl::string::strTrim(afl::string::strFirst(getMessageText(index, tx, players), "\n"));

    // Shortcuts:
    if (line.length() < 5) {
        /* translators: must start with "( )" */
        return tx.translateString("(_) Unknown");
    }
    if (line[0] != '(') {
        /* pre-3.2 message format */
        return "(_) " + tweakHeader(line);
    }

    /* It is a message in our preferred format */
    String_t pre = "(";
    pre += line[2];
    pre += ')';
    switch (afl::string::charToUpper(line[2])) {
     case 'R':
        if (line.size() > 3) {
            if (line[3] == '0') {
                return pre + tx.translateString("Anonymous Message");
            } else if (Player* pl = players.getPlayerFromCharacter(line[3])) {
                return pre + pl->getName(Player::LongName);
            } else {
                // proceed, use default
            }
        }
        break;
     case 'D':
        return pre + tx.translateString("Starbase Message");
     case 'L':
        return pre + tx.translateString("Minefield Laid");
     case 'I':
        return pre + tx.translateString("Ion Storm");
     case 'G':
        return pre + tx.translateString("HConfig");
     case 'M':
        /* Mine scan/sweep. People want to separate these, to be able to filter
           out unsuccessful scans, and only see sweeps. We look for two keyphrases
           to detect sweeps. We only look into the message if we're reasonably
           sure that it is an English message to avoid false positives (hence the
           check for the header line).

           This is the same logic as used in PCC 1.1.16, and it works for English
           and NewEnglish (and, implicitly, for German, which happens to use
           distinct headers for scan and sweep). */
        if (line.find("Sub Space Message") != line.npos) {
            String_t fulltext = getMessageText(index, tx, players);
            if (fulltext.find("ines have been destroyed") != fulltext.npos
                || fulltext.find("is using beam weapons to") != fulltext.npos)
            {
                return pre + tx.translateString("Mine Sweep");
            } else {
                return pre + tx.translateString("Mine Scan");
            }
        }
        /* else use default */
        break;
    }
    afl::string::strRemove(line, ")");
    return pre + tweakHeader(line);
}

// Inquiry:
int
game::msg::Inbox::getMessageTurnNumber(size_t index)
{
    if (index < m_messages.size()) {
        return m_messages[index]->turnNumber;
    } else {
        return 0;
    }
}

// Manipulation
// /** Add a single message.
//     \param str Complete text of message
//     \param receiver Receiver of this message */
void
game::msg::Inbox::addMessage(String_t text, int turnNumber)
{
    // ex GInbox::addMessage
    m_messages.pushBackNew(new Message(text, turnNumber));
}

// /** Sort messages. This will group messages of equal subjects
//     together, but preserves the overall order (i.e. messages from
//     other races will remain first). */
void
game::msg::Inbox::sort(afl::string::Translator& tx, PlayerList& players)
{
    // ex GInbox::sort
    afl::container::PtrVector<Message> newData;
    newData.reserve(m_messages.size());

    /* This algorithm is nominally O(n^2). */
    for (size_t i = 0, n = m_messages.size(); i < n; ++i) {
        if (m_messages[i] != 0) {
            const String_t h = getMessageHeading(i, tx, players);

            // Copy over all messages with this subject.
            // This loop starts at i, so the first thing it copies is message i.
            for (size_t j = i; j < n; ++j) {
                if (m_messages[j] != 0 && getMessageHeading(j, tx, players) == h) {
                    newData.pushBackNew(m_messages.extractElement(j));
                }
            }
        }
    }
    m_messages.swap(newData);
}
