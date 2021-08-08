/**
  *  \file game/interface/inboxcontext.hpp
  *  \brief Class game::interface::InboxContext
  */
#ifndef C2NG_GAME_INTERFACE_INBOXCONTEXT_HPP
#define C2NG_GAME_INTERFACE_INBOXCONTEXT_HPP

#include "afl/base/ref.hpp"
#include "afl/string/translator.hpp"
#include "game/game.hpp"
#include "game/msg/inbox.hpp"
#include "game/parser/messagetemplate.hpp"
#include "interpreter/context.hpp"

namespace game { namespace interface {

    /** Inbox context.
        Implements the result of the "InMsg" function (-> InboxFunction). */
    class InboxContext : public interpreter::Context, interpreter::Context::ReadOnlyAccessor {
     public:
        /** Constructor.
            \param index Message index (0-based)
            \param tx    Translator
            \param root  Root
            \param game  Game */
        InboxContext(size_t index,
                     afl::string::Translator& tx,
                     afl::base::Ref<const Root> root,
                     afl::base::Ref<const Game> game);
        ~InboxContext();

        virtual InboxContext* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual InboxContext* clone() const;
        virtual game::map::Object* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor);
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, interpreter::SaveContext& ctx) const;

     private:
        const game::msg::Mailbox& mailbox();
        void clearLineCache();
        afl::base::Ptr<game::parser::MessageLines_t> getLineCache();

        size_t m_index;
        afl::string::Translator& m_translator;
        afl::base::Ref<const Root> m_root;
        afl::base::Ref<const Game> m_game;

        afl::base::Ptr<game::parser::MessageLines_t> m_lineCache;
    };

} }

#endif
