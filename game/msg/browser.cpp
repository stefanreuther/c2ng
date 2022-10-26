/**
  *  \file game/msg/browser.cpp
  *  \brief Class game::msg::Browser
  */

#include "game/msg/browser.hpp"
#include "game/msg/mailbox.hpp"

class game::msg::Browser::Acceptor {
 public:
    virtual bool accept(size_t index) const = 0;
};

class game::msg::Browser::BrowseAcceptor : public Acceptor {
 public:
    BrowseAcceptor(const Browser& parent)
        : m_parent(parent)
        { }
    virtual bool accept(size_t index) const
        { return !m_parent.isMessageFiltered(index); }
 private:
    const Browser& m_parent;
};

class game::msg::Browser::SearchAcceptor : public Acceptor {
 public:
    SearchAcceptor(const Browser& parent, const String_t& needle)
        : m_parent(parent), m_needle(afl::string::strUCase(needle))
        { }
    virtual bool accept(size_t index) const
        {
            if (m_parent.isMessageFiltered(index)) {
                return false;
            }
            if (afl::string::strUCase(m_parent.m_mailbox.getMessageText(index, m_parent.m_translator, m_parent.m_playerList)).find(m_needle) == String_t::npos) {
                return false;
            }
            return true;
        }
 private:
    const Browser& m_parent;
    const String_t m_needle;
};


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
    return findFirstMessage(BrowseAcceptor(*this)).index;
}

size_t
game::msg::Browser::findLastMessage() const
{
    return findLastMessage(BrowseAcceptor(*this)).index;
}

size_t
game::msg::Browser::browse(size_t index, Mode mode, int amount) const
{
    return browse(index, mode, amount, BrowseAcceptor(*this)).index;
}

game::msg::Browser::Result
game::msg::Browser::search(size_t index, Mode mode, int amount, const String_t& needle) const
{
    return browse(index, mode, amount, SearchAcceptor(*this, needle));
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

game::msg::Browser::Result
game::msg::Browser::findFirstMessage(const Acceptor& a) const
{
    size_t numMessages = m_mailbox.getNumMessages();
    size_t i = 0;
    while (i < numMessages && !a.accept(i)) {
        ++i;
    }
    if (i >= numMessages) {
        return Result(0, false);
    } else {
        return Result(i, true);
    }
}

game::msg::Browser::Result
game::msg::Browser::findLastMessage(const Acceptor& a) const
{
    size_t numMessages = m_mailbox.getNumMessages();
    size_t i = numMessages;
    while (i > 0 && !a.accept(i-1)) {
        --i;
    }
    if (i == 0) {
        size_t r = numMessages == 0 ? 0 : numMessages-1;
        return Result(r, false);
    } else {
        return Result(i-1, true);
    }
}

game::msg::Browser::Result
game::msg::Browser::browse(size_t index, Mode mode, int amount, const Acceptor& a) const
{
    Result r(index, false);
    size_t newIndex = index;

    switch (mode) {
     case First:
        r = findFirstMessage(a);
        break;

     case Last:
        r = findLastMessage(a);
        break;

     case Previous:
        // ex WMessageDisplay::doPrev, CMessageView.Page
        while (!r.found && newIndex > 0) {
            --newIndex;
            if (a.accept(newIndex)) {
                r.index = newIndex;
                if (--amount <= 0) {
                    r.found = true;
                }
            }
        }
        break;

     case Next:
        // ex WMessageDisplay::doNext, CMessageView.Page
        while (!r.found && ++newIndex < m_mailbox.getNumMessages()) {
            if (a.accept(newIndex)) {
                r.index = newIndex;
                if (--amount <= 0) {
                    r.found = true;
                }
            }
        }
        break;
    }

    return r;
}
