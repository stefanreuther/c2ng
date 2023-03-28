/**
  *  \file game/interface/explosioncontext.hpp
  *  \brief Class game::interface::ExplosionContext
  */
#ifndef C2NG_GAME_INTERFACE_EXPLOSIONCONTEXT_HPP
#define C2NG_GAME_INTERFACE_EXPLOSIONCONTEXT_HPP

#include "game/map/explosion.hpp"
#include "game/turn.hpp"
#include "game/session.hpp"
#include "interpreter/simplecontext.hpp"

namespace game { namespace interface {

    /** Explosion context.
        Implements the result of enumerating the "Explosion" function.
        To create, usually use ExplosionContext::create().

        @see ExplosionFunction */
    class ExplosionContext : public interpreter::SimpleContext, public interpreter::Context::ReadOnlyAccessor {
     public:
        /** Constructor.
            @param id       Id (index into ExplosionType)
            @param session  Session (for translator, interface)
            @param turn     Turn */
        ExplosionContext(Id_t id, Session& session, afl::base::Ref<Turn> turn);

        /** Destructor. */
        ~ExplosionContext();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual ExplosionContext* clone() const;
        virtual game::map::Explosion* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

        /** Create ExplosionContext.
            @param id      Id (index into ExplosionType)
            @param session Session
            @return newly-allocated ExplosionContext; null if preconditions are missing */
        static ExplosionContext* create(Id_t id, Session& session);

     private:
        Id_t m_id;
        Session& m_session;
        afl::base::Ref<Turn> m_turn;
    };

} }

#endif
