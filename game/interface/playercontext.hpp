/**
  *  \file game/interface/playercontext.hpp
  */
#ifndef C2NG_GAME_INTERFACE_PLAYERCONTEXT_HPP
#define C2NG_GAME_INTERFACE_PLAYERCONTEXT_HPP

#include "interpreter/context.hpp"
#include "game/game.hpp"
#include "game/root.hpp"

namespace game { namespace interface {

    class PlayerContext : public interpreter::Context {
     public:
        PlayerContext(int nr, afl::base::Ptr<Game> game, afl::base::Ptr<Root> root);
        ~PlayerContext();

        // Context:
        virtual bool lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual void set(PropertyIndex_t index, afl::data::Value* value);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual PlayerContext* clone() const;
        virtual game::map::Object* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor);

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, interpreter::SaveContext* ctx) const;

     private:
        int m_number;
        afl::base::Ptr<Game> m_game;
        afl::base::Ptr<Root> m_root;
    };

} }

#endif
