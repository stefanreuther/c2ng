/**
  *  \file game/interface/explosioncontext.hpp
  */
#ifndef C2NG_GAME_INTERFACE_EXPLOSIONCONTEXT_HPP
#define C2NG_GAME_INTERFACE_EXPLOSIONCONTEXT_HPP

#include "game/map/explosion.hpp"
#include "game/turn.hpp"
#include "game/session.hpp"
#include "interpreter/context.hpp"

namespace game { namespace interface {

    class ExplosionContext : public interpreter::Context, public interpreter::Context::ReadOnlyAccessor {
     public:
        ExplosionContext(Id_t id, Session& session, afl::base::Ref<Turn> turn);
        ~ExplosionContext();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual ExplosionContext* clone() const;
        virtual game::map::Explosion* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor);

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

        static ExplosionContext* create(Id_t id, Session& session);

     private:
        Id_t m_id;
        Session& m_session;
        afl::base::Ref<Turn> m_turn;
    };

} }

#endif
