/**
  *  \file game/msg/outbox.cpp
  *  \brief Class game::msg::Outbox
  */

#include "game/msg/outbox.hpp"
#include "afl/string/format.hpp"
#include "game/limits.hpp"

namespace {
    /* Header line for a universal message.
       Used/recognized by other programs, don't translate. */
    const char UNIVERSAL_TEXT[] = "  <<< Universal Message >>>";

    /* Header line for a message to ourselves and others.
       Starts with a '<' to avoid PHost recognizing it as a command message.
       We need to filter it out upon reception. */
    const char CC_SELF_PREFIX[] = "<CC: ";

    /* Header line for a message to ourselves and others. */
    const char CC_PREFIX[] = "CC: ";


    /* Check whether receiver indicates a universal message.
       We consider a universal message to be one that goes to all real players (i.e. 1-11). */
    bool isUniversalReceiver(game::PlayerSet_t receivers, const game::PlayerList& players)
    {
        return receivers.contains(players.getAllPlayers());
    }

    /* Get list of all allowed receivers.
       We allow all real players plus player 0 (=host). */
    game::PlayerSet_t getAllReceivers(const game::PlayerList& players)
    {
        return players.getAllPlayers() + 0;
    }

    /* Get "TO:" line for a receiver bitfield. */
    String_t getReceiverText(game::PlayerSet_t bits, afl::string::Translator& tx, const game::PlayerList& players)
    {
        /* Note: do not translate "Host" here, because this function is
           also used to generate title lines for sent messages */
        bits &= getAllReceivers(players);
        if (bits.empty()) {
            // Message will not be sent, so we can translate this
            return tx.translateString("Nobody");
        } else if (bits.isUnitSet()) {
            // One receiver
            for (int i = 0; i <= game::MAX_PLAYERS; ++i) {
                if (bits.contains(i)) {
                    if (i == 0) {
                        return "Host";
                    } else {
                        return players.getPlayerName(i, game::Player::LongName);
                    }
                }
            }
            return "Huh?";
        } else {
            // Many receivers
            String_t rv;
            for (int i = 0; i <= game::MAX_PLAYERS; ++i) {
                if (bits.contains(i)) {
                    if (rv.length()) {
                        rv += " ";
                    }
                    if (i == 0) {
                        rv += "Host";
                    } else {
                        rv += afl::string::Format("%d", i);
                    }
                }
            }
            return rv;
        }
    }

    String_t maybeStripHeaders(String_t msg, game::PlayerSet_t rec)
    {
        /* strip headers only from unicast messages */
        if (!rec.isUnitSet()) {
            return msg;
        }

        const String_t firstLine = afl::string::strFirst(msg, "\n");
        if (firstLine == UNIVERSAL_TEXT
            || firstLine.compare(0, sizeof(CC_PREFIX)-1,      CC_PREFIX,      sizeof(CC_PREFIX)-1) == 0
            || firstLine.compare(0, sizeof(CC_SELF_PREFIX)-1, CC_SELF_PREFIX, sizeof(CC_SELF_PREFIX)-1) == 0)
        {
            if (afl::string::strRemove(msg, "\n")) {
                return msg;
            } else {
                return String_t();  // silly people sending empty messages
            }
        } else {
            return msg;
        }
    }
}


struct game::msg::Outbox::Message {
    Id_t        id;
    int         sender;
    String_t    text;
    PlayerSet_t receivers;

    Message(Id_t id, int sender, const String_t& text, PlayerSet_t receivers)
        : id(id), sender(sender), text(text), receivers(receivers)
        { }
};

// Constructor.
game::msg::Outbox::Outbox()
    : Mailbox(),
      m_messages(),
      m_idCounter(0)
{ }

// Destructor.
game::msg::Outbox::~Outbox()
{ }

// Mailbox:
size_t
game::msg::Outbox::getNumMessages() const
{
    // ex GOutbox::getCount
    return m_messages.size();
}

String_t
game::msg::Outbox::getMessageText(size_t index, afl::string::Translator& tx, const PlayerList& players) const
{
    // ex GOutbox::getText
    if (index < m_messages.size()) {
        return getHeadersForDisplay(m_messages[index]->sender, m_messages[index]->receivers, tx, players) + m_messages[index]->text;
    } else {
        return String_t();
    }
}

String_t
game::msg::Outbox::getMessageHeading(size_t index, afl::string::Translator& tx, const PlayerList& players) const
{
    // ex GOutbox::getHeading
    if (index < m_messages.size()) {
        if (isUniversalReceiver(m_messages[index]->receivers, players)) {
            return "Universal Message";
        } else {
            return afl::string::Format(tx("To: %s"), getReceiverText(m_messages[index]->receivers, tx, players));
        }
    } else {
        return String_t();
    }
}

int
game::msg::Outbox::getMessageTurnNumber(size_t /*index*/) const
{
    return 0;
}

bool
game::msg::Outbox::isMessageFiltered(size_t /*index*/, afl::string::Translator& /*tx*/, const PlayerList& /*players*/, const Configuration& /*config*/) const
{
    return false;
}

game::msg::Mailbox::Flags_t
game::msg::Outbox::getMessageFlags(size_t /*index*/) const
{
    return Flags_t();
}

game::msg::Mailbox::Actions_t
game::msg::Outbox::getMessageActions(size_t /*index*/) const
{
    return Actions_t();
}

void
game::msg::Outbox::performMessageAction(size_t /*index*/, Action /*a*/)
{
    // No actions for now
}

// Get prefix for message when sent.
String_t
game::msg::Outbox::getMessageSendPrefix(size_t index, int receiver,
                                        afl::string::Translator& tx,
                                        const PlayerList& players) const
{
    if (index < m_messages.size()) {
        PlayerSet_t receivers = m_messages[index]->receivers & getAllReceivers(players);

        // Universal message? (all or all+host)
        if (isUniversalReceiver(receivers, players)) {
            return String_t(UNIVERSAL_TEXT) + '\n';
        }

        // More than one receiver?
        receivers -= receiver;
        if (!receivers.empty()) {
            const char* pfx = (m_messages[index]->sender == receiver ? CC_SELF_PREFIX : CC_PREFIX);
            return pfx + getReceiverText(receivers, tx, players) + '\n';
        }
    }
    return String_t();
}

// Get raw message text.
String_t
game::msg::Outbox::getMessageRawText(size_t index) const
{
    // ex GOutbox::getRawText
    if (index < m_messages.size()) {
        return m_messages[index]->text;
    } else {
        return String_t();
    }
}

// Get set of message receivers.
game::PlayerSet_t
game::msg::Outbox::getMessageReceivers(size_t index) const
{
    // ex GOutbox::getReceiverMask
    if (index < m_messages.size()) {
        return m_messages[index]->receivers;
    } else {
        return PlayerSet_t();
    }
}

// Get message sender number.
int
game::msg::Outbox::getMessageSender(size_t index) const
{
    // ex GOutbox::getSender (sort-of)
    if (index < m_messages.size()) {
        return m_messages[index]->sender;
    } else {
        return 0;
    }
}

// Get message Id.
game::Id_t
game::msg::Outbox::getMessageId(size_t index) const
{
    if (index < m_messages.size()) {
        return m_messages[index]->id;
    } else {
        return 0;
    }
}

// Set receivers.
void
game::msg::Outbox::setMessageReceivers(size_t index, PlayerSet_t receivers)
{
    // GOutbox::setReceiverMask
    if (index < m_messages.size()) {
        m_messages[index]->receivers = receivers;
    }
}

// Set message content.
void
game::msg::Outbox::setMessageText(size_t index, String_t text)
{
    // GOutbox::setMessage
    if (index < m_messages.size()) {
        m_messages[index]->text = text;
    }
}

// Delete messages starting at an index.
void
game::msg::Outbox::deleteMessagesAfter(size_t index)
{
    // ex GOutbox::deleteMessagesAfter
    if (index < m_messages.size()) {
        m_messages.resize(index);
    }
}

// Delete message by index.
void
game::msg::Outbox::deleteMessage(size_t index)
{
    // ex GOutbox::deleteMessage
    if (index < m_messages.size()) {
        m_messages.erase(m_messages.begin() + index);
    }
}

// Find message, given a Id.
bool
game::msg::Outbox::findMessageById(Id_t id, size_t& index) const
{
    for (size_t i = 0, n = m_messages.size(); i < n; ++i) {
        if (m_messages[i]->id == id) {
            index = i;
            return true;
        }
    }
    return false;
}

// Add a new message (send).
game::Id_t
game::msg::Outbox::addMessage(int sender, String_t text, PlayerSet_t receivers)
{
    // ex GOutbox::addMessage
    Id_t id = allocateId();
    m_messages.pushBackNew(new Message(id, sender, afl::string::strRTrim(text), receivers));
    return id;
}

// Add a new message coming from a message file.
void
game::msg::Outbox::addMessageFromFile(int sender, String_t text, PlayerSet_t receivers)
{
    // ex GOutbox::addMessageFromFile
    /* attempt to merge messages. Preconditions:
       - message box contains at least one message
       - receivers don't overlap
       - message bodies are identical, sans headers */
    text = afl::string::strRTrim(text);
    const String_t rawText = maybeStripHeaders(text, receivers);

    if (!m_messages.empty()
        && (m_messages.back()->sender == sender)
        && (m_messages.back()->receivers & receivers).empty()
        && (maybeStripHeaders(m_messages.back()->text, m_messages.back()->receivers) == rawText))
    {
        /* merge */
        m_messages.back()->receivers |= receivers;
        m_messages.back()->text = rawText;
    } else {
        /* don't merge */
        addMessage(sender, text, receivers);
    }
}

// Clear this mailbox.
void
game::msg::Outbox::clear()
{
    m_messages.clear();

    // Argument for resetting here: observer needs to revalidate whether their message still exists in any case.
    // Argument against resetting here: cache invalidation is harder with resetting, clear() is identical to deleteMessagesAfter(0).
    // m_idCounter = 0;
}

// Get message headers for display.
String_t
game::msg::Outbox::getHeadersForDisplay(int sender,
                                        PlayerSet_t receivers,
                                        afl::string::Translator& tx,
                                        const PlayerList& players)
{
    // ex GOutbox::getHeadersForDisplay
    receivers &= getAllReceivers(players);

    String_t senderName = players.getPlayerName(sender, Player::LongName);
    String_t receiverText = getReceiverText(receivers, tx, players);
    String_t text = afl::string::Format(tx("<<< Sub Space Message >>>\nFROM: %s\nTO: %s\n"), senderName, receiverText);
    if (isUniversalReceiver(receivers, players)) {
        text += UNIVERSAL_TEXT;
        text += '\n';
    } else if (!receivers.isUnitSet()) {
        text += CC_PREFIX; // FIXME: i18n?
        text += receiverText;
        text += '\n';
    } else {
        // no additional header
    }
    return text;
}

game::Id_t
game::msg::Outbox::allocateId()
{
    // FIXME: deal with overflow
    return ++m_idCounter;
}

