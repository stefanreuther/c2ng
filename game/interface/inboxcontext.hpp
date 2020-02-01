/**
  *  \file game/interface/inboxcontext.hpp
  */
#ifndef C2NG_GAME_INTERFACE_INBOXCONTEXT_HPP
#define C2NG_GAME_INTERFACE_INBOXCONTEXT_HPP

#include "afl/base/ref.hpp"
#include "game/game.hpp"
#include "game/session.hpp"
#include "interpreter/context.hpp"
#include "game/parser/messagetemplate.hpp"

namespace game { namespace interface {

    class InboxContext : public interpreter::Context {
     public:
        InboxContext(size_t index,
                     Session& session,
                     afl::base::Ref<Root> root,
                     afl::base::Ref<Game> game);
        ~InboxContext();

        virtual InboxContext* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual void set(PropertyIndex_t index, afl::data::Value* value);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual InboxContext* clone() const;
        virtual game::map::Object* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor);
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, interpreter::SaveContext& ctx) const;

     private:
        game::msg::Mailbox& mailbox();
        void clearLineCache();
        afl::base::Ptr<game::parser::MessageLines_t> getLineCache();

        size_t m_index;
        Session& m_session;
        afl::base::Ref<Root> m_root;
        afl::base::Ref<Game> m_game;

        afl::base::Ptr<game::parser::MessageLines_t> m_lineCache;
    };

} }

#endif
