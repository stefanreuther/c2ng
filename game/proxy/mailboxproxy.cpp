/**
  *  \file game/proxy/mailboxproxy.cpp
  *  \brief Class game::proxy::MailboxProxy
  */

#include <cctype>
#include "game/proxy/mailboxproxy.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/format.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/msg/configuration.hpp"
#include "game/msg/file.hpp"
#include "game/msg/outbox.hpp"
#include "game/parser/informationconsumer.hpp"
#include "game/playerset.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/root.hpp"

using afl::string::Format;
using game::actions::mustHaveRoot;
using game::actions::mustHaveGame;
using game::msg::Browser;

namespace {
    struct SearchRequest {
        Browser::Mode mode;
        int amount;
        bool acceptFiltered;
        String_t needle;
        SearchRequest(Browser::Mode mode, int amount, bool acceptFiltered, String_t needle)
            : mode(mode), amount(amount), acceptFiltered(acceptFiltered), needle(needle)
            { }
    };
}

class game::proxy::MailboxProxy::Trampoline {
 public:
    Trampoline(MailboxAdaptor& adaptor, util::RequestSender<MailboxProxy> reply);

    void setCurrentMessage(size_t index);
    void browse(game::msg::Browser::Mode mode, int amount, bool acceptFiltered);
    void search(SearchRequest req);
    bool write(const String_t& fileName, size_t first, size_t last, String_t& errorMessage);
    void toggleHeadingFiltered(String_t heading);
    void performMessageAction(game::msg::Mailbox::Action a);
    void receiveData();
    QuoteResult quoteMessage(size_t index, QuoteAction action);

    void packStatus(Status& st);
    void buildSummary(game::msg::Browser::Summary_t& summary, size_t* index) const;

    void sendResponse(bool requested);
    void sendSearchFailure();
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
game::proxy::MailboxProxy::Trampoline::search(SearchRequest req)
{
    // ex readmsg.pas:MessageSearch
    Session& session = m_adaptor.session();

    Browser b(m_adaptor.mailbox(), session.translator(), mustHaveRoot(session).playerList(), req.acceptFiltered ? 0 : m_adaptor.getConfiguration());
    Browser::Result r = b.search(m_currentMessage, req.mode, req.amount, req.needle);
    if (r.found) {
        setCurrentMessage(r.index);
    } else {
        sendSearchFailure();
    }
}

bool
game::proxy::MailboxProxy::Trampoline::write(const String_t& fileName, size_t first, size_t last, String_t& errorMessage)
{
    // ex WMessageDisplay::doWriteMessage (part), readmsg.pas:SaveAllMessages, readmsg.pas:SaveThisMessage
    // TODO: consider moving that to Mailbox and merging with MessageWriteCommand::call
    Session& session = m_adaptor.session();
    afl::io::FileSystem& fs = session.world().fileSystem();
    game::msg::Mailbox& mbox = m_adaptor.mailbox();
    if (Root* r = session.getRoot().get()) {
        try {
            afl::base::Ptr<afl::io::Stream> s = fs.openFileNT(fileName, afl::io::FileSystem::OpenWrite);
            if (s.get() == 0) {
                s = fs.openFile(fileName, afl::io::FileSystem::CreateNew).asPtr();
            }
            s->setPos(s->getSize());

            // Write message. Use game character set, because programs like PCC 1.x / mgrep
            // use these files and assume game character set. The message will not contain
            // anything else, anyway.
            afl::io::TextFile tf(*s);
            tf.setCharsetNew(r->charset().clone());
            game::msg::writeMessages(tf, mbox, first, last, r->playerList(), session.translator());
            tf.flush();
        }
        catch (std::exception& e) {
            errorMessage = e.what();
            return false;
        }
    }
    return true;
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
game::proxy::MailboxProxy::Trampoline::receiveData()
{
    class Consumer : public game::parser::InformationConsumer {
     public:
        Consumer(Game& g, Root& root, Session& session, size_t index)
            : m_game(g), m_root(root), m_session(session), m_index(index)
            { }

        virtual void addMessageInformation(const game::parser::MessageInformation& info)
            {
                m_game.addMessageInformation(info, m_root.hostConfiguration(), m_root.hostVersion(), m_session.world().atomTable(), m_index, false, m_session.translator(), m_session.log());
            }

        using InformationConsumer::addMessageInformation;
     private:
        Game& m_game;
        Root& m_root;
        Session& m_session;
        size_t m_index;
    };


    Session& session = m_adaptor.session();
    Root& root = mustHaveRoot(session);
    game::msg::Mailbox& mbox = m_adaptor.mailbox();
    const size_t index = m_currentMessage;

    if (Game* g = session.getGame().get()) {
        Consumer c(*g, root, session, index);
        mbox.receiveMessageData(index, c, g->teamSettings(), true, root.charset());
    }

    sendResponse(false);
}

game::proxy::MailboxProxy::QuoteResult
game::proxy::MailboxProxy::Trampoline::quoteMessage(size_t index, QuoteAction action)
{
    // Viewpoint player
    int sender = 0;
    if (Game* g = m_adaptor.session().getGame().get()) {
        sender = g->getViewpointPlayer();
    }

    // Message text
    afl::string::Translator& tx = m_adaptor.session().translator();
    String_t text;
    if (Root* r = m_adaptor.session().getRoot().get()) {
        switch (action) {
         case QuoteForForwarding:
            text = m_adaptor.mailbox().getMessageForwardText(index, tx, r->playerList());
            break;

         case QuoteForReplying:
            text = m_adaptor.mailbox().getMessageReplyText(index, tx, r->playerList());
            break;
        }
    }

    return QuoteResult(sender, text);
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
    Game* g = session.getGame().get();
    const int viewpointPlayer = (g != 0 ? g->getViewpointPlayer() : 0);
    const game::msg::Mailbox& mbox = m_adaptor.mailbox();
    afl::string::Translator& tx = session.translator();
    const size_t index = m_currentMessage;

    const game::msg::Mailbox::Metadata md = mbox.getMessageMetadata(index, tx, root.playerList());

    m.text = mbox.getMessageDisplayText(index, tx, root.playerList());
    m.isFiltered = Browser(mbox, tx, root.playerList(), m_adaptor.getConfiguration()).isMessageFiltered(index);

    m.goto1 = md.primaryLink;
    session.getReferenceName(m.goto1, LongName).get(m.goto1Name);
    m.goto2 = md.secondaryLink;
    session.getReferenceName(m.goto2, LongName).get(m.goto2Name);
    m.reply = md.reply;
    m.replyAll = md.replyAll;
    if (m.replyAll != PlayerSet_t(viewpointPlayer) && !m.replyAll.contains(root.playerList().getAllPlayers())) {
        // Remove ourselves from the replyAll list unless we're the only one (message-to-self),
        // or if this would cause a Universal Message to become a not-universal message.
        m.replyAll -= viewpointPlayer;
    }
    if (!m.reply.empty()) {
        m.replyName = formatPlayerHostSet(m.reply, root.playerList(), session.translator());
    }

    m.actions = mbox.getMessageActions(m_currentMessage);
    m.flags = md.flags;

    if (const game::msg::Outbox* out = dynamic_cast<const game::msg::Outbox*>(&mbox)) {
        m.id = out->getMessageId(m_currentMessage);
    }

    m.dataStatus = md.dataStatus;

    m_reply.postRequest(&MailboxProxy::updateCurrentMessage, index, m, requested);
}

inline void
game::proxy::MailboxProxy::Trampoline::sendSearchFailure()
{
    m_reply.postRequest(&MailboxProxy::emitSearchFailure);
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
game::proxy::MailboxProxy::search(game::msg::Browser::Mode mode, int amount, bool acceptFiltered, const String_t& needle)
{
    // Search does not take part in debouncing; the last response might be an error
    // and if we suppress all answers before that, we don't have updateCurrentMessage() data.
    m_request.postRequest(&Trampoline::search, SearchRequest(mode, amount, acceptFiltered, needle));
}

bool
game::proxy::MailboxProxy::write(WaitIndicator& ind, const String_t& fileName, size_t first, size_t last, String_t& errorMessage)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(const String_t& fileName, size_t first, size_t last)
            : m_fileName(fileName), m_first(first), m_last(last),
              m_success(true), m_errorMessage()
            { }
        virtual void handle(Trampoline& tpl)
            { m_success = tpl.write(m_fileName, m_first, m_last, m_errorMessage); }
        bool isOK() const
            { return m_success; }
        const String_t& getErrorMessage() const
            { return m_errorMessage; }
     private:
        String_t m_fileName;
        size_t m_first;
        size_t m_last;
        bool m_success;
        String_t m_errorMessage;  // pass by copy to avoid potential aliasing with user-side
    };

    Task t(fileName, first, last);
    ind.call(m_request, t);
    if (!t.isOK()) {
        errorMessage = t.getErrorMessage();
        return false;
    } else {
        return true;
    }
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
game::proxy::MailboxProxy::receiveData()
{
    m_request.postRequest(&Trampoline::receiveData);
}

game::proxy::MailboxProxy::QuoteResult
game::proxy::MailboxProxy::quoteMessage(WaitIndicator& ind, size_t index, QuoteAction action)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(size_t index, QuoteAction action, QuoteResult& result)
            : m_index(index), m_action(action), m_result(result)
            { }
        virtual void handle(Trampoline& tpl)
            { m_result = tpl.quoteMessage(m_index, m_action); }
     private:
        size_t m_index;
        QuoteAction m_action;
        QuoteResult& m_result;
    };
    QuoteResult result(0, String_t());
    Task t(index, action, result);
    ind.call(m_request, t);
    return result;
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

void
game::proxy::MailboxProxy::emitSearchFailure()
{
    sig_searchFailure.raise();
}
