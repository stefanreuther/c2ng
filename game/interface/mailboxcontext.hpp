/**
  *  \file game/interface/mailboxcontext.hpp
  *  \brief Class game::interface::MailboxContext
  */
#ifndef C2NG_GAME_INTERFACE_MAILBOXCONTEXT_HPP
#define C2NG_GAME_INTERFACE_MAILBOXCONTEXT_HPP

#include "game/msg/inbox.hpp"
#include "game/msg/mailbox.hpp"
#include "game/session.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/singlecontext.hpp"

namespace game { namespace interface {

    /** Script-interface to a temporary, modifiable game::msg::Inbox object.
        Implements the Mailbox() function. */
    class MailboxContext : public interpreter::SingleContext, public interpreter::Context::ReadOnlyAccessor {
     public:
        /** Constructor.
            Creates a fresh MailboxContext containing an empty mailbox.
            @param session Session */
        static MailboxContext* create(Session& session);

        /** Destructor. */
        ~MailboxContext();

        /** Access underlying mailbox.
            @return Mailbox */
        game::msg::Mailbox& mailbox();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual MailboxContext* clone() const;
        virtual afl::base::Deletable* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

     private:
        MailboxContext(const afl::base::Ptr<game::msg::Inbox> mailbox, Session& session);

        afl::base::Ptr<game::msg::Inbox> m_mailbox;
        Session& m_session;
    };

    /** Implementation of the Mailbox() function.
        @param session Session
        @param args Arguments
        @return MailboxContext object */
    afl::data::Value* IFMailbox(game::Session& session, interpreter::Arguments& args);

} }

#endif
