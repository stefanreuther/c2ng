/**
  *  \file game/msg/inbox.cpp
  */

#include "game/msg/inbox.hpp"
#include "afl/string/char.hpp"
#include "game/msg/format.hpp"
#include "game/parser/binarytransfer.hpp"
#include "game/parser/messagetemplate.hpp"
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
    Mailbox::DataStatus dataStatus;

    Message(const String_t& text, int turnNumber)
        : text(text), turnNumber(turnNumber), dataStatus(NoData)
        { }
};

game::msg::Inbox::Inbox()
    : m_messages()
{ }

game::msg::Inbox::~Inbox()
{ }

// Mailbox:
size_t
game::msg::Inbox::getNumMessages() const
{
    // ex GInbox::getCount
    return m_messages.size();
}

String_t
game::msg::Inbox::getMessageHeaderText(size_t /*index*/, afl::string::Translator& /*tx*/, const PlayerList& /*players*/) const
{
    // No headers
    return String_t();
}

String_t
game::msg::Inbox::getMessageBodyText(size_t index, afl::string::Translator& /*tx*/, const PlayerList& /*players*/) const
{
    // ex GInbox::getText
    if (const Message* p = getMessage(index)) {
        return p->text;
    } else {
        return String_t();
    }
}

String_t
game::msg::Inbox::getMessageForwardText(size_t index, afl::string::Translator& tx, const PlayerList& players) const
{
    return defaultGetMessageForwardText(index, tx, players);
}

String_t
game::msg::Inbox::getMessageReplyText(size_t index, afl::string::Translator& tx, const PlayerList& players) const
{
    return defaultGetMessageReplyText(index, tx, players);
}

util::rich::Text
game::msg::Inbox::getMessageDisplayText(size_t index, afl::string::Translator& tx, const PlayerList& players) const
{
    if (const Message* p = getMessage(index)) {
        return defaultGetMessageDisplayText(p->text, p->dataStatus, tx, players);
    } else {
        return util::rich::Text();
    }
}

String_t
game::msg::Inbox::getMessageHeading(size_t index, afl::string::Translator& tx, const PlayerList& players) const
{
    // ex GInbox::getHeading, sendmsg.pas:MessageSubject
    // This is the same algorithm as in PCC 1.x.
    // c2ng change: use only one parenized letter.
    String_t line = afl::string::strTrim(afl::string::strFirst(getMessageBodyText(index, tx, players), "\n"));

    // Shortcuts:
    if (line.length() < 5) {
        /* translators: must start with "( )" */
        return tx("(_) Unknown");
    }
    if (line[0] != '(') {
        /* pre-3.2 message format */
        return "(_) " + tweakHeader(line);
    }

    /* It is a message in our preferred format */
    String_t pre = "(";
    pre += line[2];
    pre += ") ";
    switch (afl::string::charToUpper(line[2])) {
     case 'R':
        if (line.size() > 3) {
            if (line[3] == '0') {
                return pre + tx("Anonymous Message");
            } else if (Player* pl = players.getPlayerFromCharacter(line[3])) {
                return pre + pl->getName(Player::LongName, tx);
            } else {
                // proceed, use default
            }
        }
        break;
     case 'D':
        return pre + tx("Starbase Message");
     case 'L':
        return pre + tx("Minefield Laid");
     case 'I':
        return pre + tx("Ion Storm");
     case 'G':
        return pre + tx("HConfig");
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
            String_t fulltext = getMessageBodyText(index, tx, players);
            if (fulltext.find("ines have been destroyed") != fulltext.npos
                || fulltext.find("is using beam weapons to") != fulltext.npos)
            {
                return pre + tx("Mine Sweep");
            } else {
                return pre + tx("Mine Scan");
            }
        }
        /* else use default */
        break;
    }
    afl::string::strRemove(line, ")");
    return pre + tweakHeader(line);
}

// Inquiry:
game::msg::Mailbox::Metadata
game::msg::Inbox::getMessageMetadata(size_t index, afl::string::Translator& tx, const PlayerList& players) const
{
    Metadata md;
    if (const Message* p = getMessage(index)) {
        const Format fmt = formatMessage(p->text, players, tx);
        md.turnNumber    = p->turnNumber;
        md.dataStatus    = p->dataStatus;
        md.secondaryLink = fmt.firstLink;
        md.reply         = fmt.reply;
        md.replyAll      = fmt.replyAll;
    }
    return md;
}

game::msg::Mailbox::Actions_t
game::msg::Inbox::getMessageActions(size_t /*index*/) const
{
    return Actions_t();
}

void
game::msg::Inbox::performMessageAction(size_t /*index*/, Action /*a*/)
{ }

void
game::msg::Inbox::receiveMessageData(size_t index, game::parser::InformationConsumer& consumer, const TeamSettings& teamSettings, bool onRequest, afl::charset::Charset& cs)
{
    if (Message* p = getMessage(index)) {
        p->dataStatus = defaultReceiveMessageData(p->text, p->turnNumber-1, consumer, teamSettings, onRequest, cs);
    }
}

// Manipulation
void
game::msg::Inbox::addMessage(String_t text, int turnNumber)
{
    // ex GInbox::addMessage
    m_messages.pushBackNew(new Message(text, turnNumber));
}

void
game::msg::Inbox::sort(afl::string::Translator& tx, const PlayerList& players)
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

game::msg::Inbox::Message*
game::msg::Inbox::getMessage(size_t index)
{
    if (index < m_messages.size()) {
        return m_messages[index];
    } else {
        return 0;
    }
}

const game::msg::Inbox::Message*
game::msg::Inbox::getMessage(size_t index) const
{
    return const_cast<Inbox*>(this)->getMessage(index);
}
