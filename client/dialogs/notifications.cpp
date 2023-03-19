/**
  *  \file client/dialogs/notifications.cpp
  *  \brief Notifications
  */

#include "client/dialogs/notifications.hpp"
#include "client/dialogs/inboxdialog.hpp"

namespace {
    /*
     *  NotificationAdaptor: MailboxAdaptor implementation
     */

    class NotificationAdaptor : public game::proxy::MailboxAdaptor {
     public:
        NotificationAdaptor(game::Session& session, afl::base::Optional<uint32_t> processId)
            : m_session(session),
              m_currentMessage(0)
            {
                if (const uint32_t* p = processId.get()) {
                    m_currentMessage = m_session.notifications().findIndexByProcessId(*p).orElse(0);
                }
            }

        virtual game::Session& session() const
            { return m_session; }

        virtual game::msg::Mailbox& mailbox() const
            { return m_session.notifications(); }

        virtual game::msg::Configuration* getConfiguration() const
            { return 0; }

        virtual size_t getCurrentMessage() const
            { return m_currentMessage; }

        virtual void setCurrentMessage(size_t n)
            { m_currentMessage = n; }

     private:
        game::Session& m_session;
        size_t m_currentMessage;
    };


    /*
     *  AdaptorFromSession
     */

    class AdaptorFromSession : public afl::base::Closure<game::proxy::MailboxAdaptor*(game::Session&)> {
     public:
        AdaptorFromSession(afl::base::Optional<uint32_t> processId)
            : m_processId(processId)
            { }

        virtual game::proxy::MailboxAdaptor* call(game::Session& s)
            { return new NotificationAdaptor(s, m_processId); }
     private:
        afl::base::Optional<uint32_t> m_processId;
    };
}

void
client::dialogs::showNotifications(afl::base::Optional<uint32_t> processId,
                                   game::proxy::ProcessListProxy& plProxy,
                                   client::si::UserSide& iface,
                                   ui::Root& root,
                                   afl::string::Translator& tx,
                                   client::si::OutputState& out)
{
    // ex doNotifyScreen
    InboxDialog dlg(tx("Notifications"), iface.gameSender().makeTemporary(new AdaptorFromSession(processId)), iface, root, tx);
    dlg.run(out, "pcc2:notify", tx("No notifications"));

    // Technically, this could be outside this function; it's here so it cannot be forgotten.
    plProxy.resumeConfirmedProcesses();
}
