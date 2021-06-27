/**
  *  \file game/proxy/mailboxproxy.cpp
  *  \brief Class game::proxy::MailboxProxy
  */

#include "game/proxy/mailboxproxy.hpp"
#include "game/actions/preconditions.hpp"
#include "game/msg/configuration.hpp"
#include "game/msg/outbox.hpp"
#include "game/parser/format.hpp"
#include "game/playerset.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/root.hpp"

using game::actions::mustHaveRoot;
using game::actions::mustHaveGame;
using game::msg::Browser;

class game::proxy::MailboxProxy::Trampoline {
 public:
    Trampoline(MailboxAdaptor& adaptor, util::RequestSender<MailboxProxy> reply);

    void setCurrentMessage(size_t index);
    void browse(game::msg::Browser::Mode mode, int amount, bool acceptFiltered);
    void toggleHeadingFiltered(String_t heading);
    void performMessageAction(game::msg::Mailbox::Action a);

    void packStatus(Status& st);
    void buildSummary(game::msg::Browser::Summary_t& summary, size_t* index) const;

    void sendResponse(bool requested);
    void sendSummary();

 private:
    MailboxAdaptor& m_adaptor;
    util::RequestSender<MailboxProxy> m_reply;
    size_t m_currentMessage;
};

game::proxy::MailboxProxy::Trampoline::Trampoline(MailboxAdaptor& adaptor, util::RequestSender<MailboxProxy> reply)
    : m_adaptor(adaptor),
      m_reply(reply),
      m_currentMessage(m_adaptor.getCurrentMessage())
{ }

void
game::proxy::MailboxProxy::Trampoline::setCurrentMessage(size_t index)
{
    // Range check
    size_t max = m_adaptor.mailbox().getNumMessages();
    if (index >= max) {
        index = max != 0 ? max-1 : 0;
    }

    // Emit it
    m_adaptor.setCurrentMessage(index);
    m_currentMessage = index;
    sendResponse(true);
}

void
game::proxy::MailboxProxy::Trampoline::browse(game::msg::Browser::Mode mode, int amount, bool acceptFiltered)
{
    Session& session = m_adaptor.session();

    Browser b(m_adaptor.mailbox(), session.translator(), mustHaveRoot(session).playerList(), acceptFiltered ? 0 : m_adaptor.getConfiguration());
    setCurrentMessage(b.browse(m_currentMessage, mode, amount));
}

void
game::proxy::MailboxProxy::Trampoline::toggleHeadingFiltered(String_t heading)
{
    if (game::msg::Configuration* p = m_adaptor.getConfiguration()) {
        p->toggleHeadingFiltered(heading);
        sendResponse(false);
        sendSummary();
    }
}

void
game::proxy::MailboxProxy::Trampoline::performMessageAction(game::msg::Mailbox::Action a)
{
    m_adaptor.mailbox().performMessageAction(m_currentMessage, a);
    sendResponse(false);
}

void
game::proxy::MailboxProxy::Trampoline::packStatus(Status& st)
{
    st.numMessages = m_adaptor.mailbox().getNumMessages();
    st.currentMessage = m_currentMessage;
}

void
game::proxy::MailboxProxy::Trampoline::buildSummary(game::msg::Browser::Summary_t& summary, size_t* index) const
{
    // Environment
    Session& session = m_adaptor.session();
    Root& root = mustHaveRoot(session);
    game::msg::Mailbox& mbox = m_adaptor.mailbox();
    afl::string::Translator& tx = session.translator();

    // Build summary
    Browser(mbox, tx, root.playerList(), m_adaptor.getConfiguration()).buildSummary(summary);

    // Locate current message in it
    if (index != 0) {
        size_t i = 0;
        while (i+1 < summary.size() && m_currentMessage >= summary[i+1].index) {
            ++i;
        }
        *index = i;
    }
}

void
game::proxy::MailboxProxy::Trampoline::sendResponse(bool requested)
{
    Message m;
    Session& session = m_adaptor.session();
    Root& root = mustHaveRoot(session);
    game::msg::Mailbox& mbox = m_adaptor.mailbox();
    afl::string::Translator& tx = session.translator();
    const size_t index = m_currentMessage;

    game::parser::Format fmt;
    formatMessage(fmt, mbox.getMessageText(index, tx, root.playerList()),
                  root.playerList());

    m.text = fmt.text.withStyle(util::rich::StyleAttribute::Fixed);
    m.isFiltered = Browser(mbox, tx, root.playerList(), m_adaptor.getConfiguration()).isMessageFiltered(index);

    m.goto2 = fmt.firstLink;
    session.getReferenceName(fmt.firstLink, LongName, m.goto2Name);
    m.reply = fmt.reply;
    m.replyAll = fmt.replyAll;
    if (!fmt.reply.empty()) {
        m.replyName = formatPlayerHostSet(fmt.reply, root.playerList(), session.translator());
    }

    m.actions = mbox.getMessageActions(m_currentMessage);
    m.flags = mbox.getMessageFlags(m_currentMessage);

    if (const game::msg::Outbox* out = dynamic_cast<game::msg::Outbox*>(&mbox)) {
        m.id = out->getMessageId(m_currentMessage);
    }

    m_reply.postRequest(&MailboxProxy::updateCurrentMessage, index, m, requested);
}

void
game::proxy::MailboxProxy::Trampoline::sendSummary()
{
    class Task : public util::Request<MailboxProxy> {
     public:
        Task(const Trampoline& tpl)
            { tpl.buildSummary(m_summary, 0); }
        virtual void handle(MailboxProxy& proxy)
            { proxy.sig_summaryChanged.raise(m_summary); }
     private:
        game::msg::Browser::Summary_t m_summary;
    };
    m_reply.postNewRequest(new Task(*this));
}


/*
 *  TrampolineFromAdaptor
 */

class game::proxy::MailboxProxy::TrampolineFromAdaptor : public afl::base::Closure<Trampoline*(MailboxAdaptor&)> {
 public:
    TrampolineFromAdaptor(const util::RequestSender<MailboxProxy>& reply)
        : m_reply(reply)
        { }
    virtual Trampoline* call(MailboxAdaptor& adaptor)
        { return new Trampoline(adaptor, m_reply); }
 private:
    util::RequestSender<MailboxProxy> m_reply;
};


/*
 *  MailboxProxy
 */

game::proxy::MailboxProxy::MailboxProxy(util::RequestSender<MailboxAdaptor> sender, util::RequestDispatcher& recv)
    : m_reply(recv, *this),
      m_request(sender.makeTemporary(new TrampolineFromAdaptor(m_reply.getSender()))),
      m_numRequests(0)
{ }

game::proxy::MailboxProxy::~MailboxProxy()
{ }

void
game::proxy::MailboxProxy::getStatus(WaitIndicator& ind, Status& status)
{
    // Query status from game side
    class Task : public util::Request<Trampoline> {
     public:
        Task(Status& status)
            : m_status(status)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.packStatus(m_status); }
     private:
        Status& m_status;
    };
    Task t(status);
    ind.call(m_request, t);
}

void
game::proxy::MailboxProxy::getSummary(WaitIndicator& ind, game::msg::Browser::Summary_t& summary, size_t& index)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(game::msg::Browser::Summary_t& summary, size_t& index)
            : m_summary(summary), m_index(index)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.buildSummary(m_summary, &m_index); }
     private:
        game::msg::Browser::Summary_t& m_summary;
        size_t& m_index;
    };
    Task t(summary, index);
    ind.call(m_request, t);
}

void
game::proxy::MailboxProxy::setCurrentMessage(size_t index)
{
    ++m_numRequests;
    m_request.postRequest(&Trampoline::setCurrentMessage, index);
}

void
game::proxy::MailboxProxy::browse(game::msg::Browser::Mode mode, int amount, bool acceptFiltered)
{
    ++m_numRequests;
    m_request.postRequest(&Trampoline::browse, mode, amount, acceptFiltered);
}

void
game::proxy::MailboxProxy::toggleHeadingFiltered(String_t heading)
{
    m_request.postRequest(&Trampoline::toggleHeadingFiltered, heading);
}

void
game::proxy::MailboxProxy::performMessageAction(game::msg::Mailbox::Action a)
{
    m_request.postRequest(&Trampoline::performMessageAction, a);
}

void
game::proxy::MailboxProxy::updateCurrentMessage(size_t index, Message data, bool requested)
{
    if (requested && m_numRequests > 0) {
        --m_numRequests;
    }
    if (m_numRequests == 0) {
        sig_update.raise(index, data);
    }
}
