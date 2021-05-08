/**
  *  \file game/msg/browser.cpp
  *  \brief Class game::msg::Browser
  */

#include "game/msg/browser.hpp"
#include "game/msg/mailbox.hpp"

game::msg::Browser::Browser(const Mailbox& mailbox, afl::string::Translator& tx, const PlayerList& players, const Configuration* config)
    : m_mailbox(mailbox),
      m_translator(tx),
      m_playerList(players),
      m_pConfig(config)
{ }

bool
game::msg::Browser::isMessageFiltered(size_t index) const
{
    return m_pConfig != 0
        && m_mailbox.isMessageFiltered(index, m_translator, m_playerList, *m_pConfig);
}

size_t
game::msg::Browser::findFirstMessage() const
{
    size_t numMessages = m_mailbox.getNumMessages();
    size_t i = 0;
    while (i < numMessages && isMessageFiltered(i)) {
        ++i;
    }
    if (i >= numMessages) {
        i = 0;
    }
    return i;
}

size_t
game::msg::Browser::findLastMessage() const
{
    size_t numMessages = m_mailbox.getNumMessages();
    size_t i = numMessages;
    while (i > 0 && isMessageFiltered(i-1)) {
        --i;
    }
    if (i == 0) {
        i = numMessages;
    }
    if (i == 0) {
        return 0;
    } else {
        return i-1;
    }
}

size_t
game::msg::Browser::browse(size_t index, Mode mode, int repeat) const
{
    size_t newIndex = index;
    bool found = false;

    switch (mode) {
     case First:
        index = findFirstMessage();
        break;

     case Last:
        index = findLastMessage();
        break;

     case Previous:
        // ex WMessageDisplay::doPrev
        while (!found && newIndex > 0) {
            --newIndex;
            if (!isMessageFiltered(newIndex)) {
                index = newIndex;
                if (--repeat <= 0) {
                    found = true;
                }
            }
        }
        break;

     case Next:
        // ex WMessageDisplay::doNext
        while (!found && ++newIndex < m_mailbox.getNumMessages()) {
            if (!isMessageFiltered(newIndex)) {
                index = newIndex;
                if (--repeat <= 0) {
                    found = true;
                }
            }
        }
        break;
    }

    return index;
}

void
game::msg::Browser::buildSummary(Summary_t& summary)
{
    // ex WSubjectList::createGroupList
    for (size_t i = 0, n = m_mailbox.getNumMessages(); i < n; ++i) {
        String_t thisHeading = m_mailbox.getMessageHeading(i, m_translator, m_playerList);
        if (summary.empty() || thisHeading != summary.back().heading) {
            summary.push_back(SummaryEntry(i, 1, isMessageFiltered(i), thisHeading));
        } else {
            ++summary.back().count;
        }
    }
}
