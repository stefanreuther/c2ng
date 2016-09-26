/**
  *  \file game/interface/minefieldcontext.hpp
  */
#ifndef C2NG_GAME_INTERFACE_MINEFIELDCONTEXT_HPP
#define C2NG_GAME_INTERFACE_MINEFIELDCONTEXT_HPP

#include "game/game.hpp"
#include "interpreter/context.hpp"
#include "game/map/minefield.hpp"
#include "game/root.hpp"

namespace game { namespace interface {

    class MinefieldContext : public interpreter::Context {
     public:
        MinefieldContext(int id, afl::base::Ptr<Root> root, afl::base::Ptr<Game> game);
        ~MinefieldContext();

        // Context:
        virtual bool lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual void set(PropertyIndex_t index, afl::data::Value* value);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual MinefieldContext* clone() const;
        virtual game::map::Minefield* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor);

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, interpreter::SaveContext* ctx) const;

     private:
        int m_id;
        afl::base::Ptr<Root> m_root;
        afl::base::Ptr<Game> m_game;
    };

} }

#endif
