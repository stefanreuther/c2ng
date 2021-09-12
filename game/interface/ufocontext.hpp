/**
  *  \file game/interface/ufocontext.hpp
  */
#ifndef C2NG_GAME_INTERFACE_UFOCONTEXT_HPP
#define C2NG_GAME_INTERFACE_UFOCONTEXT_HPP

#include "game/game.hpp"
#include "game/map/ufo.hpp"
#include "game/session.hpp"
#include "game/turn.hpp"
#include "interpreter/context.hpp"

namespace game { namespace interface {

    class UfoContext : public interpreter::Context, public interpreter::Context::PropertyAccessor {
     public:
        UfoContext(Id_t slot, afl::base::Ref<Turn> turn, Session& session);
        ~UfoContext();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual void set(PropertyIndex_t index, const afl::data::Value* value);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual UfoContext* clone() const;
        virtual game::map::Ufo* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor);

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, interpreter::SaveContext& ctx) const;

     private:
        Id_t m_slot;
        afl::base::Ref<Turn> m_turn;
        Session& m_session;
    };

} }

#endif
