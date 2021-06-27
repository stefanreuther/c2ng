/**
  *  \file game/proxy/outboxproxy.cpp
  *  \brief Class game::proxy::OutboxProxy
  */

#include "game/proxy/outboxproxy.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/msg/outbox.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/values.hpp"

using game::actions::mustHaveGame;
using game::actions::mustHaveRoot;
using game::msg::Outbox;

namespace {
    /* @q CCUI$CurrentOutMsg:Int (Internal Variable)
       Zero-based index of current outbox message.
       @since PCC2 2.40.11 */
    const char*const INDEX_VAR_NAME = "CCUI$CURRENTOUTMSG";

    Outbox& getOutbox(game::Session& session)
    {
        return game::actions::mustHaveGame(session).currentTurn().outbox();
    }
}

game::proxy::OutboxProxy::OutboxProxy(util::RequestSender<Session> sender)
    : m_gameSender(sender)
{ }

game::proxy::OutboxProxy::~OutboxProxy()
{ }

String_t
game::proxy::OutboxProxy::getHeadersForDisplay(WaitIndicator& ind, int sender, PlayerSet_t receivers)
{
    class Task : public util::Request<Session> {
     public:
        Task(String_t& result, int sender, PlayerSet_t receivers)
            : m_result(result), m_sender(sender), m_receivers(receivers)
            { }
        virtual void handle(Session& session)
            { m_result = game::msg::Outbox::getHeadersForDisplay(m_sender, m_receivers, session.translator(), mustHaveRoot(session).playerList()); }
     private:
        String_t& m_result;
        const int m_sender;
        const PlayerSet_t m_receivers;
    };
    String_t result;
    Task t(result, sender, receivers);
    ind.call(m_gameSender, t);
    return result;
}

bool
game::proxy::OutboxProxy::getMessage(WaitIndicator& ind, Id_t id, Info& result)
{
    class Task : public util::Request<Session> {
     public:
        Task(bool& ok, Id_t id, Info& result)
            : m_ok(ok), m_id(id), m_result(result)
            { }
        virtual void handle(Session& session)
            {
                const Outbox& mbx = getOutbox(session);
                size_t index;
                if (mbx.findMessageById(m_id, index)) {
                    m_result.receivers = mbx.getMessageReceivers(index);
                    m_result.text      = mbx.getMessageRawText(index);
                    m_result.sender    = mbx.getMessageSender(index);
                    m_ok = true;
                }
            }
     private:
        bool& m_ok;
        const Id_t m_id;
        Info& m_result;
    };
    bool ok = false;
    Task t(ok, id, result);
    ind.call(m_gameSender, t);
    return ok;
}

game::StringVerifier*
game::proxy::OutboxProxy::createStringVerifier(WaitIndicator& ind)
{
    typedef std::auto_ptr<StringVerifier> Result_t;
    class Task : public util::Request<Session> {
     public:
        Task(Result_t& result)
            : m_result(result)
            { }
        virtual void handle(Session& session)
            { m_result.reset(mustHaveRoot(session).stringVerifier().clone()); }
     private:
        Result_t& m_result;
    };
    Result_t result;
    Task t(result);
    ind.call(m_gameSender, t);
    return result.release();
}

void
game::proxy::OutboxProxy::addMessage(int sender, String_t text, PlayerSet_t receivers)
{
    class Task : public util::Request<Session> {
     public:
        Task(int sender, String_t text, PlayerSet_t receivers)
            : m_sender(sender), m_text(text), m_receivers(receivers)
            { }
        virtual void handle(Session& session)
            { getOutbox(session).addMessage(m_sender, m_text, m_receivers); }
     private:
        const int m_sender;
        const String_t m_text;
        const PlayerSet_t m_receivers;
    };
    m_gameSender.postNewRequest(new Task(sender, text, receivers));
}

void
game::proxy::OutboxProxy::setMessageText(Id_t id, String_t text)
{
    class Task : public util::Request<Session> {
     public:
        Task(Id_t id, String_t text)
            : m_id(id), m_text(text)
            { }
        virtual void handle(Session& session)
            {
                Outbox& mbx = getOutbox(session);
                size_t index;
                if (mbx.findMessageById(m_id, index)) {
                    mbx.setMessageText(index, m_text);
                }
            }
     private:
        const Id_t m_id;
        const String_t m_text;
    };
    m_gameSender.postNewRequest(new Task(id, text));
}

void
game::proxy::OutboxProxy::setMessageReceivers(Id_t id, PlayerSet_t receivers)
{
    class Task : public util::Request<Session> {
     public:
        Task(Id_t id, PlayerSet_t receivers)
            : m_id(id), m_receivers(receivers)
            { }
        virtual void handle(Session& session)
            {
                Outbox& mbx = getOutbox(session);
                size_t index;
                if (mbx.findMessageById(m_id, index)) {
                    mbx.setMessageReceivers(index, m_receivers);
                }
            }
     private:
        const Id_t m_id;
        const PlayerSet_t m_receivers;
    };
    m_gameSender.postNewRequest(new Task(id, receivers));
}

void
game::proxy::OutboxProxy::deleteMessage(Id_t id)
{
    class Task : public util::Request<Session> {
     public:
        Task(Id_t id)
            : m_id(id)
            { }
        virtual void handle(Session& session)
            {
                Outbox& mbx = getOutbox(session);
                size_t index;
                if (mbx.findMessageById(m_id, index)) {
                    mbx.deleteMessage(index);
                }
            }
     private:
        const Id_t m_id;
    };
    m_gameSender.postNewRequest(new Task(id));
}

util::RequestSender<game::proxy::MailboxAdaptor>
game::proxy::OutboxProxy::getMailboxAdaptor()
{
    class Adaptor : public MailboxAdaptor {
     public:
        Adaptor(Session& session)
            : m_session(session)
            { }
        virtual Session& session() const
            { return m_session; }
        virtual game::msg::Mailbox& mailbox() const
            { return getOutbox(m_session); }
        virtual game::msg::Configuration* getConfiguration() const
            { return 0; }
        virtual size_t getCurrentMessage() const
            {
                // Fetch
                size_t result = 0;
                try {
                    int32_t i;
                    if (interpreter::checkIntegerArg(i, m_session.world().getGlobalValue(INDEX_VAR_NAME))) {
                        result = static_cast<size_t>(i);
                    }
                }
                catch (...)
                { }

                // Return only if valid
                return (result < getOutbox(m_session).getNumMessages() ? result : 0);
            }
        virtual void setCurrentMessage(size_t n)
            { m_session.world().setNewGlobalValue(INDEX_VAR_NAME, interpreter::makeIntegerValue(int32_t(n))); }
     private:
        Session& m_session;
    };
    class Converter : public afl::base::Closure<MailboxAdaptor*(Session&)> {
     public:
        virtual MailboxAdaptor* call(Session& session)
            { return new Adaptor(session); }
    };
    return m_gameSender.makeTemporary(new Converter());
}
