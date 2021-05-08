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
game::msg::SubsetMailbox::getMessageText(size_t index, afl::string::Translator& tx, const PlayerList& players) const
{
    // ex GProxyMailbox::getText
    return (index < m_indexes.size()
            ? m_parent.getMessageText(m_indexes[index], tx, players)
            : String_t());
}

String_t
game::msg::SubsetMailbox::getMessageHeading(size_t index, afl::string::Translator& tx, const PlayerList& players) const
{
    // ex GProxyMailbox::getHeading
    return (index < m_indexes.size()
            ? m_parent.getMessageHeading(m_indexes[index], tx, players)
            : String_t());
}

int
game::msg::SubsetMailbox::getMessageTurnNumber(size_t index) const
{
    return (index < m_indexes.size()
            ? m_parent.getMessageTurnNumber(m_indexes[index])
            : 0);
}

bool
game::msg::SubsetMailbox::isMessageFiltered(size_t index, afl::string::Translator& tx, const PlayerList& players, const Configuration& config) const
{
    return (index < m_indexes.size()
            ? m_parent.isMessageFiltered(m_indexes[index], tx, players, config)
            : false);
}

game::msg::Mailbox::Flags_t
game::msg::SubsetMailbox::getMessageFlags(size_t index) const
{
    return (index < m_indexes.size()
            ? m_parent.getMessageFlags(m_indexes[index])
            : Flags_t());
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
