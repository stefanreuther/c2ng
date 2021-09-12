/**
  *  \file game/interface/playercontext.hpp
  */
#ifndef C2NG_GAME_INTERFACE_PLAYERCONTEXT_HPP
#define C2NG_GAME_INTERFACE_PLAYERCONTEXT_HPP

#include "interpreter/context.hpp"
#include "game/game.hpp"
#include "game/root.hpp"
#include "game/session.hpp"

namespace game { namespace interface {

    class PlayerContext : public interpreter::Context, public interpreter::Context::ReadOnlyAccessor {
     public:
        PlayerContext(int nr, afl::base::Ref<Game> game, afl::base::Ref<Root> root);
        ~PlayerContext();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual PlayerContext* clone() const;
        virtual game::map::Object* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor);

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, interpreter::SaveContext& ctx) const;

        static PlayerContext* create(int nr, Session& session);

     private:
        int m_number;
        afl::base::Ref<Game> m_game;
        afl::base::Ref<Root> m_root;
    };

} }

#endif
