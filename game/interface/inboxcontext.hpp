/**
  *  \file game/interface/inboxcontext.hpp
  *  \brief Class game::interface::InboxContext
  */
#ifndef C2NG_GAME_INTERFACE_INBOXCONTEXT_HPP
#define C2NG_GAME_INTERFACE_INBOXCONTEXT_HPP

#include "afl/base/ref.hpp"
#include "game/msg/inbox.hpp"
#include "game/parser/messagetemplate.hpp"
#include "game/session.hpp"
#include "game/turn.hpp"
#include "interpreter/simplecontext.hpp"

namespace game { namespace interface {

    /** Inbox context.
        Implements the result of the "InMsg" function (-> InboxFunction). */
    class InboxContext : public interpreter::SimpleContext, interpreter::Context::ReadOnlyAccessor {
     public:
        /** Constructor.
            \param index Message index (0-based)
            \param session Session (for Game::messageConfiguration(), reference resolution, translator, Root::playerList())
            \param turn  Turn (for messages) */
        InboxContext(size_t index, game::Session& session, const afl::base::Ref<const Turn>& turn);

        /** Destructor. */
        ~InboxContext();

        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual InboxContext* clone() const;
        virtual afl::base::Deletable* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const;
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

     private:
        const game::msg::Mailbox& mailbox();
        void clearLineCache();
        afl::base::Ptr<game::parser::MessageLines_t> getLineCache();
        game::msg::Mailbox::Metadata getCurrentMetadata(const Root& root);

        size_t m_index;
        game::Session& m_session;
        const afl::base::Ref<const Turn> m_turn;

        afl::base::Ptr<game::parser::MessageLines_t> m_lineCache;
    };

} }

#endif
