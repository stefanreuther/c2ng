/**
  *  \file game/msg/subsetmailbox.cpp
  *  \brief Class game::msg::SubsetMailbox
  */

#include "game/msg/subsetmailbox.hpp"

game::msg::SubsetMailbox::SubsetMailbox(Mailbox& parent, const std::vector<size_t>& indexes)
    : m_parent(parent),
      m_indexes(indexes)
{ }

game::msg::SubsetMailbox::~SubsetMailbox()
{ }

afl::base::Optional<size_t>
game::msg::SubsetMailbox::find(size_t outerIndex)
{
    for (size_t i = 0; i < m_indexes.size(); ++i) {
        if (m_indexes[i] == outerIndex) {
            return i;
        }
    }
    return afl::base::Nothing;
}

size_t
game::msg::SubsetMailbox::getOuterIndex(size_t index) const
{
    return (index < m_indexes.size()
            ? m_indexes[index]
            : 0);
}

size_t
game::msg::SubsetMailbox::getNumMessages() const
{
    // ex GProxyMailbox::getCount
    return m_indexes.size();
}

String_t
game::msg::SubsetMailbox::getMessageHeaderText(size_t index, afl::string::Translator& tx, const PlayerList& players) const
{
    // ex GProxyMailbox::getText [part]
    return (index < m_indexes.size()
            ? m_parent.getMessageHeaderText(m_indexes[index], tx, players)
            : String_t());
}

String_t
game::msg::SubsetMailbox::getMessageBodyText(size_t index, afl::string::Translator& tx, const PlayerList& players) const
{
    // ex GProxyMailbox::getText [part]
    return (index < m_indexes.size()
            ? m_parent.getMessageBodyText(m_indexes[index], tx, players)
            : String_t());
}

String_t
game::msg::SubsetMailbox::getMessageForwardText(size_t index, afl::string::Translator& tx, const PlayerList& players) const
{
    return (index < m_indexes.size()
            ? m_parent.getMessageForwardText(m_indexes[index], tx, players)
            : String_t());
}

String_t
game::msg::SubsetMailbox::getMessageReplyText(size_t index, afl::string::Translator& tx, const PlayerList& players) const
{
    return (index < m_indexes.size()
            ? m_parent.getMessageReplyText(m_indexes[index], tx, players)
            : String_t());
}

util::rich::Text
game::msg::SubsetMailbox::getMessageDisplayText(size_t index, afl::string::Translator& tx, const PlayerList& players) const
{
    return (index < m_indexes.size()
            ? m_parent.getMessageDisplayText(m_indexes[index], tx, players)
            : util::rich::Text());
}

String_t
game::msg::SubsetMailbox::getMessageHeading(size_t index, afl::string::Translator& tx, const PlayerList& players) const
{
    // ex GProxyMailbox::getHeading
    return (index < m_indexes.size()
            ? m_parent.getMessageHeading(m_indexes[index], tx, players)
            : String_t());
}

game::msg::Mailbox::Metadata
game::msg::SubsetMailbox::getMessageMetadata(size_t index, afl::string::Translator& tx, const PlayerList& players) const
{
    return (index < m_indexes.size()
            ? m_parent.getMessageMetadata(m_indexes[index], tx, players)
            : Metadata());
}

game::msg::Mailbox::Actions_t
game::msg::SubsetMailbox::getMessageActions(size_t index) const
{
    return (index < m_indexes.size()
            ? m_parent.getMessageActions(m_indexes[index])
            : Actions_t());
}

void
game::msg::SubsetMailbox::performMessageAction(size_t index, Action a)
{
    if (index < m_indexes.size()) {
        m_parent.performMessageAction(m_indexes[index], a);
    }
}

void
game::msg::SubsetMailbox::receiveMessageData(size_t index, game::parser::InformationConsumer& consumer, const TeamSettings& teamSettings, bool onRequest, afl::charset::Charset& cs)
{
    if (index < m_indexes.size()) {
        m_parent.receiveMessageData(m_indexes[index], consumer, teamSettings, onRequest, cs);
    }
}
